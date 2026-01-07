# OpenSentry Camera Node

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Docker](https://img.shields.io/badge/Docker-Ready-blue.svg)](https://www.docker.com/)
[![C++](https://img.shields.io/badge/C++-17-00599C.svg)](https://isocpp.org/)

A self-contained camera node for the OpenSentry surveillance system. Each node captures video from a local camera and streams it via RTSP, with automatic discovery via mDNS and control via MQTT.

> **Companion Project:** [OpenSentry Command Center](https://github.com/yourusername/opensentry-command) - Web-based monitoring and control interface

## Features

- **RTSP Streaming** - H.264 encoded video stream via bundled MediaMTX
- **mDNS Discovery** - Automatic detection by OpenSentry Command Center
- **MQTT Control** - Remote start/stop/shutdown commands (bundled Mosquitto broker)
- **Fully Self-Contained** - Docker deployment with MediaMTX + Mosquitto bundled
- **Multi-Architecture** - Auto-detects amd64, arm64 (Raspberry Pi 4/5), armv7

## Quick Start (Docker)

### Prerequisites
- Docker and Docker Compose installed
- USB webcam or other V4L2-compatible camera
- No external MQTT broker needed (Mosquitto is bundled)

### 1. Configure

Copy the example environment file and customize:

```bash
cp .env.example .env
```

Edit `.env`:
```bash
CAMERA_ID=camera1
CAMERA_NAME=Living Room Camera
MQTT_SERVER=tcp://192.168.1.100:1883
CAMERA_DEVICE=/dev/video0
```

### 2. Build and Run

```bash
docker-compose up -d
```

### 3. View Logs

```bash
docker-compose logs -f
```

## Running Multiple Nodes

For multiple cameras on the same host, create separate directories or use different `.env` files:

```bash
# Camera 1
CAMERA_ID=camera1 RTSP_PORT=8554 docker-compose up -d

# Camera 2 (different terminal/directory)
CAMERA_ID=camera2 RTSP_PORT=8555 CAMERA_DEVICE=/dev/video1 docker-compose up -d
```

## Raspberry Pi Deployment

The Docker image **automatically detects** ARM64 architecture. Just run:

```bash
docker compose up -d --build
```

Supported architectures:
- **amd64** - Standard x86_64 (Intel/AMD)
- **arm64v8** - Raspberry Pi 4, Pi 5, ARM servers
- **armv7** - Raspberry Pi 3, older ARM devices

## Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `CAMERA_ID` | `camera1` | Unique identifier for this camera |
| `CAMERA_NAME` | `OpenSentry Camera` | Display name in Command Center |
| `MQTT_SERVER` | `tcp://localhost:1883` | MQTT broker address |
| `CAMERA_DEVICE` | `/dev/video0` | Camera device path |
| `RTSP_PORT` | `8554` | RTSP server port |

## Ports

| Port | Protocol | Description |
|------|----------|-------------|
| 1883 | MQTT | Bundled Mosquitto broker |
| 8554 | RTSP | Main video stream |
| 1935 | RTMP | Optional RTMP stream |
| 8888 | HTTP | HLS stream (optional) |
| 8889 | HTTP | WebRTC (optional) |

## Native Build (Without Docker)

### Dependencies

Ubuntu/Debian:
```bash
sudo apt-get install -y \
    build-essential cmake pkg-config \
    libopencv-dev \
    libavcodec-dev libavformat-dev libavutil-dev libswscale-dev \
    libavahi-client-dev \
    libpaho-mqtt-dev libpaho-mqttpp-dev
```

### Build

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Run

First, start MediaMTX:
```bash
./mediamtx
```

Then run the node:
```bash
CAMERA_ID=camera1 CAMERA_NAME="My Camera" ./OpenSentry_Node
```

## Architecture

```
┌─────────────────────────────────────────┐
│           OpenSentry Node               │
│  ┌─────────────┐    ┌───────────────┐   │
│  │   Camera    │───▶│   FFmpeg      │   │
│  │  Capture    │    │   Encoder     │   │
│  └─────────────┘    └───────┬───────┘   │
│                             │           │
│  ┌─────────────┐    ┌───────▼───────┐   │
│  │    mDNS     │    │   MediaMTX    │   │
│  │ Broadcaster │    │  RTSP Server  │   │
│  └──────┬──────┘    └───────┬───────┘   │
│         │                   │           │
│  ┌──────▼──────┐            │           │
│  │    MQTT     │            │           │
│  │   Client    │            │           │
│  └─────────────┘            │           │
└─────────┼───────────────────┼───────────┘
          │                   │
          ▼                   ▼
    Command Center      RTSP Clients
```

## Troubleshooting

### Camera not found
```bash
# Check available cameras
ls /dev/video*

# Add user to video group
sudo usermod -aG video $USER
```

### MQTT connection failed
- Verify MQTT broker is running
- Check `MQTT_SERVER` address is reachable
- For Docker, use `host.docker.internal` or actual IP (not `localhost`)

### mDNS not discovered
- Ensure `network_mode: host` in docker-compose.yml
- Check Avahi daemon is running
- Verify firewall allows mDNS (UDP port 5353)

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

MIT License - see [LICENSE](LICENSE) for details.
