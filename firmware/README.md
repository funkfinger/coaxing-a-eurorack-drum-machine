# Eurorack Drum Machine - PlatformIO Version

This is the main PlatformIO project for the RP2040-based eurorack drum machine with I2S audio output using the PCM5102A DAC.

## Features

- **Arduino Framework**: Easy to use Arduino functions and libraries
- **PlatformIO**: Modern development environment with dependency management
- **PIO-based I2S**: Uses RP2040's PIO blocks for precise I2S timing
- **Dual-core**: Core 0 handles main logic, Core 1 generates LRCLK
- **Serial Debug**: Real-time status output via USB serial

## Hardware Connections

```
RP2040 Pico Pin    →    PCM5102A Pin
─────────────────────────────────────
Pin 36 (3V3)       →    VCC
Pin 38 (GND)       →    GND
GPIO 18 (Pin 24)   →    BCK (Bit Clock)
GPIO 19 (Pin 25)   →    DIN (Data Input)
GPIO 20 (Pin 26)   →    LCK (Left/Right Clock)
```

### Audio Output

```
PCM5102A Pin       →    Audio Connection
─────────────────────────────────────
LOUT               →    Left channel
ROUT               →    Right channel
AGND               →    Audio ground
```

## Prerequisites

1. **PlatformIO**: Install PlatformIO IDE or CLI
2. **Hardware**: RP2040 Pico + PCM5102A breakout board
3. **Breadboard**: For prototyping connections

## Installation

### Option 1: PlatformIO IDE (Recommended)

1. Install [PlatformIO IDE](https://platformio.org/platformio-ide)
2. Open this folder as a PlatformIO project
3. Build and upload

### Option 2: PlatformIO CLI

```bash
# Install PlatformIO CLI
pip install platformio

# Navigate to project directory
cd platformio

# Build the project
pio run

# Upload to Pico (hold BOOTSEL while connecting USB)
pio run --target upload

# Monitor serial output
pio device monitor
```

## Project Structure

```
platformio/
├── platformio.ini      # Project configuration
├── src/
│   ├── main.cpp        # Main Arduino code
│   └── i2s.pio         # PIO assembly (optional)
└── README.md           # This file
```

## Configuration

### platformio.ini Settings

- **Platform**: `raspberrypi`
- **Board**: `pico`
- **Framework**: `arduino`
- **Core**: `earlephilhower` (Arduino-Pico)

### Audio Parameters

```cpp
#define SAMPLE_RATE 44100
#define BUFFER_SIZE 512
#define TONE_FREQUENCY 440.0f  // A4 note
```

## Usage

1. **Wire the hardware** according to the connection diagram
2. **Build and upload** the project
3. **Connect headphones** to PCM5102A outputs
4. **Open serial monitor** to see debug output
5. **Listen for 440Hz tone** - should be a clean sine wave

## Serial Output

Expected output:

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

## Troubleshooting

### No Audio Output

- Check all wiring connections
- Verify PCM5102A has 3.3V power
- Ensure PCM5102A jumpers are set correctly (H1, H2, H3 shorted)
- Try different headphones

### Build Errors

- Update PlatformIO to latest version
- Check that `raspberrypi` platform is installed
- Verify Arduino-Pico core is available

### Upload Issues

- Hold BOOTSEL button while connecting USB
- Pico should appear as USB drive
- Copy `.pio/build/pico/firmware.uf2` manually if needed

## Customization

### Change Audio Frequency

```cpp
#define TONE_FREQUENCY 880.0f  // A5 note
```

### Different Waveforms

Replace `generate_test_tone()` with:

- Sine wave generation
- White noise
- Sample playback from SD card

### Different Sample Rates

```cpp
#define SAMPLE_RATE 48000  // 48kHz
```

## Next Steps

This basic I2S test can be extended to:

- **Multi-voice mixing**: Play multiple samples simultaneously
- **SD card samples**: Load drum samples from SD card
- **CV triggers**: Add trigger inputs for drum machine functionality
- **OLED display**: Add user interface
- **Eurorack integration**: Add proper CV conditioning

## Resources

- [PlatformIO Documentation](https://docs.platformio.org/)
- [Arduino-Pico Core](https://github.com/earlephilhower/arduino-pico)
- [RP2040 Datasheet](https://datasheets.raspberrypi.org/rp2040/rp2040-datasheet.pdf)
- [PCM5102A Datasheet](https://www.ti.com/lit/ds/symlink/pcm5102a.pdf)
