#!/bin/bash
set -e

# ============================================================================
# Credential Derivation from Single Secret
# ============================================================================
# If OPENSENTRY_SECRET is set, derive all credentials from it
# Otherwise fall back to individual env vars or defaults
derive_credential() {
    local secret="$1"
    local service="$2"
    # Derive a 32-char password from secret + service name
    echo -n "${secret}:${service}" | sha256sum | cut -c1-32
}

if [ -n "$OPENSENTRY_SECRET" ]; then
    echo "[OpenSentry Node] Using derived credentials from OPENSENTRY_SECRET"
    MQTT_USER="opensentry"
    MQTT_PASS=$(derive_credential "$OPENSENTRY_SECRET" "mqtt")
    export RTSP_USERNAME="opensentry"
    export RTSP_PASSWORD=$(derive_credential "$OPENSENTRY_SECRET" "rtsp")
else
    echo "[OpenSentry Node] Using individual credentials (legacy mode)"
    MQTT_USER="${MQTT_USERNAME:-opensentry}"
    MQTT_PASS="${MQTT_PASSWORD:-opensentry}"
    export RTSP_USERNAME="${RTSP_USERNAME:-opensentry}"
    export RTSP_PASSWORD="${RTSP_PASSWORD:-opensentry}"
fi

# ============================================================================
# Service Startup
# ============================================================================

# Start D-Bus daemon (required for Avahi)
mkdir -p /var/run/dbus
dbus-daemon --system --fork 2>/dev/null || true
sleep 1

# Start Avahi daemon for mDNS
mkdir -p /var/run/avahi-daemon
avahi-daemon -D 2>/dev/null || echo "[Warning] Avahi daemon failed to start"
sleep 1

# Generate MQTT password file
echo "[OpenSentry Node] Setting up MQTT authentication..."
# Create password file using mosquitto_passwd
touch /etc/mosquitto/passwd
mosquitto_passwd -b /etc/mosquitto/passwd "$MQTT_USER" "$MQTT_PASS"
echo "[OpenSentry Node] MQTT user '$MQTT_USER' configured"

# Start Mosquitto MQTT broker
echo "[OpenSentry Node] Starting Mosquitto MQTT broker..."
mosquitto -c /etc/mosquitto/mosquitto.conf &
MOSQUITTO_PID=$!
sleep 1

# Check if Mosquitto started
if kill -0 $MOSQUITTO_PID 2>/dev/null; then
    echo "[OpenSentry Node] Mosquitto started (PID: $MOSQUITTO_PID)"
else
    echo "[Warning] Mosquitto failed to start - continuing without local MQTT"
fi

# Generate MediaMTX config with credentials baked in (env var substitution can be unreliable)
cat > /tmp/mediamtx.yml << EOF
logLevel: info
logDestinations: [stdout]
rtsp: yes
protocols: [tcp]
rtspAddress: :8554
rtmp: yes
rtmpAddress: :1935
hls: yes
hlsAddress: :8888
webrtc: yes
webrtcAddress: :8889
paths:
  all:
    source: publisher
    readUser: "${RTSP_USERNAME}"
    readPass: "${RTSP_PASSWORD}"
    publishIPs:
      - 127.0.0.1
      - "::1"
EOF

echo "[OpenSentry Node] RTSP authentication enabled for user '$RTSP_USERNAME'"

echo "[OpenSentry Node] Starting MediaMTX RTSP server..."
mediamtx /tmp/mediamtx.yml &
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
    kill $MOSQUITTO_PID 2>/dev/null || true
    exit 0
}
trap cleanup SIGTERM SIGINT

# Start the OpenSentry Node application
exec OpenSentry_Node
