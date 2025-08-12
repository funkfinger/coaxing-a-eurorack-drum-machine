# PCM5102A Troubleshooting Guide

## ✅ **Confirmed Working:**
- Pico GPIO pins are toggling correctly (3.3V ↔ 0V)
- Breadboard connections are receiving signals
- PlatformIO upload and serial communication working

## 🔍 **PCM5102A Specific Checks**

### 1. **Power Supply Verification**
**Check with multimeter:**
- VCC to GND should read **3.3V** (±0.1V)
- If no voltage: Check 3.3V connection from Pico Pin 36
- If wrong voltage: Check for loose connections

### 2. **PCM5102A Board Configuration**
Most PCM5102A boards have **solder jumpers** that MUST be set correctly:

**Required Settings:**
- **H1**: Must be **SHORTED** (enables 3.3V operation)
- **H2**: Must be **SHORTED** (enables internal PLL)
- **H3**: Must be **SHORTED** (I2S format)

**How to check:**
- Look for tiny solder pads labeled H1, H2, H3 on the PCM5102A board
- They should have solder bridges connecting the pads
- If open, add solder to bridge the connections

### 3. **Pin Connection Verification**
**Double-check these exact connections:**
```
Pico Pin 36 (3V3)    → PCM5102A VCC
Pico Pin 38 (GND)    → PCM5102A GND
Pico GPIO 18 (Pin 24) → PCM5102A BCK
Pico GPIO 19 (Pin 25) → PCM5102A DIN
Pico GPIO 20 (Pin 26) → PCM5102A LCK
```

**Common mistakes:**
- Wrong pin numbers (use GPIO numbers, not physical pin numbers)
- Loose breadboard connections
- Wrong PCM5102A pin labels (some boards vary)

### 4. **PCM5102A Board Variants**
Different suppliers use different pin layouts:

**Type A (Most Common):**
```
VCC  GND  BCK  DIN  LCK  LOUT  ROUT  AGND
```

**Type B:**
```
VIN  GND  BCK  DIN  LCK  LOUT  ROUT  GND
```

**Type C:**
```
VCC  3V3  GND  BCK  DIN  LCK  LOUT  ROUT  AGND
```

Make sure you're connecting to the right pins for YOUR specific board!

### 5. **Audio Output Testing**
**Test different connection methods:**

**Method 1 - Stereo Headphones:**
- Left wire → LOUT
- Right wire → ROUT
- Ground wire → AGND (or GND)

**Method 2 - Mono Test:**
- One headphone wire → LOUT
- Other headphone wire → AGND

**Method 3 - Speaker Test:**
- Small speaker (+) → LOUT
- Small speaker (-) → AGND

### 6. **Signal Verification with Multimeter**
**While GPIO test is running:**
- BCK pin: Should show ~1.65V average (toggling between 0V and 3.3V)
- DIN pin: Should show ~1.65V average
- LCK pin: Should show ~1.65V average

**If pins show 0V or 3.3V steady:** Connection problem
**If pins show ~1.65V:** Signals are reaching PCM5102A ✅

### 7. **PCM5102A Board Quality Issues**
**Some cheap boards have problems:**
- Faulty solder joints
- Wrong component values
- Missing components
- Incorrect PCB layout

**Test with known good board if available**

### 8. **Alternative PCM5102A Boards to Try**
If current board doesn't work:
- Adafruit I2S 3W Class D Amplifier Breakout - MAX98357A
- SparkFun Audio DAC Breakout - CS4344
- Different PCM5102A supplier

## 🔧 **Next Steps**

### Step 1: Check Power
Use multimeter to verify 3.3V between VCC and GND on PCM5102A

### Step 2: Check Jumpers
Verify H1, H2, H3 are shorted with solder bridges

### Step 3: Check Pin Layout
Confirm you're using the right pins for your specific PCM5102A board

### Step 4: Test Audio Output
Try different headphones/speakers and connection methods

### Step 5: Signal Verification
Use multimeter to check signals are reaching PCM5102A pins

## 📸 **What to Check Next**
1. Take a photo of your PCM5102A board (both sides)
2. Check the part number/markings on the board
3. Verify the jumper settings with a magnifying glass
4. Test with different headphones/speakers

## 🎯 **Most Likely Issues**
1. **Jumpers not set** (H1, H2, H3 need solder bridges)
2. **Wrong pin connections** (check your specific board layout)
3. **Power supply issue** (not getting 3.3V)
4. **Faulty PCM5102A board** (try different board)

The GPIO test proves your Pico and code are working correctly - the issue is definitely with the PCM5102A setup!
