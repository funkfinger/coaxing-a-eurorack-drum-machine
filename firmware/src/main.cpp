/**
 * Arduino I2S Test - Same approach as working code
 * 
 * Uses Arduino I2S library instead of raw PIO programming
 * This should work exactly like the working Mozzi code
 */

#include <Arduino.h>
#include <I2S.h>
#include <math.h>

// I2S pin definitions - SAME AS WORKING CODE
#define I2S_BCK_PIN 26   // Bit clock
#define I2S_DATA_PIN 28  // Data output
#define I2S_LCK_PIN 27   // L/R clock (not used directly, but for reference)

// Audio parameters
#define SAMPLE_RATE 16384  // Same as MOZZI_AUDIO_RATE
#define TONE_FREQUENCY 1000.0f

// I2S output object - SAME AS WORKING CODE
I2S i2s(OUTPUT, I2S_BCK_PIN, I2S_DATA_PIN);

// Audio generation variables
float phase = 0.0f;
float phase_increment;

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("=== Arduino I2S Test (Same as Working Code) ===");
  Serial.printf("Sample Rate: %d Hz\n", SAMPLE_RATE);
  Serial.printf("Tone Frequency: %.0f Hz\n", TONE_FREQUENCY);
  Serial.printf("  GPIO %d -> BCK (Bit Clock)\n", I2S_BCK_PIN);
  Serial.printf("  GPIO %d -> DIN (Data)\n", I2S_DATA_PIN);
  Serial.printf("  GPIO %d -> LCK (L/R Clock)\n", I2S_LCK_PIN);
  Serial.println();
  
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Calculate phase increment for sine wave
  phase_increment = 2.0f * M_PI * TONE_FREQUENCY / SAMPLE_RATE;
  
  // Initialize I2S with 16-bit samples - SAME AS WORKING CODE
  i2s.setBitsPerSample(16);
  
  // Start I2S at the configured sample rate - SAME AS WORKING CODE
  if (!i2s.begin(SAMPLE_RATE)) {
    Serial.println("Failed to initialize I2S!");
    while (1) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      delay(100);
    }
  }
  
  Serial.println("Arduino I2S initialized successfully!");
  Serial.println("You should hear a 1kHz sine wave");
}

void loop() {
  // Generate and output audio samples continuously
  // This mimics what Mozzi does in the working code
  
  for (int i = 0; i < 64; i++) {  // Process samples in small batches
    // Generate sine wave sample
    float sine_value = sin(phase);
    int16_t sample = (int16_t)(sine_value * 16383);  // 50% volume
    
    // Write stereo samples (same sample for both channels) - SAME AS WORKING CODE
    i2s.write16(sample, sample);
    
    // Update phase
    phase += phase_increment;
    if (phase >= 2.0f * M_PI) {
      phase -= 2.0f * M_PI;
    }
  }
  
  // Blink LED to show activity
  static unsigned long last_blink = 0;
  if (millis() - last_blink >= 500) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    last_blink = millis();
    
    // Status update
    static int blink_count = 0;
    if (++blink_count >= 10) {
      Serial.println("Audio playing - Arduino I2S approach");
      blink_count = 0;
    }
  }
}
