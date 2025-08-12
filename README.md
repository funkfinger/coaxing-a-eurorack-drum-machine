# Eurorack Drum Machine Project

A Raspberry Pi Pico-based eurorack drum machine with 4-voice polyphonic sample playback, CV triggers, and OLED interface.

## 🎯 **Project Status**

### ✅ **Phase 1 Complete: Basic Audio System**

- **I2S Audio Output**: Working with PCM5102A DAC
- **Development Environment**: PlatformIO with Arduino framework
- **Hardware Verification**: Breadboard setup confirmed
- **Audio Quality**: Clean 16-bit output at 16.384kHz

### 🚧 **Current Phase: Multi-voice Audio Engine**

- Sample playback from SD card
- 4-voice polyphonic mixing
- Trigger detection system

## 🔧 **Hardware Setup**

### **Working Configuration**

```
RP2040 Pico Pin    →    PCM5102A Pin
─────────────────────────────────────
Pin 36 (3V3)       →    VCC
Pin 38 (GND)       →    GND
GPIO 26 (Pin 31)   →    BCK (Bit Clock)
GPIO 28 (Pin 34)   →    DIN (Data Input)
GPIO 27 (Pin 32)   →    LCK (Left/Right Clock)
```

### **Required Components**

- Raspberry Pi Pico (RP2040)
- PCM5102A I2S DAC breakout board
- MicroSD card module (future)
- SSD1306 OLED display (future)
- Rotary encoder + buttons (future)

## 🚀 **Quick Start**

### **Prerequisites**

- PlatformIO IDE or CLI
- RP2040 + PCM5102A breadboard setup

### **Build and Flash**

```bash
cd firmware
pio run --target upload
pio device monitor
```

### **Expected Result**

- Clean 1kHz sine wave from PCM5102A outputs
- LED blinking on Pico
- Serial debug output

## 📁 **Project Structure**

```
coaxing-a-eurorack-drum-machine/
├── firmware/                   # Main PlatformIO project
│   ├── src/main.cpp            # Working I2S audio code
│   ├── platformio.ini          # Project configuration
│   └── README.md               # Detailed setup guide
├── hardware/                   # Component specifications
│   ├── component-specifications.md
│   ├── bill-of-materials.csv
│   └── component-rationale.md
├── AI/                         # Development planning
│   └── development-plan.md
├── archive/                    # Old test code
│   ├── breadboard-tests/       # Original Pico SDK tests
│   └── firmware/old-tests/     # PIO programming attempts
└── README.md                   # This file
```

## 🎵 **Technical Specifications**

### **Audio**

- **Sample Rate**: 16,384 Hz
- **Bit Depth**: 16-bit signed
- **Channels**: Stereo (mono samples duplicated)
- **Voices**: 4 simultaneous (planned)
- **Output**: Line level via PCM5102A

### **Control**

- **Triggers**: 4x CV inputs (0-5V, planned)
- **Interface**: OLED + rotary encoder (planned)
- **Storage**: MicroSD card for samples (planned)

### **Power**

- **Supply**: Eurorack +12V/-12V/+5V
- **Consumption**: ~200mA total (estimated)

## 🛠️ **Development Workflow**

### **Current Tools**

- **Platform**: PlatformIO with Arduino framework
- **Core**: earlephilhower Arduino-Pico
- **Audio Library**: Built-in Arduino I2S
- **Debug**: USB serial monitor

### **Key Learnings**

1. **Arduino I2S library** works better than raw PIO programming
2. **16.384kHz sample rate** provides reliable operation
3. **GPIO 26/27/28** confirmed as working I2S pins
4. **PCM5102A jumpers** must be properly configured

## 📋 **Next Development Steps**

1. **Add SD Card Support**

   - SPI interface implementation
   - WAV file parsing
   - Sample loading system

2. **Implement Sample Playback**

   - Replace sine wave with sample playback
   - Multiple sample slots
   - Basic audio mixing

3. **Add Trigger Detection**

   - GPIO interrupt handling
   - CV input conditioning
   - Manual trigger buttons

4. **User Interface**
   - OLED display integration
   - Rotary encoder navigation
   - Sample selection menu

## 🔗 **Resources**

- [PlatformIO Documentation](https://docs.platformio.org/)
- [Arduino-Pico Core](https://github.com/earlephilhower/arduino-pico)
- [RP2040 Datasheet](https://datasheets.raspberrypi.org/rp2040/rp2040-datasheet.pdf)
- [PCM5102A Datasheet](https://www.ti.com/lit/ds/symlink/pcm5102a.pdf)

## 📄 **License**

This project is open source. See individual files for specific licensing terms.

---

**Status**: Phase 1 complete ✅ | Ready for Phase 2 development 🚀
