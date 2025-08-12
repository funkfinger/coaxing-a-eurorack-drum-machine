# Eurorack Drum Machine Development Plan

## Project Overview

Complete development of a Raspberry Pi Pico-based eurorack drum machine with 4-voice polyphonic sample playback, CV triggers, and OLED interface.

## Hardware Design

### Core Components

- **Raspberry Pi Pico (RP2040)** - Main controller
- **PCM5102A I2S DAC** - Audio output (3.3V compatible)
- **MicroSD card module** - Sample storage
- **SSD1306 OLED display** (128x64, I2C)
- **Rotary encoder with push button** - Navigation
- **4x momentary buttons** - Manual triggers

### Eurorack Interface

- **4x CV input jacks** - Drum triggers (0-5V)
- **1x CV input jack** - Sample/pitch control
- **1x audio output jack** - Main audio out
- **Eurorack power connector** (+12V, -12V, +5V)

### Signal Conditioning

- Voltage dividers for CV inputs (5V → 3.3V)
- Op-amp buffer circuits for clean CV detection
- Audio output buffer (op-amp based)

## Software Architecture

### Core Features

- 4-voice polyphonic sample playback
- Real-time audio mixing using RP2040's dual cores
- CV trigger detection with debouncing
- Sample navigation via rotary encoder
- File system management for SD card samples

### Code Structure

```c
// main.c
// Core 0: UI, CV processing, file management
// Core 1: Audio processing and I2S output
int main() {
    init_hardware();
    multicore_launch_core1(audio_core);
    ui_main_loop(); // Core 0 handles UI/CV
}
```

## Development Phases

### Phase 1: Basic Audio System

- [ ] Set up I2S DAC communication
- [ ] Implement single-voice WAV playback
- [ ] Test audio output quality

### Phase 2: Multi-voice Audio Engine

- [ ] Implement 4-voice mixing
- [ ] Add trigger detection system
- [ ] Basic sample switching

### Phase 3: User Interface

- [ ] OLED display integration
- [ ] Rotary encoder navigation
- [ ] Button handling system

### Phase 4: Eurorack Integration

- [ ] CV input conditioning
- [ ] Calibration routines
- [ ] Final hardware assembly

## PCB Design Considerations

- Separate analog/digital grounds
- Proper power filtering for clean audio
- ESD protection on CV inputs
- Standard eurorack dimensions (HP width)

## ✅ Current Status (Updated)

### Completed:

- **Development Environment**: PlatformIO with Arduino framework ✅
- **Basic Audio System**: I2S output working with PCM5102A ✅
- **Hardware Verification**: Breadboard setup confirmed working ✅

### Working Configuration:

```
RP2040 GPIO 26 → PCM5102A BCK
RP2040 GPIO 28 → PCM5102A DIN
RP2040 GPIO 27 → PCM5102A LCK
```

**Active Development**: `/firmware/` directory contains the working PlatformIO project

## Next Steps

Ready to proceed with multi-voice audio engine and sample playback implementation.
