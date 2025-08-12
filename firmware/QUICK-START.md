# Quick Start Guide - PlatformIO I2S Test

## ✅ **Ready to Flash!**

Your PlatformIO project is built and ready to test. Here's how to get it running:

### 📁 **Generated Files**
- `firmware.uf2` - Ready to flash to Pico (174KB)
- `firmware.elf` - Debug version
- `firmware.bin` - Binary version

### 🔌 **Hardware Setup**

Wire your breadboard exactly like this:

```
RP2040 Pico Pin    →    PCM5102A Pin
─────────────────────────────────────
Pin 36 (3V3)       →    VCC
Pin 38 (GND)       →    GND  
GPIO 18 (Pin 24)   →    BCK (Bit Clock)
GPIO 19 (Pin 25)   →    DIN (Data Input)
GPIO 20 (Pin 26)   →    LCK (Left/Right Clock)
```

**Audio Output:**
```
PCM5102A Pin       →    Headphones
─────────────────────────────────────
LOUT               →    Left channel
ROUT               →    Right channel
AGND               →    Ground
```

### 🚀 **Flash and Test**

1. **Program the Pico:**
   ```bash
   # Hold BOOTSEL button while connecting USB
   # Copy firmware.uf2 to the Pico drive
   cp .pio/build/pico/firmware.uf2 /Volumes/RPI-RP2/
   ```

2. **Monitor Serial Output:**
   ```bash
   pio device monitor
   # or
   screen /dev/cu.usbmodem* 115200
   ```

3. **Expected Output:**
   ```
   Arduino I2S Test Starting...
   Connections:
     GPIO 18 -> BCK (Bit Clock)
     GPIO 19 -> DIN (Data)
     GPIO 20 -> LCK (L/R Clock)

   Generated test tone
   I2S PIO initialized
   DMA initialized
   Started core1 for LRCLK generation
   Audio output started - you should hear a square wave tone
   Setup complete!
   Still running... LED blinking, audio should be playing
   ```

4. **Listen for Audio:**
   - Connect headphones to PCM5102A outputs
   - You should hear a 440Hz square wave tone
   - LED on Pico should blink every 500ms

### 🔧 **Development Commands**

```bash
# Build project
pio run

# Clean build
pio run --target clean

# Upload (alternative method)
pio run --target upload

# Monitor serial
pio device monitor

# List connected devices
pio device list
```

### 🐛 **Troubleshooting**

**No Audio:**
- Check all wiring connections
- Verify PCM5102A has 3.3V power
- Check PCM5102A jumper settings (H1, H2, H3 should be shorted)

**No Serial Output:**
- Make sure USB cable is connected
- Try different terminal: `screen /dev/cu.usbmodem* 115200`

**Build Errors:**
- Update PlatformIO: `pio upgrade`
- Clean build: `pio run --target clean`

### 🎛️ **Customization**

**Change Frequency:**
Edit `src/main.cpp`:
```cpp
#define TONE_FREQUENCY 880.0f  // A5 note instead of A4
```

**Change Sample Rate:**
```cpp
#define SAMPLE_RATE 48000  // 48kHz instead of 44.1kHz
```

**Different Waveform:**
Replace the `generate_test_tone()` function with sine wave or other patterns.

### 📊 **Memory Usage**
- **RAM**: 4.4% (11,416 bytes used)
- **Flash**: 7.1% (74,508 bytes used)
- Plenty of room for expansion!

### 🎯 **Next Steps**

Once basic audio is working:
1. Add SD card sample loading
2. Implement multiple voice mixing
3. Add CV trigger inputs
4. Add OLED display
5. Build full drum machine functionality

This PlatformIO setup gives you a much better development experience than the raw Pico SDK!
