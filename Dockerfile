# OpenSentry Node - Dockerfile
# Bundles the camera node application with MediaMTX RTSP server

# Build stage
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libopencv-dev \
    libavcodec-dev \
    libavformat-dev \
    libavutil-dev \
    libswscale-dev \
    libavahi-client-dev \
    libavahi-compat-libdnssd-dev \
    libpaho-mqtt-dev \
    libpaho-mqttpp-dev \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Copy source code
WORKDIR /app
COPY CMakeLists.txt .
COPY src/ src/

# Build the application
RUN mkdir build && cd build && \
    cmake .. && \
    make -j$(nproc)

# Runtime stage
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libopencv-core4.5d \
    libopencv-videoio4.5d \
    libopencv-imgproc4.5d \
    libopencv-imgcodecs4.5d \
    libopencv-highgui4.5d \
    libavcodec58 \
    libavformat58 \
    libavutil56 \
    libswscale5 \
    libavahi-client3 \
    libavahi-common3 \
    libpaho-mqtt1.3 \
    libpaho-mqttpp3-1 \
    libssl3 \
    avahi-daemon \
    dbus \
    mosquitto \
    mosquitto-clients \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Download and install MediaMTX
# Auto-detect architecture: map Docker's TARGETPLATFORM to MediaMTX naming
ARG MEDIAMTX_VERSION=1.11.0
ARG TARGETARCH
RUN ARCH=$(dpkg --print-architecture) && \
    echo "Detected architecture: $ARCH" && \
    if [ "$ARCH" = "arm64" ] || [ "$ARCH" = "aarch64" ]; then \
        MTX_ARCH="arm64v8"; \
    elif [ "$ARCH" = "armhf" ] || [ "$ARCH" = "armv7l" ]; then \
        MTX_ARCH="armv7"; \
    else \
        MTX_ARCH="amd64"; \
    fi && \
    echo "Downloading MediaMTX for: $MTX_ARCH" && \
    wget -q https://github.com/bluenviron/mediamtx/releases/download/v${MEDIAMTX_VERSION}/mediamtx_v${MEDIAMTX_VERSION}_linux_${MTX_ARCH}.tar.gz \
    && tar -xzf mediamtx_v${MEDIAMTX_VERSION}_linux_${MTX_ARCH}.tar.gz -C /usr/local/bin mediamtx \
    && rm mediamtx_v${MEDIAMTX_VERSION}_linux_${MTX_ARCH}.tar.gz \
    && chmod +x /usr/local/bin/mediamtx

# Copy built application from builder stage
COPY --from=builder /app/build/OpenSentry_Node /usr/local/bin/

# Copy entrypoint script
COPY docker-entrypoint.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/docker-entrypoint.sh

# Copy MediaMTX config
COPY mediamtx.yml /etc/mediamtx.yml

# Copy Mosquitto config
COPY mosquitto.conf /etc/mosquitto/mosquitto.conf

# Expose ports
# 1883 - MQTT (localhost only, internal)
# 8883 - MQTTS (TLS encrypted, external)
# 8554 - RTSP
# 8322 - RTSPS (encrypted)
# 1935 - RTMP (optional)
# 8888 - HLS (optional)
# 8889 - WebRTC (optional)
EXPOSE 8883 8554 8322 1935 8888 8889

# Environment variables for configuration
ENV CAMERA_ID=camera1
ENV CAMERA_NAME="OpenSentry Camera"
ENV MQTT_SERVER=tcp://localhost:1884
ENV CAMERA_DEVICE=/dev/video0

ENTRYPOINT ["docker-entrypoint.sh"]
