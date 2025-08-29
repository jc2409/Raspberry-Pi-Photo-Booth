# Raspberry Pi Photo Booth

A lightweight C++ photobooth for Raspberry Pi that:
- captures a still image via `rpicam-still`
- composes a **1x1 framed** print with Arm branding using OpenCV
- emails the framed image using a small Python helper

## Requirements
- Raspberry Pi 4B / 5
- Raspberry Pi Camera Module (libcamera/rpicam compatible) enabled
- OpenCV (C++): `libopencv-dev`
- Python 3
- python-dotenv (for .env credential loading)

## Install
Connect the camera module by: 
1. Locate the Camera Module port
2. Gently pull up on the edges of the port’s plastic clip
3. Insert the Camera Module ribbon cable; make sure the connectors at the bottom of the ribbon cable are facing the contacts in the port.
4. Push the plastic clip back into place
5. Reboot your Raspberry Pi

System packages:
```bash
sudo apt-get update
sudo apt-get install -y \
  libopencv-dev \
  python3 python3-venv python3-pip \
  pkg-config
```

Dependencies:
```python
cd scripts
python3 -m venv .venv
source .venv/bin/activate
pip install python-dotenv
```

## Run the Software

Create a `.env` file in the `scripts` folder:
```env
SENDER_EMAIL=youraddress@gmail.com
SENDER_PASSWORD=your_app_password   # e.g., Gmail App Password if 2FA
```

Build the project:
```bash
make clean
make build
```

Run:
```bash
make run
```

Flow:
1. Enter your name.
2. Confirm capture (y) and wait for a 3-second countdown.
3. Outputs (default):
    - Raw photo: ./out/<name>_<epoch>.jpg
    - Framed PNG: ./out/framed/<name>_<epoch>_framed.png
4. Enter an email address to send the framed image (if .env is set)