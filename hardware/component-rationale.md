# Component Selection Rationale

## Design Philosophy
The component selection prioritizes:
1. **Reliability** - Proven components with good availability
2. **Cost-effectiveness** - Reasonable price for hobbyist/DIY market
3. **Performance** - Adequate specs for high-quality audio
4. **Ease of use** - Components suitable for prototyping and assembly
5. **Eurorack compatibility** - Standard interfaces and power requirements

## Core Processing

### Raspberry Pi Pico (RP2040)
**Selected**: Raspberry Pi Pico
**Alternatives considered**: Arduino Nano 33, STM32F4, Teensy 4.0

**Rationale**:
- **Dual-core architecture** enables clean separation of audio processing (Core 1) and UI/control (Core 0)
- **PIO blocks** provide flexible I2S implementation without blocking CPU
- **Large SRAM** (264KB) sufficient for audio buffers and sample caching
- **Cost-effective** at ~$4, excellent price/performance ratio
- **Strong ecosystem** with mature SDK and community support
- **3.3V native** simplifies interface design with other components

## Audio Processing

### PCM5102A I2S DAC
**Selected**: PCM5102A
**Alternatives considered**: CS4344, WM8731, internal PWM

**Rationale**:
- **24-bit resolution** provides excellent audio quality for drum samples
- **Single 3.3V supply** eliminates need for dual supplies
- **Low THD+N** (-93dB) ensures clean audio output
- **I2S interface** offloads audio timing from CPU
- **Integrated charge pump** simplifies analog design
- **Widely available** on affordable breakout boards
- **Proven in audio projects** with extensive documentation

## Display Interface

### SSD1306 OLED 128×64
**Selected**: SSD1306 128×64 I2C
**Alternatives considered**: LCD 16×2, larger OLEDs, TFT displays

**Rationale**:
- **High contrast** OLED provides excellent readability in all lighting
- **I2C interface** minimizes pin usage (only 2 pins + power)
- **128×64 resolution** sufficient for sample names and basic graphics
- **Low power consumption** important for eurorack power budget
- **Standard size** (0.96") fits well in 8-10HP module width
- **Mature libraries** available for RP2040/Arduino ecosystem

## Storage Solution

### MicroSD Card Interface
**Selected**: SPI MicroSD module
**Alternatives considered**: eMMC, SPI flash, USB storage

**Rationale**:
- **Removable storage** allows easy sample library updates
- **Large capacity** (up to 32GB) holds extensive sample libraries
- **Standard format** compatible with computer file systems
- **SPI interface** well-supported by RP2040 SDK
- **Cost-effective** both module (~$3) and media (~$8 for 32GB)
- **User-friendly** familiar format for musicians

## User Interface

### Rotary Encoder (PEC11R series)
**Selected**: Incremental encoder with integrated push button
**Alternatives considered**: Potentiometer, separate encoder + button

**Rationale**:
- **Infinite rotation** better for menu navigation than potentiometer
- **Integrated switch** saves panel space and simplifies wiring
- **Detented operation** provides tactile feedback for precise selection
- **Standard 6mm shaft** compatible with common knobs
- **Reliable contact** mechanical encoders more robust than optical

### Tactile Switches (B3F series)
**Selected**: 6mm tactile switches
**Alternatives considered**: Larger buttons, capacitive touch, membrane switches

**Rationale**:
- **Immediate response** crucial for drum triggering applications
- **Compact size** allows 4 buttons in limited panel space
- **Reliable actuation** proven design with long life cycle
- **Standard footprint** easy to source and replace
- **Good tactile feedback** important for performance use

## Eurorack Integration

### Power Interface
**Selected**: Standard 16-pin IDC header
**Alternatives considered**: 10-pin legacy, direct wire connection

**Rationale**:
- **Industry standard** ensures compatibility with all modern cases
- **Keyed connector** prevents reverse connection damage
- **Multiple power rails** (+12V, -12V, +5V) provide design flexibility
- **Adequate current** typical eurorack PSU can supply required power
- **Easy installation** standard ribbon cable connection

### Audio/CV Jacks (PJ301M-12)
**Selected**: 3.5mm TS mono jacks
**Alternatives considered**: 1/4" jacks, 3.5mm TRS

**Rationale**:
- **Eurorack standard** 3.5mm TS is universal in modular synthesis
- **Compact size** maximizes jack density in limited panel space
- **Reliable connection** proven design with good contact pressure
- **Cost-effective** widely available from eurorack suppliers
- **Easy mounting** standard panel hole size and threading

## Signal Conditioning

### Op-Amps (TL074)
**Selected**: TL074 quad JFET op-amp
**Alternatives considered**: OPA4134, LM324, discrete transistors

**Rationale**:
- **Low noise** JFET input stage ideal for audio applications
- **Quad package** provides 4 op-amps in single IC, saving space
- **±12V operation** directly compatible with eurorack power rails
- **Low bias current** suitable for high-impedance CV inputs
- **Industry standard** widely used in audio equipment
- **Cost-effective** excellent performance per dollar

## Power Management

### 3.3V Regulator (AMS1117-3.3)
**Selected**: AMS1117-3.3 LDO regulator
**Alternatives considered**: Switching regulators, LM1117

**Rationale**:
- **Low noise** linear regulator better for audio applications than switchers
- **Adequate current** 800mA capacity sufficient for all digital components
- **Simple implementation** requires minimal external components
- **Good regulation** maintains stable 3.3V under varying loads
- **Cost-effective** widely available and inexpensive
- **Standard package** SOT-223 easy to solder and mount

## Mechanical Design

### 8HP Eurorack Format
**Selected**: 8HP width (40.3mm)
**Alternatives considered**: 6HP, 10HP, 12HP

**Rationale**:
- **Compact size** efficient use of rack space
- **Adequate panel area** sufficient for all required controls
- **Standard width** common size in eurorack ecosystem
- **Cost balance** reasonable panel cost vs. functionality
- **Skiff-friendly** shallow depth compatible with portable cases

## Cost Optimization

Total BOM cost of ~$134 represents good value by:
- **Using modules** for complex components (DAC, OLED, SD) reduces development time
- **Standard components** leveraging high-volume parts for cost savings
- **Minimal custom parts** only PCB and front panel require custom manufacturing
- **Proven designs** reducing risk of costly redesigns
- **Hobbyist-friendly** accessible to DIY builders without specialized equipment

## Future Upgrade Paths

The component selection allows for future enhancements:
- **Faster MCU** pin-compatible RP2040 variants or modules
- **Better DAC** higher-end I2S DACs with same interface
- **Larger display** bigger OLEDs with same I2C interface
- **Additional storage** larger SD cards or additional SPI devices
- **Enhanced I/O** additional CV inputs using same conditioning circuits
