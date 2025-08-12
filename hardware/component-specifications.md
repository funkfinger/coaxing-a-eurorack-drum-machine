# Component Specifications - Eurorack Drum Machine

## Core Processing Unit

### Raspberry Pi Pico (RP2040)
- **Part Number**: Raspberry Pi Pico
- **MCU**: RP2040 dual-core ARM Cortex-M0+ @ 133MHz
- **Memory**: 264KB SRAM, 2MB QSPI Flash
- **GPIO**: 26 × multi-function GPIO pins
- **I2S Support**: Via PIO (Programmable I/O) blocks
- **Power**: 3.3V operation, ~100mA typical
- **Form Factor**: 21mm × 51mm
- **Price**: ~$4 USD

## Audio Output

### PCM5102A I2S DAC Module
- **Part Number**: PCM5102A (Texas Instruments)
- **Resolution**: 24-bit, up to 384kHz sample rate
- **SNR**: 112dB (typical)
- **THD+N**: -93dB @ 1kHz
- **Supply**: 3.3V single supply compatible
- **Interface**: I2S (BCLK, LRCLK, DATA)
- **Output**: Differential stereo outputs
- **Module**: Pre-built breakout boards available (~$8-12)
- **Required External**: Low-pass filter, DC blocking caps

## Display

### SSD1306 OLED Display
- **Part Number**: SSD1306 128×64 OLED Module
- **Resolution**: 128×64 pixels, monochrome
- **Interface**: I2C (default) or SPI
- **Supply**: 3.3V-5V compatible
- **Size**: 0.96" diagonal
- **Current**: ~20mA active, <1µA sleep
- **Module**: Common breakout boards (~$5-8)

## Storage

### MicroSD Card Module
- **Interface**: SPI
- **Supply**: 3.3V
- **Card Support**: microSD/microSDHC up to 32GB
- **Module**: Standard SPI breakout boards (~$2-5)
- **Speed**: Class 10 recommended for audio streaming

## User Interface

### Rotary Encoder
- **Type**: Incremental rotary encoder with push button
- **Detents**: 20-24 per revolution
- **Switch**: Momentary push button (SPST)
- **Mounting**: Panel mount, 6mm shaft
- **Example**: Bourns PEC11R series (~$3-5)

### Trigger Buttons
- **Type**: Momentary tactile switches
- **Quantity**: 4 units
- **Actuation**: 160gf typical
- **Travel**: 0.25mm
- **Mounting**: Panel mount or PCB mount
- **Example**: Omron B3F series (~$1-2 each)

## Eurorack Interface

### Power Connector
- **Type**: 16-pin shrouded IDC header (2×8)
- **Keying**: Polarized ribbon cable connector
- **Rails**: +12V, -12V, +5V (optional), GND
- **Current Requirements**:
  - +12V: ~150mA (analog circuits)
  - -12V: ~50mA (op-amps)
  - +5V: ~200mA (digital circuits)
- **Part**: Standard Eurorack power header

### Audio Output Jack
- **Type**: 3.5mm TS (mono) jack
- **Mounting**: Panel mount
- **Impedance**: High impedance output (~1kΩ)
- **Level**: ±5V (10Vpp) eurorack standard
- **Part**: Thonkiconn or equivalent (~$1-2)

### CV Input Jacks
- **Type**: 3.5mm TS (mono) jack
- **Quantity**: 5 units (4 triggers + 1 control)
- **Input Range**: 0-5V (eurorack gate/trigger standard)
- **Mounting**: Panel mount
- **Part**: Thonkiconn or equivalent (~$1-2 each)

## Signal Conditioning

### Op-Amps
- **Part**: TL074 or TL084 (quad op-amp)
- **Supply**: ±12V (from eurorack power)
- **Purpose**: CV input buffering, audio output buffering
- **Package**: DIP-14 or SOIC-14
- **Quantity**: 2 units
- **Price**: ~$1-2 each

### Voltage Dividers (CV Input)
- **R1**: 10kΩ (5V to 3.3V divider upper)
- **R2**: 20kΩ (5V to 3.3V divider lower)
- **Tolerance**: 1% metal film
- **Power**: 1/4W minimum
- **Protection**: 3.3V Zener diodes for overvoltage

### Audio Output Buffer
- **Coupling**: 10µF electrolytic capacitors
- **Gain Setting**: 10kΩ/3.3kΩ resistors (3x gain)
- **Filtering**: 100nF ceramic + 10µF electrolytic

## Power Supply Components

### Voltage Regulators
- **3.3V Regulator**: AMS1117-3.3 or LM1117-3.3
- **Input**: +5V from eurorack (if available) or +12V
- **Output Current**: 800mA minimum
- **Package**: SOT-223 or TO-220
- **Decoupling**: 100µF input, 47µF output

### Power Filtering
- **Bulk Capacitors**: 100µF electrolytic per rail
- **Bypass Capacitors**: 100nF ceramic per IC
- **Ferrite Beads**: For digital supply isolation
- **TVS Diodes**: ESD protection on CV inputs

## Mechanical Components

### Enclosure
- **Standard**: Eurorack module format
- **Width**: 8HP (40.3mm) or 10HP (50.5mm)
- **Depth**: <40mm (skiff-friendly)
- **Material**: Aluminum front panel
- **Mounting**: M3 screws, standard eurorack rails

### Hardware
- **Panel Screws**: M3 × 6mm (4 units)
- **Standoffs**: M3 × 10mm (for PCB mounting)
- **Washers**: M3 flat washers
- **Nuts**: M3 hex nuts

## Total Estimated Cost
- Core components: ~$35-45
- Interface components: ~$15-25
- PCB and mechanical: ~$20-30
- **Total**: ~$70-100 USD (excluding labor/assembly)

## Supplier Recommendations
- **Electronics**: Mouser, Digikey, LCSC
- **Modules**: Adafruit, SparkFun, Amazon
- **Eurorack specific**: Thonk, Synthcube, Modular Addict
- **PCB**: JLCPCB, PCBWay, OSH Park
