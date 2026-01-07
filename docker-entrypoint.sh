#!/bin/bash
set -e

# Start D-Bus daemon (required for Avahi)
mkdir -p /var/run/dbus
dbus-daemon --system --fork 2>/dev/null || true
sleep 1

# Start Avahi daemon for mDNS
mkdir -p /var/run/avahi-daemon
avahi-daemon -D 2>/dev/null || echo "[Warning] Avahi daemon failed to start"
sleep 1

echo "[OpenSentry Node] Starting MediaMTX RTSP server..."
mediamtx /etc/mediamtx.yml &
MEDIAMTX_PID=$!

# Wait for MediaMTX to be ready
sleep 2

# Check if MediaMTX started successfully
if ! kill -0 $MEDIAMTX_PID 2>/dev/null; then
    echo "[OpenSentry Node] ERROR: MediaMTX failed to start"
    exit 1
fi

echo "[OpenSentry Node] MediaMTX started (PID: $MEDIAMTX_PID)"
echo "[OpenSentry Node] Starting camera node..."
echo "[OpenSentry Node] Camera ID: $CAMERA_ID"
echo "[OpenSentry Node] Camera Name: $CAMERA_NAME"
echo "[OpenSentry Node] MQTT Server: $MQTT_SERVER"
echo "[OpenSentry Node] Camera Device: $CAMERA_DEVICE"

# Handle shutdown gracefully
cleanup() {
    echo "[OpenSentry Node] Shutting down..."
    kill $MEDIAMTX_PID 2>/dev/null || true
    exit 0
}
trap cleanup SIGTERM SIGINT

# Start the OpenSentry Node application
exec OpenSentry_Node
