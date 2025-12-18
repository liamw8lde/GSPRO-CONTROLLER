# GSPro Controller for WT32-SC01 Plus

A modern touchscreen controller for GSPro Golf Simulator using the WT32-SC01 Plus V3.3 (ESP32-S3 with 3.5" IPS touchscreen). Features a beautiful glassmorphism UI with neon glow effects, smooth animations, wireless connectivity, OTA updates, and intelligent power management.

## Features

### Core Functionality
- 🎮 **Bluetooth HID Keyboard** - Wireless control for GSPro Golf Simulator
- 📱 **3.5" IPS Touchscreen** - Responsive touch interface (480x320)
- ⚡ **ESP32-S3 Powered** - High performance with PSRAM support

### User Interface
- 🎨 **Ultra-modern Glassmorphism UI** - Beautiful translucent card design
- ✨ **Neon Glow Effects** - Smooth animations and visual feedback
- 🌙 **Dark Theme** - Eye-friendly with vibrant accent colors
- ⚙️ **Settings Screen** - Comprehensive configuration interface

### Connectivity & Updates
- 📡 **WiFi Support** - Network scanning and connection management
- 🔄 **OTA Updates** - Over-The-Air firmware updates via WiFi
- 🔵 **Bluetooth Management** - Easy pairing and reconnection

### Power Management
- 💤 **Deep Sleep Mode** - Ultra-low power consumption when idle
- 👆 **Touch Wake-Up** - Automatic touch detection to wake from sleep
- 🔋 **Energy Efficient** - Smart timer-based wake checks

### GSPro Controls
- 🔄 **Mulligan** (Ctrl+M) - Take another shot
- 📍 **Pin Indicator** (P) - Show pin position
- 🔭 **Scout View** (J) - Preview green layout
- 🗺️ **Heat Map** (Y) - Shot dispersion view
- 🕊️ **Free Flight** (F5) - Free camera mode
- 🦅 **Flyover** (O) - Course flyover view
- ⛳ **Tee Box Controls** (C/V) - Move tee left/right
- 🎯 **Aim Controls** (Arrow Keys) - Precise shot aiming

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

### OTA Configuration

For security, you should change the default OTA password in `src/main.cpp`:

```cpp
// Line 1070 in main.cpp
ArduinoOTA.setPassword("gspro2024");  // Change this password!
```

**To change the password:**
1. Open `src/main.cpp`
2. Find line 1070: `ArduinoOTA.setPassword("gspro2024");`
3. Replace `"gspro2024"` with your own secure password
4. Upload the new firmware

**OTA Settings:**
- **Hostname:** `GSProController`
- **Default Password:** `gspro2024` (⚠️ CHANGE THIS!)
- **Port:** 3232 (default Arduino OTA port)

## Dependencies

All dependencies are automatically managed by PlatformIO:

- **LovyanGFX** `^1.1.12` - High-performance display library
- **LVGL** `^8.3.11` - Graphics library for embedded systems
- **ESP32 BLE Keyboard** - Bluetooth HID keyboard functionality
- **WiFi** (built-in) - WiFi connectivity for ESP32
- **ArduinoOTA** (built-in) - Over-The-Air firmware updates
- **ESP Sleep** (built-in) - Deep sleep power management

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

### WiFi & OTA Issues

**Problem:** Can't find WiFi networks
**Solution:**
- Ensure your WiFi router is powered on and broadcasting
- Check that you're within range of the network
- Try scanning multiple times
- The ESP32 only supports 2.4GHz WiFi (not 5GHz)

**Problem:** OTA upload not working
**Solution:**
- Ensure the device is connected to WiFi first
- Verify you're on the same network as the device
- Check the OTA password matches what's in the code
- In PlatformIO, look for network ports (not just USB ports)
- Firewall may be blocking port 3232 - add exception if needed

**Problem:** OTA update fails mid-transfer
**Solution:**
- Ensure stable WiFi connection
- Move device closer to router
- Try uploading again (ArduinoOTA is resumable)

### Bluetooth Issues

**Problem:** Can't pair with computer
**Solution:**
- Remove old "GSPro Controller" pairings from your computer
- Restart Bluetooth on your computer
- Use the "RESTART BLE" button in Settings screen
- Power cycle the device

**Problem:** Bluetooth disconnects frequently
**Solution:**
- Ensure you're within range (typically 10 meters)
- Remove obstacles between device and computer
- Check for other Bluetooth devices causing interference

### Deep Sleep Issues

**Problem:** Device won't wake from deep sleep
**Solution:**
- Touch the screen firmly for 1-2 seconds
- If stuck, press the physical reset button on the device
- The device checks for touch every 1 second

**Problem:** Battery drains in deep sleep
**Solution:**
- This is expected behavior as the touch controller stays active
- For longer storage, power off the device completely

## Usage

### Initial Setup

1. **Power on** the WT32-SC01 Plus
2. The modern UI will appear with a boot animation
3. **Pair via Bluetooth:**
   - On your computer, search for Bluetooth devices
   - Look for "GSPro Controller"
   - Connect to pair with your device
4. When connected, the status indicator will turn green and show "LINKED"

### Main Controls

The main screen provides quick access to all GSPro functions:

**Left Panel - Views:**
- PIN - Toggle pin indicator
- SCOUT - Preview green layout
- HEAT MAP - View shot dispersion
- FLYOVER - Course flyover mode

**Center Panel - Aim Controls:**
- Arrow buttons for precise shot aiming
- Large, easy-to-hit targets for quick adjustments

**Right Panel - Actions:**
- MULLIGAN - Take another shot
- FREE FLT - Free flight camera mode
- TEE BOX - Move tee position left/right (C/V keys)

### Settings Screen

Access the settings screen by tapping the **SETTINGS** button in the header.

**WiFi Panel:**
- **SCAN** - Search for available WiFi networks
- **DISCONNECT** - Disconnect from current network
- View network list with signal strength and security status
- Connect to WiFi to enable OTA updates

**Bluetooth Panel:**
- View connection status
- **RESTART BLE** - Restart Bluetooth if connection issues occur
- Device name: "GSPro Controller"

**OTA Update Panel:**
- Monitor firmware update progress
- Automatic updates when connected to WiFi
- Progress bar shows update status
- **To update firmware:**
  1. Connect to WiFi using the WiFi panel
  2. Use PlatformIO or Arduino IDE with network port:
     - Hostname: `GSProController`
     - Password: `gspro2024` (change in code for security)
  3. Upload new firmware over WiFi

**Power Panel:**
- **DEEP SLEEP** - Enter low-power sleep mode
- Touch screen to wake up (automatic detection)
- Saves battery when not in use
- Display turns off completely during sleep

### Status Bar

The bottom status bar shows:
- Current action/command feedback
- Keyboard shortcut reference
- Real-time status updates

### Connection Indicator

Top-right corner shows Bluetooth status:
- **Yellow (pulsing)** - Waiting for connection
- **Green (solid)** - Connected and ready

## Contributing

Feel free to open issues or submit pull requests for improvements!

## License

This project is open source. Please check the LICENSE file for details.

## Credits

- **LovyanGFX:** Display driver library by lovyan03
- **LVGL:** Graphics library for embedded systems
- **ESP32 BLE Keyboard:** Bluetooth HID library
- **ArduinoOTA:** Over-The-Air update framework by Arduino
- **ESP32 WiFi & Sleep:** Espressif ESP32 framework libraries

## Technical Details

**Built with:**
- PlatformIO IDE
- Arduino Framework for ESP32
- ESP32-S3 with dual-core processor @ 240MHz
- 16MB Flash memory
- PSRAM enabled for smooth graphics

**Key Technologies:**
- Bluetooth Low Energy (BLE) HID
- WiFi 802.11 b/g/n (2.4GHz)
- ArduinoOTA for wireless updates
- LVGL graphics with hardware acceleration
- Deep sleep with timer-based wake

---

**Enjoy your modern GSPro controller!** ⛳✨
