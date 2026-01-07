//
// Created by sbussiso on 1/5/26.
//
#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <sstream>

// mDNS includes (Avahi)
#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/error.h>
#include <avahi-common/alternative.h>
#include <avahi-common/malloc.h>
#include <avahi-common/thread-watch.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include "mqtt/async_client.h"

using namespace cv;
using namespace std;

// Global flags
atomic<bool> streaming(true);
atomic<bool> running(true);

// Configuration - read from environment variables with defaults
string getEnvOrDefault(const char* name, const string& defaultValue) {
    const char* value = getenv(name);
    return value ? string(value) : defaultValue;
}

// These will be initialized in main()
string MQTT_SERVER;
string CLIENT_ID;
string CAMERA_ID;
string CAMERA_NAME;
int CAMERA_DEVICE_INDEX;

// ============================================================================
// mDNS Service Broadcaster for Camera Node
// ============================================================================
class CameraMDNSBroadcaster {
private:
    AvahiThreadedPoll* threaded_poll;
    AvahiClient* client;
    AvahiEntryGroup* group;
    string service_name;
    string camera_id;
    int rtsp_port;
    string rtsp_path;
    string current_status;
    
    static void entry_group_callback(AvahiEntryGroup* g, AvahiEntryGroupState state, void* userdata) {
        CameraMDNSBroadcaster* broadcaster = static_cast<CameraMDNSBroadcaster*>(userdata);
        
        switch(state) {
            case AVAHI_ENTRY_GROUP_ESTABLISHED:
                cout << "[mDNS] Service '" << broadcaster->service_name 
                     << "' successfully established" << endl;
                break;
                
            case AVAHI_ENTRY_GROUP_COLLISION: {
                char* alt_name = avahi_alternative_service_name(broadcaster->service_name.c_str());
                broadcaster->service_name = alt_name;
                avahi_free(alt_name);
                cout << "[mDNS] Service name collision, renaming to '" 
                     << broadcaster->service_name << "'" << endl;
                broadcaster->create_services();
                break;
            }
                
            case AVAHI_ENTRY_GROUP_FAILURE:
                cerr << "[mDNS] Entry group failure" << endl;
                break;
                
            case AVAHI_ENTRY_GROUP_UNCOMMITED:
            case AVAHI_ENTRY_GROUP_REGISTERING:
                break;
        }
    }
    
    static void client_callback(AvahiClient* c, AvahiClientState state, void* userdata) {
        CameraMDNSBroadcaster* broadcaster = static_cast<CameraMDNSBroadcaster*>(userdata);
        
        // Store the client pointer - callback fires before avahi_client_new() returns
        broadcaster->client = c;
        
        switch(state) {
            case AVAHI_CLIENT_S_RUNNING:
                cout << "[mDNS] Client running, registering service..." << endl;
                broadcaster->create_services();
                break;
                
            case AVAHI_CLIENT_FAILURE:
                cerr << "[mDNS] Client failure: " << avahi_strerror(avahi_client_errno(c)) << endl;
                break;
                
            case AVAHI_CLIENT_S_COLLISION:
            case AVAHI_CLIENT_S_REGISTERING:
                if(broadcaster->group)
                    avahi_entry_group_reset(broadcaster->group);
                break;
                
            case AVAHI_CLIENT_CONNECTING:
                cout << "[mDNS] Connecting to Avahi daemon..." << endl;
                break;
        }
    }
    
    void create_services() {
        if(!client) {
            return;  // Client not ready yet
        }
        
        if(avahi_client_get_state(client) != AVAHI_CLIENT_S_RUNNING) {
            return;  // Client not running
        }
            
        if(!group) {
            group = avahi_entry_group_new(client, entry_group_callback, this);
            if(!group) {
                cerr << "[mDNS] Failed to create entry group: " << avahi_strerror(avahi_client_errno(client)) << endl;
                return;
            }
        }
        
        if(avahi_entry_group_is_empty(group)) {
            // Build TXT records - store in persistent strings
            string txt_camera_id = "camera_id=" + camera_id;
            string txt_name = "name=" + service_name;
            string txt_rtsp_port = "rtsp_port=" + to_string(rtsp_port);
            string txt_rtsp_path = "rtsp_path=" + rtsp_path;
            string txt_status = "status=" + current_status;
            
            int ret = avahi_entry_group_add_service(
                group,
                AVAHI_IF_UNSPEC,
                AVAHI_PROTO_UNSPEC,
                static_cast<AvahiPublishFlags>(0),
                service_name.c_str(),
                "_opensentry._tcp",        // Service type for OpenSentry cameras
                nullptr,                   // domain
                nullptr,                   // host
                rtsp_port,
                txt_camera_id.c_str(),
                txt_name.c_str(),
                txt_rtsp_port.c_str(),
                txt_rtsp_path.c_str(),
                txt_status.c_str(),
                "type=camera",
                "protocol=rtsp",
                nullptr
            );
            
            if(ret < 0) {
                cerr << "[mDNS] Failed to add service: " << avahi_strerror(ret) << endl;
                return;
            }
            
            ret = avahi_entry_group_commit(group);
            if(ret < 0) {
                cerr << "[mDNS] Failed to commit entry group: " << avahi_strerror(ret) << endl;
            }
        }
    }
    
public:
    CameraMDNSBroadcaster(const string& cam_id, const string& name, int port, const string& path)
        : threaded_poll(nullptr), client(nullptr), group(nullptr),
          service_name(name), camera_id(cam_id), rtsp_port(port), 
          rtsp_path(path), current_status("online") {}
    
    ~CameraMDNSBroadcaster() {
        stop();
    }
    
    bool start() {
        int error;
        
        threaded_poll = avahi_threaded_poll_new();
        if(!threaded_poll) {
            cerr << "[mDNS] Failed to create threaded poll" << endl;
            return false;
        }
        
        client = avahi_client_new(
            avahi_threaded_poll_get(threaded_poll),
            static_cast<AvahiClientFlags>(0),
            client_callback,
            this,
            &error
        );
        
        if(!client) {
            cerr << "[mDNS] Failed to create client: " << avahi_strerror(error) << endl;
            return false;
        }
        
        avahi_threaded_poll_start(threaded_poll);
        
        cout << "[mDNS] Broadcaster started for camera: " << camera_id << endl;
        cout << "[mDNS]   Service: " << service_name << endl;
        cout << "[mDNS]   Type: _opensentry._tcp" << endl;
        cout << "[mDNS]   Port: " << rtsp_port << endl;
        
        return true;
    }
    
    void update_status(const string& status) {
        if(status == current_status)
            return;
            
        current_status = status;
        
        // Only update if mDNS is fully initialized
        if(group && client && threaded_poll) {
            avahi_threaded_poll_lock(threaded_poll);
            if(avahi_client_get_state(client) == AVAHI_CLIENT_S_RUNNING) {
                avahi_entry_group_reset(group);
                create_services();
                cout << "[mDNS] Status updated to: " << status << endl;
            }
            avahi_threaded_poll_unlock(threaded_poll);
        }
        // Silently skip if mDNS not ready - status will be set when service registers
    }
    
    void stop() {
        if(threaded_poll) {
            avahi_threaded_poll_stop(threaded_poll);
        }
        
        if(client) {
            avahi_client_free(client);
            client = nullptr;
        }
        
        if(threaded_poll) {
            avahi_threaded_poll_free(threaded_poll);
            threaded_poll = nullptr;
        }
        
        cout << "[mDNS] Broadcaster stopped" << endl;
    }
};

// Global mDNS broadcaster pointer for callbacks
CameraMDNSBroadcaster* g_mdns_broadcaster = nullptr;

// ============================================================================
// MQTT Callback Handler
// ============================================================================
class MQTTCallback : public virtual mqtt::callback {
public:
    void message_arrived(mqtt::const_message_ptr msg) override {
        string topic = msg->get_topic();
        string payload = msg->to_string();

        cout << "[MQTT] Received: " << topic << " = " << payload << endl;

        if (topic == "opensentry/" + CAMERA_ID + "/command") {
            if (payload == "start") {
                streaming = true;
                cout << "[MQTT] Starting stream" << endl;
                if(g_mdns_broadcaster) g_mdns_broadcaster->update_status("streaming");
            } else if (payload == "stop") {
                streaming = false;
                cout << "[MQTT] Stopping stream (paused)" << endl;
                if(g_mdns_broadcaster) g_mdns_broadcaster->update_status("idle");
            } else if (payload == "shutdown") {
                running = false;
                cout << "[MQTT] Shutting down" << endl;
                if(g_mdns_broadcaster) g_mdns_broadcaster->update_status("offline");
            }
        }
    }

    void connection_lost(const string& cause) override {
        cerr << "[MQTT] Connection lost: " << cause << endl;
        cerr << "[MQTT] Will attempt to reconnect..." << endl;
    }
    
    void connected(const string& cause) override {
        cout << "[MQTT] Reconnected" << endl;
    }
};

void mqtt_heartbeat_thread(mqtt::async_client& mqtt_client) {
    while (running) {
        if (mqtt_client.is_connected()) {
            string status = streaming ? "streaming" : "idle";
            mqtt_client.publish("opensentry/" + CAMERA_ID + "/status", status, 0, false);
        }
        this_thread::sleep_for(chrono::seconds(5));
    }
}

int main()
{
    // Initialize configuration from environment variables
    CAMERA_ID = getEnvOrDefault("CAMERA_ID", "camera1");
    CAMERA_NAME = getEnvOrDefault("CAMERA_NAME", "OpenSentry Camera 1");
    MQTT_SERVER = getEnvOrDefault("MQTT_SERVER", "tcp://localhost:1883");
    CLIENT_ID = "opensentry_node_" + CAMERA_ID;
    
    // Parse camera device - extract index from /dev/video0 format or use directly
    string cameraDeviceStr = getEnvOrDefault("CAMERA_DEVICE", "/dev/video0");
    if (cameraDeviceStr.find("/dev/video") == 0) {
        CAMERA_DEVICE_INDEX = stoi(cameraDeviceStr.substr(10));
    } else {
        CAMERA_DEVICE_INDEX = stoi(cameraDeviceStr);
    }
    
    cout << "========================================" << endl;
    cout << "  OpenSentry Camera Node - " << CAMERA_ID << endl;
    cout << "========================================" << endl;
    cout << "  Name: " << CAMERA_NAME << endl;
    cout << "  MQTT: " << MQTT_SERVER << endl;
    cout << "  Camera: /dev/video" << CAMERA_DEVICE_INDEX << endl;
    cout << "========================================" << endl;
    
    // Initialize mDNS broadcaster
    CameraMDNSBroadcaster mdns_broadcaster(
        CAMERA_ID,                    // Camera ID
        CAMERA_NAME,                  // Service name (friendly name)
        8554,                         // RTSP port
        CAMERA_ID                     // RTSP path
    );
    g_mdns_broadcaster = &mdns_broadcaster;
    
    bool mdns_available = mdns_broadcaster.start();
    if(!mdns_available) {
        cerr << "[WARNING] mDNS broadcaster failed to start - continuing without discovery" << endl;
        cerr << "[WARNING] Camera will still stream but won't be auto-discovered" << endl;
    }
    
    // Setup MQTT
    mqtt::async_client mqtt_client(MQTT_SERVER, CLIENT_ID);
    MQTTCallback callback;
    mqtt_client.set_callback(callback);

    mqtt::connect_options connOpts;
    connOpts.set_keep_alive_interval(20);
    connOpts.set_clean_session(true);
    connOpts.set_automatic_reconnect(true);  // Auto-reconnect on connection loss
    connOpts.set_automatic_reconnect(1, 30); // Min 1s, max 30s backoff

    bool mqtt_connected = false;
    try {
        cout << "[MQTT] Connecting to broker..." << endl;
        mqtt_client.connect(connOpts)->wait();
        cout << "[MQTT] Connected!" << endl;

        // Subscribe to command topic
        mqtt_client.subscribe("opensentry/" + CAMERA_ID + "/command", 1)->wait();
        cout << "[MQTT] Subscribed to commands" << endl;

        // Announce online
        mqtt_client.publish("opensentry/" + CAMERA_ID + "/status", "online", 0, false);
        if(mdns_available) mdns_broadcaster.update_status("online");
        mqtt_connected = true;

    } catch (const mqtt::exception& exc) {
        cerr << "[MQTT] Warning: " << exc.what() << endl;
        cerr << "[MQTT] Continuing without MQTT - no remote control available" << endl;
        if(mdns_available) mdns_broadcaster.update_status("streaming");
    }

    // Start heartbeat thread only if MQTT is connected
    thread heartbeat;
    if(mqtt_connected) {
        heartbeat = thread(mqtt_heartbeat_thread, ref(mqtt_client));
    }

    // Open camera
    cout << "[Camera] Opening camera device /dev/video" << CAMERA_DEVICE_INDEX << "..." << endl;
    VideoCapture camera(CAMERA_DEVICE_INDEX);

    if (!camera.isOpened()) {
        cerr << endl;
        cerr << "========================================" << endl;
        cerr << "  ERROR: Camera not found!" << endl;
        cerr << "========================================" << endl;
        cerr << "  Could not open /dev/video" << CAMERA_DEVICE_INDEX << endl;
        cerr << endl;
        cerr << "  Please check:" << endl;
        cerr << "    1. Is your USB camera plugged in?" << endl;
        cerr << "    2. Run: ls /dev/video*" << endl;
        cerr << "    3. Check permissions: sudo usermod -aG video $USER" << endl;
        cerr << endl;
        cerr << "========================================" << endl;
        cerr << endl;
        if(mqtt_connected) mqtt_client.publish("opensentry/" + CAMERA_ID + "/status", "error_no_camera", 0, false);
        if(mdns_available) mdns_broadcaster.update_status("error_no_camera");
        
        // Clean shutdown
        running = false;
        if(mqtt_connected) {
            mqtt_client.disconnect()->wait();
            heartbeat.join();
        }
        return -1;
    }

    // Get camera properties
    int width = static_cast<int>(camera.get(CAP_PROP_FRAME_WIDTH));
    int height = static_cast<int>(camera.get(CAP_PROP_FRAME_HEIGHT));
    int fps = 30;

    cout << "[Camera] Opened: " << width << "x" << height << endl;

    // Initialize FFmpeg
    avformat_network_init();

    // Create output format context for RTSP
    AVFormatContext *outFormatCtx = nullptr;
    const char *rtspURL = "rtsp://localhost:8554/camera1";

    avformat_alloc_output_context2(&outFormatCtx, nullptr, "rtsp", rtspURL);
    if (!outFormatCtx) {
        cerr << "[ERROR] Could not create output context" << endl;
        return -1;
    }

    // Find H.264 encoder
    const AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        cerr << "[ERROR] H.264 codec not found" << endl;
        return -1;
    }

    // Create video stream
    AVStream *outStream = avformat_new_stream(outFormatCtx, nullptr);
    if (!outStream) {
        cerr << "[ERROR] Failed to create stream" << endl;
        return -1;
    }

    // Create codec context
    AVCodecContext *codecCtx = avcodec_alloc_context3(codec);
    codecCtx->width = width;
    codecCtx->height = height;
    codecCtx->time_base = {1, fps};
    codecCtx->framerate = {fps, 1};
    codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    codecCtx->codec_type = AVMEDIA_TYPE_VIDEO;

    av_opt_set(codecCtx->priv_data, "preset", "ultrafast", 0);
    av_opt_set(codecCtx->priv_data, "tune", "zerolatency", 0);

    if (outFormatCtx->oformat->flags & AVFMT_GLOBALHEADER) {
        codecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
        cerr << "[ERROR] Could not open codec" << endl;
        return -1;
    }

    avcodec_parameters_from_context(outStream->codecpar, codecCtx);
    outStream->time_base = codecCtx->time_base;

    if (!(outFormatCtx->oformat->flags & AVFMT_NOFILE)) {
        int ret = avio_open(&outFormatCtx->pb, rtspURL, AVIO_FLAG_WRITE);
        if (ret < 0) {
            cerr << endl;
            cerr << "========================================" << endl;
            cerr << "  ERROR: Cannot connect to RTSP server!" << endl;
            cerr << "========================================" << endl;
            cerr << "  Could not connect to: " << rtspURL << endl;
            cerr << endl;
            cerr << "  Please check:" << endl;
            cerr << "    1. Is MediaMTX running?" << endl;
            cerr << "       Start it with: ./mediamtx" << endl;
            cerr << "    2. Is port 8554 available?" << endl;
            cerr << "       Check with: netstat -tlnp | grep 8554" << endl;
            cerr << endl;
            cerr << "========================================" << endl;
            cerr << endl;
            
            // Clean shutdown
            if(mqtt_connected) mqtt_client.publish("opensentry/" + CAMERA_ID + "/status", "error_no_rtsp_server", 0, false);
            if(mdns_available) mdns_broadcaster.update_status("error");
            running = false;
            if(mqtt_connected) heartbeat.join();
            camera.release();
            avcodec_free_context(&codecCtx);
            avformat_free_context(outFormatCtx);
            if(mqtt_connected) mqtt_client.disconnect()->wait();
            if(mdns_available) mdns_broadcaster.stop();
            return -1;
        }
    }

    int headerRet = avformat_write_header(outFormatCtx, nullptr);
    if (headerRet < 0) {
        cerr << endl;
        cerr << "========================================" << endl;
        cerr << "  ERROR: Failed to initialize stream!" << endl;
        cerr << "========================================" << endl;
        cerr << "  Could not write RTSP header to: " << rtspURL << endl;
        cerr << endl;
        cerr << "  This usually means MediaMTX rejected" << endl;
        cerr << "  the connection or isn't running." << endl;
        cerr << endl;
        cerr << "  Please ensure MediaMTX is running:" << endl;
        cerr << "    ./mediamtx" << endl;
        cerr << endl;
        cerr << "========================================" << endl;
        cerr << endl;
        
        // Clean shutdown
        if(mqtt_connected) mqtt_client.publish("opensentry/" + CAMERA_ID + "/status", "error_stream_init", 0, false);
        if(mdns_available) mdns_broadcaster.update_status("error");
        running = false;
        if(mqtt_connected) heartbeat.join();
        camera.release();
        avcodec_free_context(&codecCtx);
        if (outFormatCtx->pb) avio_closep(&outFormatCtx->pb);
        avformat_free_context(outFormatCtx);
        if(mqtt_connected) mqtt_client.disconnect()->wait();
        if(mdns_available) mdns_broadcaster.stop();
        return -1;
    }

    cout << "[Stream] Streaming to: " << rtspURL << endl;
    if(mqtt_connected) mqtt_client.publish("opensentry/" + CAMERA_ID + "/status", "streaming", 0, false);
    if(mdns_available) mdns_broadcaster.update_status("streaming");

    AVFrame *frame = av_frame_alloc();
    frame->format = codecCtx->pix_fmt;
    frame->width = codecCtx->width;
    frame->height = codecCtx->height;
    av_frame_get_buffer(frame, 0);

    SwsContext *swsCtx = sws_getContext(
        width, height, AV_PIX_FMT_BGR24,
        width, height, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    AVPacket *pkt = av_packet_alloc();
    Mat cvFrame;
    Mat lastFrame;  // Store last frame for pause state
    int64_t frameNum = 0;

    while (running) {
        camera >> cvFrame;

        if (cvFrame.empty()) {
            cerr << "[ERROR] Empty frame" << endl;
            break;
        }

        // Store the current frame for pause state
        if (streaming) {
            lastFrame = cvFrame.clone();
        }

        // Always encode and send frames to keep RTSP connection alive
        // When paused, send the last captured frame (frozen image)
        Mat& frameToSend = streaming ? cvFrame : lastFrame;
        
        if (!frameToSend.empty()) {
            const int stride[] = {static_cast<int>(frameToSend.step[0])};
            sws_scale(swsCtx, &frameToSend.data, stride, 0, height, frame->data, frame->linesize);

            frame->pts = frameNum++;

            int ret = avcodec_send_frame(codecCtx, frame);
            if (ret < 0) {
                cerr << "[ERROR] Error sending frame" << endl;
                break;
            }

            while (ret >= 0) {
                ret = avcodec_receive_packet(codecCtx, pkt);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    cerr << "[ERROR] Error encoding" << endl;
                    goto cleanup;
                }

                av_packet_rescale_ts(pkt, codecCtx->time_base, outStream->time_base);
                pkt->stream_index = outStream->index;

                ret = av_interleaved_write_frame(outFormatCtx, pkt);
                av_packet_unref(pkt);

                if (ret < 0) {
                    cerr << "[ERROR] Error writing frame" << endl;
                    goto cleanup;
                }
            }
        }

        // GUI display only if DISPLAY environment variable is set (not in Docker/headless)
        if (getenv("DISPLAY") != nullptr) {
            imshow("OpenSentry Camera Node", cvFrame);
            if (waitKey(1) == 'q') {
                running = false;
                break;
            }
        } else {
            // Small delay in headless mode to prevent CPU spinning
            this_thread::sleep_for(chrono::milliseconds(1));
        }
    }

cleanup:
    // Update status to offline
    if(mdns_available) mdns_broadcaster.update_status("offline");
    if(mqtt_connected) {
        mqtt_client.publish("opensentry/" + CAMERA_ID + "/status", "offline", 0, false);
        mqtt_client.disconnect()->wait();
        heartbeat.join();
    }

    av_write_trailer(outFormatCtx);
    av_packet_free(&pkt);
    av_frame_free(&frame);
    sws_freeContext(swsCtx);
    avcodec_free_context(&codecCtx);

    if (!(outFormatCtx->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&outFormatCtx->pb);
    }

    avformat_free_context(outFormatCtx);
    camera.release();
    destroyAllWindows();

    cout << "[System] Streaming stopped" << endl;

    return 0;
}