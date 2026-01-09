# 📷 OpenSentry Camera Node

**Turn any USB webcam into a network security camera.**

**🔒 Fully Encrypted:** RTSPS video streams, MQTT over TLS

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
git clone https://github.com/SourceBox-LLC/OpenSentry-Node.git
cd OpenSentry-Node
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

**Check your [Command Center](https://github.com/SourceBox-LLC/OpenSentry-Command) dashboard - your camera will appear!** 🎉

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
# Camera name shown in dashboard
CAMERA_NAME=Front Door

# Camera device (usually /dev/video0)
CAMERA_DEVICE=/dev/video0

# Security secret (must match Command Center!)
OPENSENTRY_SECRET=your-secret-from-command-center
```

After changes: `docker compose down && docker compose up -d`

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
git clone https://github.com/SourceBox-LLC/OpenSentry-Node.git
cd OpenSentry-Node
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

## 📜 License

MIT - Free for personal and commercial use.

---

**[OpenSentry Command Center](https://github.com/SourceBox-LLC/OpenSentry-Command)** · Made with ❤️ by the OpenSentry Team
