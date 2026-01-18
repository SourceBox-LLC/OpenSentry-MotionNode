#!/bin/bash
set -e

# ============================================================================
# Auto-generate Camera ID from hostname if not explicitly set
# ============================================================================
# This ensures each device gets a unique camera ID by default
if [ -z "$CAMERA_ID" ] || [ "$CAMERA_ID" = "camera1" ]; then
    CAMERA_ID="$(hostname)"
    export CAMERA_ID
    echo "[OpenSentry Node] Auto-generated CAMERA_ID from hostname: $CAMERA_ID"
fi

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
# SSL Certificate Generation for RTSPS
# ============================================================================
CERT_DIR="/app/certs"
CERT_FILE="$CERT_DIR/server.crt"
KEY_FILE="$CERT_DIR/server.key"

if [ ! -f "$CERT_FILE" ] || [ ! -f "$KEY_FILE" ]; then
    echo "[OpenSentry Node] 🔐 Generating SSL certificates for RTSPS..."
    mkdir -p "$CERT_DIR"
    
    openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
        -keyout "$KEY_FILE" \
        -out "$CERT_FILE" \
        -subj "/C=US/ST=Local/L=Local/O=OpenSentry/OU=Camera/CN=$CAMERA_ID" \
        -addext "subjectAltName=DNS:$CAMERA_ID,DNS:localhost,IP:127.0.0.1" \
        2>/dev/null
    
    chmod 644 "$KEY_FILE"
    chmod 644 "$CERT_FILE"
    echo "[OpenSentry Node] ✅ SSL certificates generated"
else
    echo "[OpenSentry Node] ✅ SSL certificates found"
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

# Connect to the Command Center's MQTT broker
# Use tcp:// for non-TLS connection on port 1883
export MQTT_SERVER="tcp://localhost:1883"
echo "[OpenSentry Node] Using Command Center's MQTT broker (${MQTT_SERVER})"

# Generate MediaMTX config with RTSPS encryption
cat > /tmp/mediamtx.yml << EOF
logLevel: info
logDestinations: [stdout]

# RTSP/RTSPS settings
rtspAddress: :8554
rtspsAddress: :8322
rtspTransports: [tcp]
rtspEncryption: optional
rtspServerKey: ${KEY_FILE}
rtspServerCert: ${CERT_FILE}

# Disable other protocols
rtmp: no
hls: no
webrtc: no

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
    exit 0
}
trap cleanup SIGTERM SIGINT

# Start the OpenSentry Node application
exec OpenSentry_Node
