# 📷 OpenSentry Camera Node

**Turn any USB webcam into a network security camera.**

---

## 🚀 Quick Start

### 1. Install Docker

```bash
curl -fsSL https://get.docker.com | sh
sudo usermod -aG docker $USER
# Log out and back in
```

### 2. Download & Run

```bash
git clone https://github.com/SourceBox-LLC/OpenSentry-Node.git
cd OpenSentry-Node
chmod +x setup.sh && ./setup.sh
```

The setup will ask for:
- **Camera name** (e.g., "Front Door")
- **Security secret** (copy from Command Center setup)

### 3. Done!

Your camera appears in the [Command Center](https://github.com/SourceBox-LLC/OpenSentry-Command) within 30 seconds.

---

## 🍓 Raspberry Pi

Works great on **Pi 4** and **Pi 5**. Use 64-bit Raspberry Pi OS.

```bash
# SSH into your Pi, then:
curl -fsSL https://get.docker.com | sh
sudo usermod -aG docker pi
sudo reboot

# After reboot:
git clone https://github.com/SourceBox-LLC/OpenSentry-Node.git
cd OpenSentry-Node
chmod +x setup.sh && ./setup.sh
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
