# GSPro Controller for WT32-SC01 Plus

A modern touchscreen controller for GSPro Golf Simulator using the WT32-SC01 Plus V3.3 (ESP32-S3 with 3.5" IPS touchscreen). Features a beautiful glassmorphism UI with neon glow effects and smooth animations.

## Features

- 🎨 Ultra-modern glassmorphism UI design
- ✨ Neon glow effects and smooth animations
- 📱 3.5" IPS touchscreen interface (480x320)
- 🎮 Bluetooth HID keyboard control for GSPro
- ⚡ ESP32-S3 powered with high performance
- 🌙 Dark theme with vibrant accent colors

## Hardware Requirements

- **WT32-SC01 Plus V3.3** ESP32-S3 board with integrated 3.5" touchscreen
- USB-C cable for programming and power
- Computer running Windows, macOS, or Linux

## Software Installation

### Step 1: Install Visual Studio Code

Visual Studio Code (VSCode) is the recommended IDE for this project.

#### Windows
1. Download VSCode from [https://code.visualstudio.com/](https://code.visualstudio.com/)
2. Run the installer (`VSCodeUserSetup-{version}.exe`)
3. Follow the installation wizard:
   - ✅ Accept the agreement
   - ✅ Select destination folder
   - ✅ **Check "Add to PATH"** (recommended)
   - ✅ **Check "Create a desktop icon"** (optional)
   - ✅ **Check "Register Code as an editor for supported file types"** (recommended)
4. Click Install and launch VSCode

#### macOS
1. Download VSCode from [https://code.visualstudio.com/](https://code.visualstudio.com/)
2. Open the downloaded `.zip` file
3. Drag `Visual Studio Code.app` to the Applications folder
4. Launch VSCode from Applications or Spotlight

**Add to PATH (optional but recommended):**
- Open VSCode
- Press `Cmd+Shift+P` to open Command Palette
- Type "shell command" and select "Shell Command: Install 'code' command in PATH"

#### Linux (Ubuntu/Debian)
```bash
# Download and install via package manager
sudo apt update
sudo apt install wget gpg
wget -qO- https://packages.microsoft.com/keys/microsoft.asc | gpg --dearmor > packages.microsoft.gpg
sudo install -D -o root -g root -m 644 packages.microsoft.gpg /etc/apt/keyrings/packages.microsoft.gpg
sudo sh -c 'echo "deb [arch=amd64,arm64,armhf signed-by=/etc/apt/keyrings/packages.microsoft.gpg] https://packages.microsoft.com/repos/code stable main" > /etc/apt/sources.list.d/vscode.list'
rm -f packages.microsoft.gpg

sudo apt update
sudo apt install code
```

#### Linux (Fedora/RHEL)
```bash
sudo rpm --import https://packages.microsoft.com/keys/microsoft.asc
sudo sh -c 'echo -e "[code]\nname=Visual Studio Code\nbaseurl=https://packages.microsoft.com/yumrepos/vscode\nenabled=1\ngpgcheck=1\ngpgkey=https://packages.microsoft.com/keys/microsoft.asc" > /etc/yum.repos.d/vscode.repo'

sudo dnf check-update
sudo dnf install code
```

#### Linux (Arch)
```bash
# Via AUR
yay -S visual-studio-code-bin
# or
paru -S visual-studio-code-bin
```

### Step 2: Install PlatformIO IDE Extension

PlatformIO is required to build and upload firmware to the ESP32-S3.

1. **Launch VSCode**
2. **Open Extensions view:**
   - Click the Extensions icon in the sidebar (or press `Ctrl+Shift+X` / `Cmd+Shift+X`)
3. **Search for "PlatformIO IDE"**
4. **Click "Install"** on the extension by PlatformIO
5. **Wait for installation** (this may take a few minutes as it downloads required tools)
6. **Reload VSCode** when prompted

### Step 3: Install USB Drivers (Windows only)

If you're on Windows, you may need to install USB drivers for the ESP32-S3:

1. Download **CP210x USB to UART Bridge Driver** from [Silicon Labs](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
2. Install the driver
3. Restart your computer

**Note:** macOS and Linux typically have these drivers built-in.

## Building and Uploading

### Step 1: Clone or Download the Repository

```bash
git clone https://github.com/yourusername/GSPRO-CONTROLLER.git
cd GSPRO-CONTROLLER
```

### Step 2: Open Project in VSCode

1. Launch VSCode
2. **Open Folder:** `File → Open Folder...` (or press `Ctrl+K Ctrl+O` / `Cmd+K Cmd+O`)
3. Navigate to the `GSPRO-CONTROLLER` folder and click "Select Folder"

### Step 3: Let PlatformIO Initialize

- PlatformIO will automatically detect the `platformio.ini` file
- Wait for dependencies to download (first time only)
- You'll see the PlatformIO toolbar appear at the bottom of VSCode

### Step 4: Build the Firmware

Click the **checkmark (✓)** icon in the PlatformIO toolbar at the bottom of VSCode, or:

```bash
pio run
```

### Step 5: Upload to Device

1. **Connect your WT32-SC01 Plus** to your computer via USB-C
2. **Put the device in bootloader mode** (if needed):
   - Some boards enter bootloader mode automatically
   - If not, hold the BOOT button and press RESET
3. **Click the arrow (→)** icon in the PlatformIO toolbar, or:

```bash
pio run --target upload
```

### Step 6: Monitor Serial Output

Click the **plug icon** in the PlatformIO toolbar to open the Serial Monitor, or:

```bash
pio device monitor
```

## Project Structure

```
GSPRO-CONTROLLER/
├── platformio.ini          # PlatformIO configuration
├── src/
│   ├── main.cpp           # Main application code
│   └── lv_conf.h          # LVGL configuration
└── README.md              # This file
```

## Configuration

The project is pre-configured for the WT32-SC01 Plus V3.3. Key settings in `platformio.ini`:

- **Platform:** ESP32-S3
- **Framework:** Arduino
- **Display:** ST7796 (480x320 IPS)
- **Touch:** FT5x06
- **Flash:** 16MB
- **PSRAM:** Enabled
- **USB CDC:** Enabled for serial debugging

## Dependencies

All dependencies are automatically managed by PlatformIO:

- **LovyanGFX** `^1.1.12` - High-performance display library
- **LVGL** `^8.3.11` - Graphics library for embedded systems
- **ESP32 BLE Keyboard** - Bluetooth HID keyboard functionality

## Troubleshooting

### VSCode/PlatformIO Issues

**Problem:** PlatformIO toolbar not showing
**Solution:** Reload VSCode (`Ctrl+Shift+P` / `Cmd+Shift+P` → "Developer: Reload Window")

**Problem:** "Cannot find Python"
**Solution:** PlatformIO will install Python automatically. Wait for the installation to complete.

**Problem:** Extensions not installing
**Solution:** Check your internet connection and try again. You may need to disable firewall/proxy temporarily.

### Upload Issues

**Problem:** "Failed to connect to ESP32"
**Solution:**
- Ensure the USB cable supports data transfer (not charge-only)
- Try a different USB port
- Hold BOOT button while clicking upload
- Install/reinstall USB drivers (Windows)

**Problem:** "Permission denied" on Linux
**Solution:**
```bash
sudo usermod -a -G dialout $USER
# Then log out and log back in
```

**Problem:** Wrong COM port
**Solution:** Check device manager (Windows) or `ls /dev/tty*` (Linux/macOS) to find the correct port. Update `platformio.ini` if needed:
```ini
upload_port = COM3  ; Windows
upload_port = /dev/ttyUSB0  ; Linux
upload_port = /dev/cu.usbserial-*  ; macOS
```

### Display Issues

**Problem:** Touchscreen not responding
**Solution:** Check that the touch controller (FT5x06) I2C pins are correctly configured in the code

**Problem:** Display colors incorrect
**Solution:** The display uses RGB565 format. Check color definitions in `main.cpp`

## Usage

Once uploaded to your device:

1. Power on the WT32-SC01 Plus
2. The modern UI will appear on the touchscreen
3. Pair the device with your computer via Bluetooth
4. The controller will appear as a Bluetooth keyboard
5. Use the touchscreen interface to control GSPro

## Contributing

Feel free to open issues or submit pull requests for improvements!

## License

This project is open source. Please check the LICENSE file for details.

## Credits

- **LovyanGFX:** Display driver library by lovyan03
- **LVGL:** Graphics library for embedded systems
- **ESP32 BLE Keyboard:** Bluetooth HID library

---

**Enjoy your modern GSPro controller!** ⛳✨
