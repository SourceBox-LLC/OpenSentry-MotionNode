# 🎯 OpenSentry Motion Detection Node

**Turn any USB webcam into an intelligent security camera with real-time motion detection.**

**🔒 Fully Encrypted:** RTSPS video streams, MQTT over TLS  
**🎯 Motion Detection:** OpenCV-powered movement detection with instant alerts

---

## 🚀 Quick Start

### Step 1: Install Docker

```bash
curl -fsSL https://get.docker.com | sh
sudo usermod -aG docker $USER
```
Then log out and back in.

---

### Step 2: Plug In Your Camera

Connect a USB webcam to your device.

---

### Step 3: Download the Project

```bash
git clone https://github.com/SourceBox-LLC/OpenSentry-MotionNode.git
cd OpenSentry-MotionNode
```

---

### Step 4: Run Setup

```bash
chmod +x setup.sh && ./setup.sh
```

You'll see:
```
╔═══════════════════════════════════════════════════════════════╗
║           OpenSentry Camera Node - Quick Setup                ║
╚═══════════════════════════════════════════════════════════════╝

✅ Docker found

🎥 Detecting cameras...
   Found: /dev/video0 /dev/video1
```

---

### Step 5: Name Your Camera

Choose a friendly name for the dashboard:

```
📝 Let's configure your Camera Node...

Camera name [raspberrypi]: Front Door
Camera device [/dev/video0]: 
```

---

### Step 6: Enter Security Secret

Paste the `OPENSENTRY_SECRET` from your Command Center:

1. Log into Command Center → Click **⚙️ Settings** → Copy the secret

```
🔐 Enter the OPENSENTRY_SECRET from your Command Center.
   (Go to Settings in the Command Center dashboard to find it)

OPENSENTRY_SECRET: 7de776c167242fbf10da85c3d182a9fb...
```

---

### Step 7: Done!

The camera node builds and starts automatically:

```
🚀 Building and starting Camera Node...

╔═══════════════════════════════════════════════════════════════╗
║                    Setup Complete!                            ║
╠═══════════════════════════════════════════════════════════════╣
║  Camera Name:  Front Door                                     ║
║  Device:       /dev/video0                                    ║
║  Stream:       RTSPS (encrypted) on port 8322                 ║
║  MQTT:         TLS (encrypted) on port 8883                   ║
╠═══════════════════════════════════════════════════════════════╣
║  Your camera should appear in the Command Center              ║
║  dashboard within 30 seconds.                                 ║
╠═══════════════════════════════════════════════════════════════╣
║  Commands:                                                    ║
║    View logs:    docker compose logs -f                       ║
║    Stop:         docker compose down                          ║
║    Restart:      docker compose restart                       ║
╚═══════════════════════════════════════════════════════════════╝
```

**Check your [Command Center](https://github.com/SourceBox-LLC/OpenSentry-Command) dashboard - your camera will appear as a Motion Detection Node!** 🎯

---

## 🎯 Command Center Integration

When connected to OpenSentry Command Center, this Motion Node provides:

### Automatic Features
1. **Node Type Display**: Shows as "🎯 Motion Detection Node" 
2. **Motion Alerts**: Real-time notifications when motion detected
3. **Motion History**: Click history button to view recent events
4. **Visual Indicators**: Red badge and animations during motion
5. **Smart Notifications**: Configurable alerts in Settings

### Motion History
The Command Center stores the last 100 motion events per camera:
- Timestamp of each event
- Duration of motion
- Location and size of motion area
- Accessible via Motion History button

### Notification Settings
Users can configure in Command Center Settings:
- **Motion Detection Alerts**: Toggle on/off
- **Toast Notifications**: Enable/disable pop-ups
- Preferences saved per browser

---

## 🍓 Raspberry Pi

Works great on **Pi 4** and **Pi 5**. Use 64-bit Raspberry Pi OS.

```bash
# SSH into your Pi
ssh pi@raspberrypi.local

# Install Docker
curl -fsSL https://get.docker.com | sh
sudo usermod -aG docker pi
sudo reboot

# After reboot, follow Steps 3-7 above
```

---

## 🔧 Common Commands

```bash
# View logs
docker compose logs -f

# Stop
docker compose down

# Restart
docker compose restart

# Rebuild after changes
docker compose up --build -d

# Complete teardown
chmod +x teardown.sh && ./teardown.sh
```

---

## ❓ Troubleshooting

| Problem | Solution |
|---------|----------|
| **No camera found** | Replug USB camera. Run `ls /dev/video*` to check. |
| **Not in Command Center** | Same WiFi? Same `OPENSENTRY_SECRET`? Wait 30 sec. |
| **Permission denied** | Run `sudo usermod -aG video $USER` then re-login. |
| **Laggy video** | Use Ethernet instead of WiFi. Try Pi 4/5. |

**Check logs:** `docker compose logs -f`

---

## ⚙️ Configuration

Edit `.env` file (created by setup):

```bash
# Camera Settings
CAMERA_NAME=Front Door
CAMERA_DEVICE=/dev/video0
CAMERA_ID=front-door-motion

# Security (must match Command Center!)
OPENSENTRY_SECRET=your-secret-from-command-center

# Network
COMMAND_CENTER_HOST=localhost

# Motion Detection Settings
MOTION_THRESHOLD=25          # Sensitivity (10-50, lower = more sensitive)
MOTION_MIN_AREA=500          # Minimum pixels to trigger
MOTION_COOLDOWN=2            # Seconds between events

# Node Identification
NODE_TYPE=motion
CAPABILITIES=streaming,motion_detection
```

After changes: `docker compose down && docker compose up -d`

### Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `MOTION_THRESHOLD` | 25 | Motion sensitivity (10-50) |
| `MOTION_MIN_AREA` | 500 | Minimum motion area in pixels |
| `MOTION_COOLDOWN` | 2 | Delay between motion events |
| `NODE_TYPE` | motion | Identifies as motion node |
| `CAPABILITIES` | streaming,motion_detection | Node features |

---

## 🔒 Security Features

All communications are encrypted:

| Protocol | Port | Description |
|----------|------|-------------|
| **RTSPS** | 8322 | Video stream with TLS encryption |
| **MQTTS** | 8883 | Control commands with TLS encryption |
| **RTSP** | 1884 | Internal only (localhost) |

### How It Works

1. **SSL certificates** are auto-generated on first run
2. **Credentials** are derived from the shared `OPENSENTRY_SECRET`
3. **Video streams** are encrypted using RTSPS (RTSP over TLS)
4. **Commands** (start/stop/shutdown) are sent over encrypted MQTT

### Input Validation

The camera node validates all incoming commands:
- Payload size limit (64 bytes max)
- Character whitelist (alphanumeric only)
- Command whitelist (start, stop, shutdown)

---

## 🎯 Advanced Motion Detection Features

This Motion Node extends the basic [OpenSentry Node](https://github.com/SourceBox-LLC/OpenSentry-Node) with intelligent motion detection capabilities.

### Key Differences from Basic Node

| Feature | Basic Node | Motion Node |
|---------|------------|-------------|
| **Node Type** | "basic" | "motion" |
| **Capabilities** | "streaming" | "streaming,motion_detection" |
| **Motion Events** | ❌ | ✅ Real-time JSON events |
| **Motion History** | ❌ | ✅ Stored in Command Center |
| **Visual Alerts** | ❌ | ✅ Red badge on feed |
| **Notifications** | ❌ | ✅ Toast alerts |

### How Motion Detection Works

1. **Frame Analysis** - OpenCV analyzes each video frame for movement
2. **Threshold Detection** - Movement exceeding sensitivity triggers an event
3. **Event Publishing** - JSON motion events sent via MQTT
4. **Visual Feedback** - Red bounding box drawn around motion areas
5. **Command Center Integration** - Automatic alerts and history tracking

### Motion Event Format

**Motion Start Event:**
```json
{
  "event": "motion_start",
  "timestamp": 1234567890,
  "area_x": 100,
  "area_y": 200,
  "area_width": 300,
  "area_height": 400
}
```

**Motion End Event:**
```json
{
  "event": "motion_end",
  "timestamp": 1234567891,
  "duration": 5
}
```

### MQTT Topics

| Topic | Purpose | Example Payload |
|-------|---------|-----------------|
| `opensentry/{id}/status` | Node health & type | `{"status": "streaming", "node_type": "motion", "capabilities": "streaming,motion_detection"}` |
| `opensentry/{id}/motion` | Motion events | `{"event": "motion_start", "timestamp": 1234567890}` |
| `opensentry/{id}/command` | Control commands | `start`, `stop`, `shutdown` |

### Visual Indicators in Command Center

When motion is detected:
- **Red Badge**: "🔴 MOTION DETECTED!" appears on video feed
- **Card Animation**: Camera card pulses with red glow
- **Toast Notification**: Pop-up alert (if enabled)
- **Motion History**: Event logged with timestamp

### Tuning Motion Sensitivity

Edit these values in `src/main.cpp`:

| Parameter | Line | Default | Description |
|-----------|------|---------|-------------|
| `threshold` | ~636 | 25 | Sensitivity (lower = more sensitive) |
| `contourArea` | ~645 | 500 | Minimum motion area in pixels |
| `GaussianBlur` | ~631 | 21x21 | Noise reduction kernel size |

**Recommended Settings:**
- **Indoor**: threshold=20, area=500 (detect people, pets)
- **Outdoor**: threshold=30, area=1000 (ignore wind, small animals)
- **High Security**: threshold=15, area=300 (maximum sensitivity)

---

## 🪟 Windows Setup (WSL)

Windows users can run camera nodes using WSL (Windows Subsystem for Linux).

### Step 1: Install WSL

Open **PowerShell as Administrator** and run:

```powershell
wsl --install
```

Restart your computer when prompted.

### Step 2: Set Up Ubuntu

After restart, Ubuntu will open automatically. Create a username and password when asked.

### Step 3: Install Docker in WSL

In the Ubuntu terminal:

```bash
curl -fsSL https://get.docker.com | sh
sudo usermod -aG docker $USER
```

Close and reopen Ubuntu.

### Step 4: Connect USB Camera to WSL

WSL needs extra setup to access USB devices. Install `usbipd`:

**In PowerShell (as Administrator):**
```powershell
winget install usbipd
```

**In Ubuntu:**
```bash
sudo apt install linux-tools-generic hwdata
sudo update-alternatives --install /usr/local/bin/usbip usbip /usr/lib/linux-tools/*-generic/usbip 20
```

**Attach your camera:**
```powershell
# List USB devices (in PowerShell)
usbipd list

# Attach camera to WSL (replace BUSID with your camera's ID)
usbipd bind --busid <BUSID>
usbipd attach --wsl --busid <BUSID>
```

### Step 5: Follow Quick Start

Now follow **Steps 3-7** from the [Quick Start](#-quick-start) section above.

```bash
git clone https://github.com/SourceBox-LLC/OpenSentry-MotionNode.git
cd OpenSentry-MotionNode
chmod +x setup.sh && ./setup.sh
```

> **Note:** For easier USB camera access, consider running the Camera Node on a Raspberry Pi or dedicated Linux machine instead.

---

## 🗑️ Uninstall

To completely remove the Camera Node:

```bash
chmod +x teardown.sh && ./teardown.sh
```

You'll be prompted to remove:
- Docker containers and images
- Configuration files

---

## 🔄 Comparison with Basic Node

### When to Use Motion Node
- Security-critical areas
- Intrusion detection needed
- Want motion alerts and history
- Recording triggered by motion

### When to Use Basic Node
- Simple monitoring
- Always-on recording
- Low-power devices
- Motion detection not needed

### Resource Usage

| Metric | Basic Node | Motion Node |
|--------|------------|-------------|
| **CPU Usage** | ~10-15% | ~20-30% |
| **Memory** | ~100MB | ~150MB |
| **Network** | Video only | Video + Motion events |
| **Docker Image** | ~200MB | ~250MB |

## 🛠️ Development

### Building from Source

```bash
# Prerequisites
sudo apt-get install cmake libopencv-dev libpaho-mqtt-dev libavahi-client-dev

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)
./opensentry-node
```

### Project Structure
```
OpenSentry-MotionNode/
├── src/main.cpp              # Motion detection logic
├── CMakeLists.txt           # Build configuration
├── Dockerfile               # Container definition
├── docker-compose.yml       # Service orchestration
├── docker-entrypoint.sh     # Startup script
└── setup.sh                 # Installation wizard
```

## 📜 License

MIT - Free for personal and commercial use.

## 🔗 Related Projects

- **[OpenSentry Command Center](https://github.com/SourceBox-LLC/OpenSentry-Command)** - Central dashboard with motion alerts
- **[OpenSentry Node](https://github.com/SourceBox-LLC/OpenSentry-Node)** - Basic camera node without motion detection
- **[OpenSentry AI](https://github.com/SourceBox-LLC/OpenSentry-AI)** - AI-powered object detection (coming soon)

---

**Built with ❤️ and OpenCV by the OpenSentry Team**
