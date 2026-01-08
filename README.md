# 📷 OpenSentry Camera Node

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Docker](https://img.shields.io/badge/Docker-Ready-2496ED.svg)](https://www.docker.com/)
[![Raspberry Pi](https://img.shields.io/badge/Raspberry_Pi-Ready-C51A4A.svg)](https://www.raspberrypi.org/)

**Turn any USB webcam into a smart security camera that streams to your OpenSentry Command Center.**

---

## 📖 Table of Contents

1. [What is OpenSentry Node?](#-what-is-opensentry-node)
2. [What You Need](#-what-you-need)
3. [Quick Start Guide (10 Minutes)](#-quick-start-guide-10-minutes)
4. [Raspberry Pi Setup](#-raspberry-pi-setup)
5. [Setting Up Security](#-setting-up-security)
6. [Troubleshooting](#-troubleshooting)
7. [Advanced Configuration](#-advanced-configuration)
8. [Getting Help](#-getting-help)

---

## 🎯 What is OpenSentry Node?

OpenSentry Node turns a regular USB webcam into a network security camera. It:

- ✅ **Captures video** from any USB webcam
- ✅ **Streams video** so the Command Center can view it
- ✅ **Announces itself** automatically on the network (no manual IP setup!)
- ✅ **Accepts commands** from the Command Center (start, stop, shutdown)

### How It Fits Together

```
┌──────────────────┐         ┌────────────────────┐         ┌───────────┐
│   This Project   │         │   Command Center   │         │    You    │
│ (Camera Node)    │ ──────► │   (Dashboard)      │ ──────► │  (Browser)│
│                  │         │                    │         │           │
│  📷 → 📡         │  WiFi   │      🖥️            │         │    👁️     │
│  Captures &      │         │  Collects all      │         │  View all │
│  Broadcasts      │         │  camera feeds      │         │  cameras  │
└──────────────────┘         └────────────────────┘         └───────────┘
```

**You need ONE Command Center + ONE or MORE Camera Nodes.**

---

## 📋 What You Need

### Required Hardware

| Item | Description | Notes |
|------|-------------|-------|
| **Computer/Raspberry Pi** | Device to run the camera node | Pi 4, Pi 5, or any Linux computer |
| **USB Webcam** | Any USB camera | Most USB webcams work out of the box |
| **Power Supply** | Power for your device | Official Pi power supply recommended |
| **Network Connection** | WiFi or Ethernet | Must be same network as Command Center |

### Required Software

| Item | How to Check | How to Install |
|------|--------------|----------------|
| **Docker** | Run `docker --version` | See [Install Docker](#installing-docker) below |
| **Git** (optional) | Run `git --version` | `sudo apt install git` |

### Installing Docker

**Raspberry Pi / Linux:**
```bash
curl -fsSL https://get.docker.com | sh
sudo usermod -aG docker $USER
```
⚠️ **Log out and back in after running this!**

**Verify it works:**
```bash
docker run hello-world
```

---

## 🚀 Quick Start Guide (10 Minutes)

### Step 1: Download the Project

**Option A - Using Git (recommended):**
```bash
git clone https://github.com/SourceBox-LLC/OpenSentry-Node.git
cd OpenSentry-Node
```

**Option B - Download ZIP:**
1. Go to [GitHub repo](https://github.com/SourceBox-LLC/OpenSentry-Node)
2. Click green "Code" button → "Download ZIP"
3. Unzip and open terminal in that folder

### Step 2: Plug In Your Camera

Connect your USB webcam to the device.

**Check it's detected:**
```bash
ls /dev/video*
```

You should see something like `/dev/video0`. If not, try a different USB port.

### Step 3: Create Configuration File

```bash
cp .env.example .env
```

### Step 4: Edit Configuration

Open the `.env` file:
```bash
nano .env
```

**Minimum changes needed:**
```bash
# Give your camera a friendly name
CAMERA_NAME=Living Room Camera

# Set the security secret (MUST match Command Center!)
OPENSENTRY_SECRET=paste-your-secret-here
```

Save and exit: `Ctrl+X`, then `Y`, then `Enter`

### Step 5: Start the Camera Node

```bash
docker compose up --build
```

**What you should see:**
```
[OpenSentry Node] Starting D-Bus...
[OpenSentry Node] Starting Mosquitto MQTT broker...
[OpenSentry Node] Starting MediaMTX RTSP server...
[OpenSentry Node] Starting Camera Node...
[Node] Camera ID: living-room-camera
[Node] Broadcasting via mDNS...
[Node] Status: streaming
```

### Step 6: Check It's Working

Go to your Command Center dashboard. Within 30 seconds, your new camera should appear!

**That's it!** 🎉

---

## 🍓 Raspberry Pi Setup

OpenSentry Node works great on Raspberry Pi! Here's a dedicated guide.

### Recommended Pi Models

| Model | Works? | Notes |
|-------|--------|-------|
| **Raspberry Pi 5** | ✅ Best | Fastest, recommended for multiple cameras |
| **Raspberry Pi 4** | ✅ Great | 2GB+ RAM recommended |
| **Raspberry Pi 3** | ⚠️ OK | May be slow, 1 camera max |
| **Raspberry Pi Zero** | ❌ No | Too slow for video encoding |

### Step-by-Step Pi Setup

**1. Flash Raspberry Pi OS:**
- Download [Raspberry Pi Imager](https://www.raspberrypi.com/software/)
- Choose "Raspberry Pi OS (64-bit)" - **64-bit is important!**
- Configure WiFi and enable SSH in the imager settings

**2. SSH into your Pi:**
```bash
ssh pi@raspberrypi.local
# Default password: raspberry (change this!)
```

**3. Update the system:**
```bash
sudo apt update && sudo apt upgrade -y
```

**4. Install Docker:**
```bash
curl -fsSL https://get.docker.com | sh
sudo usermod -aG docker pi
```

**5. Reboot:**
```bash
sudo reboot
```

**6. Follow the [Quick Start Guide](#-quick-start-guide-10-minutes) above**

### Camera Tips for Raspberry Pi

**Finding your camera:**
```bash
# List all video devices
ls /dev/video*

# Get detailed info about a camera
v4l2-ctl --list-devices
```

**If you have both a USB camera AND a Pi Camera Module:**
- USB cameras are usually `/dev/video0` or `/dev/video2`
- Pi Camera Module might be `/dev/video0`
- Check with `v4l2-ctl --list-devices` to see which is which

---

## 🔐 Setting Up Security

### The Security Secret

OpenSentry uses a **single secret key** to secure all communication. This secret must be **identical** on:
- ✅ The Command Center
- ✅ Every Camera Node

If they don't match, cameras won't connect!

### Step 1: Generate a Secret

On any computer with Python:
```bash
python -c "import secrets; print(secrets.token_hex(32))"
```

**Example output:** `a1b2c3d4e5f6789012345678901234567890abcdef1234567890abcdef123456`

### Step 2: Add to Camera Node

Edit your `.env` file:
```bash
nano .env
```

Find this line and paste your secret:
```bash
OPENSENTRY_SECRET=a1b2c3d4e5f6789012345678901234567890abcdef1234567890abcdef123456
```

### Step 3: Use Same Secret on Command Center

Copy the **exact same secret** to the Command Center's `.env` file.

### Step 4: Restart Everything

**On the Camera Node:**
```bash
docker compose down
docker compose up --build
```

**On the Command Center:**
```bash
docker compose down
docker compose up --build
```

---

## 🔧 Troubleshooting

### ❌ "No cameras found" or `/dev/video*` shows nothing

**Problem:** Camera not detected by the system.

**Fixes:**
1. **Unplug and replug** the camera
2. **Try a different USB port** (USB 3.0 ports work best)
3. **Check power** - some cameras need a powered USB hub
4. **Add yourself to video group:**
   ```bash
   sudo usermod -aG video $USER
   # Then log out and back in
   ```

---

### ❌ Camera not showing in Command Center

**Problem:** Node is running but Command Center doesn't see it.

**Checklist:**
1. ✅ Is `OPENSENTRY_SECRET` the same on both?
2. ✅ Are both devices on the same network?
3. ✅ Wait 30 seconds - discovery takes time
4. ✅ Check the node logs for errors:
   ```bash
   docker compose logs -f
   ```

**Look for these good signs:**
```
[Node] Broadcasting via mDNS...
[Node] Status: streaming
```

**Look for these problems:**
```
[MQTT] Connection failed    ← Secret might not match
[Camera] Failed to open     ← Camera device issue
```

---

### ❌ "Permission denied" for /dev/video0

**Problem:** Docker can't access the camera.

**Fix:**
```bash
# Add your user to the video group
sudo usermod -aG video $USER

# Log out and back in, then try again
docker compose down
docker compose up --build
```

---

### ❌ Video is laggy or choppy

**Problem:** Not enough processing power or network bandwidth.

**Fixes:**
1. **Use wired Ethernet** instead of WiFi
2. **Lower the resolution** - edit `docker-compose.yml` if needed
3. **Close other applications** on the Pi
4. **Use a Raspberry Pi 4 or 5** if using an older model

---

### ❌ "Cannot connect to Docker daemon"

**Problem:** Docker service isn't running.

**Fix:**
```bash
# Start Docker service
sudo systemctl start docker

# Make it start on boot
sudo systemctl enable docker
```

---

## ⚙️ Advanced Configuration

### All Configuration Options

Edit your `.env` file:

```bash
# ═══════════════════════════════════════════════════════════════
# CAMERA IDENTIFICATION
# ═══════════════════════════════════════════════════════════════

# Unique ID for this camera (auto-generated from hostname if empty)
CAMERA_ID=

# Friendly name shown in the dashboard
CAMERA_NAME=My Camera

# ═══════════════════════════════════════════════════════════════
# CAMERA HARDWARE
# ═══════════════════════════════════════════════════════════════

# Which camera device to use (usually /dev/video0)
CAMERA_DEVICE=/dev/video0

# ═══════════════════════════════════════════════════════════════
# SECURITY (REQUIRED)
# ═══════════════════════════════════════════════════════════════

# Must match the Command Center!
OPENSENTRY_SECRET=your-64-character-secret

# ═══════════════════════════════════════════════════════════════
# NETWORK (usually don't need to change)
# ═══════════════════════════════════════════════════════════════

# RTSP streaming port
RTSP_PORT=8554

# MQTT broker (uses built-in broker by default)
MQTT_SERVER=tcp://localhost:1883
```

### Running Multiple Cameras on One Device

If you have multiple USB cameras on one machine:

**Option 1: Separate directories**
```bash
# Camera 1
mkdir camera1 && cd camera1
# Copy files and configure for /dev/video0

# Camera 2
mkdir camera2 && cd camera2
# Copy files and configure for /dev/video2, different RTSP_PORT
```

**Option 2: Different ports**
```bash
# In .env for camera 1:
CAMERA_ID=front-door
CAMERA_DEVICE=/dev/video0
RTSP_PORT=8554

# In .env for camera 2:
CAMERA_ID=backyard
CAMERA_DEVICE=/dev/video2
RTSP_PORT=8555
```

### Viewing the Raw Stream

You can view the camera stream directly using VLC:

1. Open VLC Media Player
2. Go to **Media → Open Network Stream**
3. Enter: `rtsp://YOUR_PI_IP:8554/stream`
4. If using security, use: `rtsp://opensentry:YOUR_PASSWORD@YOUR_PI_IP:8554/stream`

---

## 📁 Project Structure

```
OpenSentry Node/
│
├── 📄 docker-compose.yml    # Docker configuration
├── 📄 Dockerfile            # Container build instructions
├── 📄 .env.example          # Example configuration (copy to .env)
├── 📄 docker-entrypoint.sh  # Startup script
├── 📄 mediamtx.yml          # Video streaming configuration
├── 📄 mosquitto.conf        # MQTT broker configuration
│
└── 📂 src/                  # Source code (C++)
    └── 📄 main.cpp          # Main application
```

---

## 🆘 Getting Help

### Still stuck?

1. **Check the logs:**
   ```bash
   docker compose logs -f
   ```

2. **Search existing issues:** [GitHub Issues](https://github.com/SourceBox-LLC/OpenSentry-Node/issues)

3. **Open a new issue:** Include:
   - What device you're using (Pi 4, laptop, etc.)
   - What camera you're using
   - The output of `docker compose logs`
   - Your `.env` file (remove the secret first!)

### Related Projects

- **[OpenSentry Command Center](https://github.com/SourceBox-LLC/OpenSentry-Command)** - The dashboard to view all cameras

---

## 📜 License

MIT License - Free for personal and commercial use.

---

<div align="center">

**Made with ❤️ by the OpenSentry Team**

</div>
