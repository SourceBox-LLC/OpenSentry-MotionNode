# 📷 OpenSentry Camera Node

**Turn any USB webcam into a network security camera.**

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

Paste the `OPENSENTRY_SECRET` from your Command Center setup:

```
🔐 Enter the OPENSENTRY_SECRET from your Command Center.
   (Run 'cat .env' on the Command Center to find it)

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

## 📜 License

MIT - Free for personal and commercial use.

---

**[OpenSentry Command Center](https://github.com/SourceBox-LLC/OpenSentry-Command)** · Made with ❤️ by the OpenSentry Team
