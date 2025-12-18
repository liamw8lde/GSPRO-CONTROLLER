# Hardware Specifications

## Standard Shipping Package

This document details the hardware components included in the standard GSPro Controller package.

## Module Specifications

### ESP32-WROVER-B Module

The ESP32-WROVER-B is a powerful Wi-Fi and Bluetooth combo module featuring:

#### Core Processor
- **Chip:** ESP32-D0WD dual-core processor
- **CPU Frequency:** 240 MHz (dual-core Xtensa LX6)
- **Architecture:** 32-bit

#### Memory Configuration
- **Flash Memory:** 32 Mbit (4 MB) onboard SPI flash
- **PSRAM:** 8 MB external PSRAM (64 Mbit)
- **SRAM:** 520 KB internal SRAM
- **ROM:** 448 KB internal ROM

#### Wireless Connectivity
- **Wi-Fi:** 802.11 b/g/n (2.4 GHz)
- **Bluetooth:** Bluetooth v4.2 BR/EDR and BLE
- **Antenna:** Integrated PCB antenna

## Display Module

### 3.5-inch IPS LCD Touchscreen

#### Display Specifications
- **Size:** 3.5 inches (diagonal)
- **Resolution:** 320 x 480 pixels
- **Interface:** SPI (Serial Peripheral Interface)
- **Controller:** ST7796 (or compatible)
- **Colors:** 65K (RGB565 format)
- **Viewing Angles:** Wide viewing angles (IPS technology)

#### Physical Connector
- **Type:** FPC (Flexible Printed Circuit) connector
- **Pitch:** 0.5mm spacing
- **Pins:** 24-pin configuration

#### Touch Panel
- **Technology:** Capacitive touch
- **Controller:** FT5x06 series
- **Interface:** I2C
- **Touch Points:** Multi-touch support

## Power and Interface

### Type-C USB Interface

#### Functionality
- **Primary Use:** Power delivery and programming
- **USB Standard:** USB Type-C
- **Power Input:** 5V DC
- **Data Transfer:** USB to UART bridge (CP210x)
- **Upload Speed:** Up to 921600 baud

#### Features
- Reversible connector design
- Single cable for power and programming
- Serial debugging support
- OTA updates over WiFi (after initial programming)

## Pin Configuration

### ESP32-WROVER-B Key Pins

#### SPI Display Interface
- **MOSI:** GPIO 23
- **MISO:** GPIO 19 (if needed for SD card)
- **SCK:** GPIO 18
- **CS:** GPIO 15 (display chip select)
- **DC:** GPIO 2 (data/command)
- **RST:** GPIO 4 (display reset)
- **BL:** GPIO 27 (backlight control)

#### I2C Touch Interface
- **SDA:** GPIO 21 (touch data)
- **SCL:** GPIO 22 (touch clock)
- **INT:** GPIO 39 (touch interrupt)
- **RST:** GPIO 25 (touch reset)

*Note: Actual pin assignments may vary based on board design. Refer to `src/main.cpp` for exact configuration.*

## Power Specifications

### Power Consumption

#### Active Mode
- **Typical:** ~80-260 mA @ 3.3V (depending on usage)
- **Peak:** ~350 mA (during WiFi/BT transmission)

#### Deep Sleep Mode
- **Ultra-Low Power:** ~10-150 µA
- **Wake Source:** Touch detection, timer wake

### Power Supply
- **Input Voltage:** 5V (via USB Type-C)
- **Operating Voltage:** 3.3V (regulated onboard)
- **Recommended Supply:** Minimum 500mA capable USB port or adapter

## Environmental Specifications

### Operating Conditions
- **Operating Temperature:** -40°C to +85°C
- **Storage Temperature:** -40°C to +125°C
- **Humidity:** 10% to 90% RH (non-condensing)

## Physical Dimensions

### Board Dimensions
- **Width:** ~85-95mm (varies by board design)
- **Height:** ~55-65mm (varies by board design)
- **Thickness:** ~12-15mm (including display)
- **Weight:** ~50-80g (approximate)

## Compliance and Certifications

The ESP32-WROVER-B module includes:
- **FCC:** Federal Communications Commission certified
- **CE:** European Conformity certified
- **IC:** Industry Canada certified
- **TELEC:** Japan Radio Equipment certified
- **RoHS:** Compliant (lead-free)

## Technical Resources

### Datasheets and Documentation
- **ESP32-WROVER-B Datasheet:** [Espressif Official Documentation](https://www.espressif.com/sites/default/files/documentation/esp32-wrover-b_datasheet_en.pdf)
- **ESP32 Technical Reference Manual:** [ESP32 TRM](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf)
- **ST7796 Display Controller:** Check manufacturer documentation
- **FT5x06 Touch Controller:** Check FocalTech documentation

## Development Board Features

### Built-in Components
- **USB-UART Bridge:** Automatic bootloader mode entry
- **Voltage Regulators:** 5V to 3.3V conversion
- **ESD Protection:** On USB data lines
- **Reset and Boot Buttons:** For manual control
- **Status LEDs:** Power and programming indicators

### Expansion Options
- **GPIO Pins:** Available for custom expansion
- **I2C Bus:** Available for additional sensors
- **SPI Bus:** Shared with display (additional CS lines available)
- **UART:** Additional serial ports available

## Compatibility Notes

### Board Variants
This documentation is specifically for the **ESP32-WROVER-B** variant with:
- 4 MB flash memory
- 8 MB PSRAM
- Integrated PCB antenna

Other ESP32 variants (WROOM, WROVER-E, etc.) may have different specifications.

### Display Compatibility
The 3.5-inch display module is designed for this specific board. Using alternative displays may require:
- Pin remapping
- Different display drivers
- Modified touch controller configuration

## Troubleshooting Hardware Issues

### Common Hardware Issues

#### Display Not Working
1. Check FPC connector is fully seated
2. Verify display power connections
3. Check SPI pin configuration in code
4. Test backlight functionality (GPIO 27)

#### Touch Not Responding
1. Verify I2C connections (SDA/SCL)
2. Check touch controller I2C address
3. Test touch interrupt pin (GPIO 39)
4. Verify touch reset sequence

#### Power Issues
1. Use quality USB Type-C cable (data-capable)
2. Ensure adequate power supply (≥500mA)
3. Check for short circuits
4. Monitor voltage levels (should be stable 3.3V)

#### Programming Failures
1. Install CP210x USB drivers (Windows)
2. Try different USB ports
3. Press and hold BOOT button during upload
4. Reduce upload speed if errors persist

## Warranty and Support

For hardware issues or defects, contact your supplier. This is an open-source software project and does not provide hardware warranties.

---

**Last Updated:** December 2025
