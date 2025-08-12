/**
 * Eurorack Drum Machine - SD Card Sample Loading
 *
 * Features:
 * - 4-voice polyphonic sample playback from SD card
 * - OLED display with sample status and navigation
 * - Button triggers for manual playback
 * - Sample selection and folder browsing
 * - I2S audio output via PCM5102A
 */

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <I2S.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>

// I2S pin definitions - SAME AS WORKING CODE
#define I2S_BCK_PIN 26   // Bit clock
#define I2S_DATA_PIN 28  // Data output
#define I2S_LCK_PIN 27   // L/R clock (reference)

// OLED configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// I2C pins for OLED
#define SDA_PIN 4  // GPIO4 for I2C SDA
#define SCL_PIN 5  // GPIO5 for I2C SCL

// SD Card SPI pins
#define SD_MISO_PIN 16  // GPIO16 - SPI0 RX
#define SD_MOSI_PIN 19  // GPIO19 - SPI0 TX
#define SD_SCK_PIN 18   // GPIO18 - SPI0 SCK
#define SD_CS_PIN 17    // GPIO17 - Chip Select

// Button/Trigger input pins
#define BUTTON_1_PIN 6  // GPIO6 - Kick
#define BUTTON_2_PIN 7  // GPIO7 - Snare
#define BUTTON_3_PIN 8  // GPIO8 - Hihat
#define BUTTON_4_PIN 9  // GPIO9 - Tom

// Navigation buttons (optional - can use serial for now)
#define NAV_UP_PIN 10      // GPIO10 - Navigate up
#define NAV_DOWN_PIN 11    // GPIO11 - Navigate down
#define NAV_SELECT_PIN 12  // GPIO12 - Select sample

// Audio parameters
#define SAMPLE_RATE 48000      // Match your 48kHz samples (was 16384)
#define DEBOUNCE_DELAY 20      // 20ms debounce delay
#define MAX_SAMPLE_SIZE 32768  // 32KB max per sample (~0.34 seconds at 48kHz)

// Sample buffer structure for SD card samples
struct SampleBuffer {
  int16_t* data;
  uint32_t length;
  uint32_t position;
  bool playing;
  bool loaded;
  String filename;
  String folder;
};

// Sample player structure
struct SamplePlayer {
  SampleBuffer buffer;
  const char* folderName;
  int currentSampleIndex;
  int totalSamples;
  String sampleList[16];  // Max 16 samples per folder
};

// Initialize sample players for each drum type
SamplePlayer samplePlayers[4] = {
    {{nullptr, 0, 0, false, false, "", ""}, "kick", 0, 0, {}},
    {{nullptr, 0, 0, false, false, "", ""}, "snare", 0, 0, {}},
    {{nullptr, 0, 0, false, false, "", ""}, "hihat", 0, 0, {}},
    {{nullptr, 0, 0, false, false, "", ""}, "tom", 0, 0, {}}};

// Button state tracking
struct ButtonState {
  int pin;
  bool lastState;
  bool currentState;
  unsigned long lastDebounceTime;
  bool triggered;
  const char* name;
};

// Initialize button states
ButtonState buttons[4] = {{BUTTON_1_PIN, HIGH, HIGH, 0, false, "Kick"},
                          {BUTTON_2_PIN, HIGH, HIGH, 0, false, "Snare"},
                          {BUTTON_3_PIN, HIGH, HIGH, 0, false, "Hihat"},
                          {BUTTON_4_PIN, HIGH, HIGH, 0, false, "Tom"}};

// Navigation buttons
ButtonState navButtons[3] = {{NAV_UP_PIN, HIGH, HIGH, 0, false, "Up"},
                             {NAV_DOWN_PIN, HIGH, HIGH, 0, false, "Down"},
                             {NAV_SELECT_PIN, HIGH, HIGH, 0, false, "Select"}};

// Create OLED display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// I2S output object
I2S i2s(OUTPUT, I2S_BCK_PIN, I2S_DATA_PIN);

// Control variables
bool oledWorking = false;
bool sdCardWorking = false;
int lastTriggeredSample = 0;
int currentMenuSample = 0;  // For sample selection menu

// Forward declarations
void initializeSDCard();
void scanSampleFolders();
void loadSample(int playerIndex, int sampleIndex);
void updateDisplay();
void updateButtons();
void processButtonTriggers();
void triggerSample(int sampleIndex);
bool loadWAVFile(const String& filepath, SampleBuffer& buffer);

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("=== Eurorack Drum Machine with SD Card ===");
  Serial.printf("Sample Rate: %d Hz\n", SAMPLE_RATE);
  Serial.printf("Max Sample Size: %d bytes\n", MAX_SAMPLE_SIZE);
  Serial.println();

  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize button pins with internal pull-up resistors
  for (int i = 0; i < 4; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
    Serial.printf("Initialized trigger button %d (%s) on GPIO%d\n", i + 1,
                  buttons[i].name, buttons[i].pin);
  }

  // Initialize navigation buttons
  for (int i = 0; i < 3; i++) {
    pinMode(navButtons[i].pin, INPUT_PULLUP);
    Serial.printf("Initialized nav button %s on GPIO%d\n", navButtons[i].name,
                  navButtons[i].pin);
  }

  // Initialize I2C for OLED
  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);
  Wire.begin();

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 allocation failed - continuing without display");
    oledWorking = false;
  } else {
    Serial.println("OLED display initialized");
    oledWorking = true;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Drum Machine");
    display.println("SD Card Init...");
    display.display();
  }

  // Initialize SD Card
  initializeSDCard();

  // Initialize I2S with 16-bit samples
  i2s.setBitsPerSample(16);

  if (!i2s.begin(SAMPLE_RATE)) {
    Serial.println("Failed to initialize I2S!");
    while (1) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      delay(100);
    }
  }

  Serial.println("I2S initialized successfully!");
  Serial.println("Commands:");
  Serial.println("  1-4: Trigger samples");
  Serial.println("  u/d: Navigate samples");
  Serial.println("  s: Select sample");
  Serial.println("  l: List samples");
  Serial.println("Ready!");

  // Update display with initial state
  if (oledWorking) {
    updateDisplay();
  }
}

void loop() {
  // Process button inputs with debouncing
  updateButtons();
  processButtonTriggers();

  // Check for serial input
  if (Serial.available()) {
    char input = Serial.read();

    switch (input) {
      case '1':
        triggerSample(0);
        break;
      case '2':
        triggerSample(1);
        break;
      case '3':
        triggerSample(2);
        break;
      case '4':
        triggerSample(3);
        break;
      case 'u':  // Navigate up
        currentMenuSample = (currentMenuSample - 1 + 4) % 4;
        Serial.printf("Selected: %s\n",
                      samplePlayers[currentMenuSample].folderName);
        break;
      case 'd':  // Navigate down
        currentMenuSample = (currentMenuSample + 1) % 4;
        Serial.printf("Selected: %s\n",
                      samplePlayers[currentMenuSample].folderName);
        break;
      case 's':  // Select next sample in current folder
        if (samplePlayers[currentMenuSample].totalSamples > 0) {
          int nextIndex =
              (samplePlayers[currentMenuSample].currentSampleIndex + 1) %
              samplePlayers[currentMenuSample].totalSamples;
          loadSample(currentMenuSample, nextIndex);
        }
        break;
      case 'l':  // List samples
        for (int i = 0; i < 4; i++) {
          Serial.printf("%s folder: %d samples\n", samplePlayers[i].folderName,
                        samplePlayers[i].totalSamples);
          for (int j = 0; j < samplePlayers[i].totalSamples; j++) {
            Serial.printf("  %d: %s\n", j,
                          samplePlayers[i].sampleList[j].c_str());
          }
        }
        break;
      default:
        break;
    }
  }

  // Generate and output audio samples continuously
  for (int i = 0; i < 32; i++) {
    int32_t mixedSample = 0;

    // Mix all playing samples
    for (int j = 0; j < 4; j++) {
      if (samplePlayers[j].buffer.playing && samplePlayers[j].buffer.loaded &&
          samplePlayers[j].buffer.position < samplePlayers[j].buffer.length) {
        // Read 16-bit sample and add to mix
        int16_t sample =
            samplePlayers[j].buffer.data[samplePlayers[j].buffer.position];
        mixedSample += sample;
        samplePlayers[j].buffer.position++;
      } else if (samplePlayers[j].buffer.playing) {
        // Sample finished playing
        samplePlayers[j].buffer.playing = false;
      }
    }

    // Clamp mixed sample to 16-bit range
    mixedSample = max(-32767, min(32767, mixedSample));

    // Write stereo samples
    i2s.write16((int16_t)mixedSample, (int16_t)mixedSample);
  }

  // Blink LED to show activity
  static unsigned long last_blink = 0;
  if (millis() - last_blink >= 500) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    last_blink = millis();
  }

  // Update display periodically
  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate > 200) {
    if (oledWorking) {
      updateDisplay();
    }
    lastDisplayUpdate = millis();
  }
}

// Initialize SD Card and scan for samples
void initializeSDCard() {
  Serial.println("Initializing SD card...");

  // Configure SPI pins
  SPI.setRX(SD_MISO_PIN);
  SPI.setTX(SD_MOSI_PIN);
  SPI.setSCK(SD_SCK_PIN);

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
    sdCardWorking = false;

    if (oledWorking) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("SD Card Failed");
      display.println("Using built-in");
      display.println("samples");
      display.display();
      delay(2000);
    }
    return;
  }

  Serial.println("SD card initialized successfully");
  sdCardWorking = true;

  // Scan for sample folders and files
  scanSampleFolders();

  // Load first sample from each folder
  for (int i = 0; i < 4; i++) {
    if (samplePlayers[i].totalSamples > 0) {
      loadSample(i, 0);
    }
  }
}

// Scan SD card for sample folders and files
void scanSampleFolders() {
  Serial.println("Scanning for sample folders...");

  for (int i = 0; i < 4; i++) {
    String folderPath = "/" + String(samplePlayers[i].folderName);
    File folder = SD.open(folderPath);

    if (!folder) {
      Serial.printf("Folder %s not found\n", folderPath.c_str());
      samplePlayers[i].totalSamples = 0;
      continue;
    }

    if (!folder.isDirectory()) {
      Serial.printf("%s is not a directory\n", folderPath.c_str());
      folder.close();
      samplePlayers[i].totalSamples = 0;
      continue;
    }

    // Scan files in folder
    int sampleCount = 0;
    File file = folder.openNextFile();

    while (file && sampleCount < 16) {
      if (!file.isDirectory()) {
        String filename = file.name();

        // Skip hidden files (starting with . or ._)
        if (filename.startsWith(".")) {
          Serial.printf("Skipping hidden file: %s\n", filename.c_str());
          file.close();
          file = folder.openNextFile();
          continue;
        }

        filename.toLowerCase();

        // Check if it's a WAV file
        if (filename.endsWith(".wav")) {
          samplePlayers[i].sampleList[sampleCount] = file.name();
          sampleCount++;
          Serial.printf("Found: %s/%s\n", folderPath.c_str(), file.name());
        }
      }
      file.close();
      file = folder.openNextFile();
    }

    folder.close();
    samplePlayers[i].totalSamples = sampleCount;
    Serial.printf("Folder %s: %d samples found\n", samplePlayers[i].folderName,
                  sampleCount);
  }
}

// Load a specific sample from SD card
void loadSample(int playerIndex, int sampleIndex) {
  if (playerIndex < 0 || playerIndex >= 4) return;
  if (sampleIndex < 0 || sampleIndex >= samplePlayers[playerIndex].totalSamples)
    return;

  // Free existing sample data
  if (samplePlayers[playerIndex].buffer.data != nullptr) {
    Serial.printf("Freeing old sample memory (free heap before: %d bytes)\n",
                  rp2040.getFreeHeap());
    free(samplePlayers[playerIndex].buffer.data);
    samplePlayers[playerIndex].buffer.data = nullptr;
    samplePlayers[playerIndex].buffer.loaded = false;
    Serial.printf("Memory freed (free heap after: %d bytes)\n",
                  rp2040.getFreeHeap());
  }

  // Build file path
  String filepath = "/" + String(samplePlayers[playerIndex].folderName) + "/" +
                    samplePlayers[playerIndex].sampleList[sampleIndex];

  Serial.printf("Loading sample: %s\n", filepath.c_str());

  // Load WAV file
  if (loadWAVFile(filepath, samplePlayers[playerIndex].buffer)) {
    samplePlayers[playerIndex].currentSampleIndex = sampleIndex;
    samplePlayers[playerIndex].buffer.filename =
        samplePlayers[playerIndex].sampleList[sampleIndex];
    samplePlayers[playerIndex].buffer.folder =
        samplePlayers[playerIndex].folderName;
    Serial.printf("Loaded: %s (%d samples)\n", filepath.c_str(),
                  samplePlayers[playerIndex].buffer.length);
  } else {
    Serial.printf("Failed to load: %s\n", filepath.c_str());
  }
}

// Simple WAV file loader (assumes 16-bit mono/stereo WAV at any sample rate)
bool loadWAVFile(const String& filepath, SampleBuffer& buffer) {
  File file = SD.open(filepath);
  if (!file) {
    Serial.printf("Cannot open file: %s\n", filepath.c_str());
    return false;
  }

  // Read WAV header (simplified - assumes standard format)
  uint8_t header[44];
  if (file.read(header, 44) != 44) {
    Serial.println("Cannot read WAV header");
    file.close();
    return false;
  }

  // Check for RIFF and WAVE signatures
  if (memcmp(header, "RIFF", 4) != 0 || memcmp(header + 8, "WAVE", 4) != 0) {
    Serial.println("Not a valid WAV file");
    file.close();
    return false;
  }

  // Extract audio format info
  uint16_t audioFormat = *(uint16_t*)(header + 20);
  uint16_t numChannels = *(uint16_t*)(header + 22);
  uint32_t sampleRate = *(uint32_t*)(header + 24);
  uint16_t bitsPerSample = *(uint16_t*)(header + 34);
  uint32_t dataSize = *(uint32_t*)(header + 40);

  Serial.printf("WAV: %dHz, %d-bit, %d channels, %d bytes\n", sampleRate,
                bitsPerSample, numChannels, dataSize);

  // Support both 16-bit and 24-bit samples
  if (bitsPerSample != 16 && bitsPerSample != 24) {
    Serial.printf("Unsupported bit depth: %d-bit (need 16 or 24-bit)\n",
                  bitsPerSample);
    file.close();
    return false;
  }

  // Calculate number of samples
  uint32_t numSamples = dataSize / (bitsPerSample / 8) / numChannels;

  // Limit sample size
  if (numSamples > MAX_SAMPLE_SIZE / 2) {
    numSamples = MAX_SAMPLE_SIZE / 2;
    Serial.printf("Sample truncated to %d samples\n", numSamples);
  }

  // Allocate memory for sample data
  uint32_t bytesNeeded = numSamples * sizeof(int16_t);
  buffer.data = (int16_t*)malloc(bytesNeeded);
  if (!buffer.data) {
    Serial.printf("Cannot allocate %d bytes for sample (free heap: %d bytes)\n",
                  bytesNeeded, rp2040.getFreeHeap());
    file.close();
    return false;
  }

  Serial.printf("Allocated %d bytes for sample (free heap: %d bytes)\n",
                bytesNeeded, rp2040.getFreeHeap());

  // Read sample data
  for (uint32_t i = 0; i < numSamples; i++) {
    if (bitsPerSample == 16) {
      // 16-bit samples
      if (numChannels == 1) {
        // Mono
        int16_t sample;
        if (file.read((uint8_t*)&sample, 2) != 2) break;
        buffer.data[i] = sample;
      } else {
        // Stereo - mix to mono
        int16_t left, right;
        if (file.read((uint8_t*)&left, 2) != 2) break;
        if (file.read((uint8_t*)&right, 2) != 2) break;
        buffer.data[i] = (left + right) / 2;
      }
    } else if (bitsPerSample == 24) {
      // 24-bit samples - convert to 16-bit
      if (numChannels == 1) {
        // Mono 24-bit
        uint8_t bytes[3];
        if (file.read(bytes, 3) != 3) break;
        int32_t sample24 = (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];
        if (sample24 & 0x800000) sample24 |= 0xFF000000;  // Sign extend
        buffer.data[i] = sample24 >> 8;                   // Convert to 16-bit
      } else {
        // Stereo 24-bit - mix to mono
        uint8_t leftBytes[3], rightBytes[3];
        if (file.read(leftBytes, 3) != 3) break;
        if (file.read(rightBytes, 3) != 3) break;

        int32_t left24 =
            (leftBytes[2] << 16) | (leftBytes[1] << 8) | leftBytes[0];
        int32_t right24 =
            (rightBytes[2] << 16) | (rightBytes[1] << 8) | rightBytes[0];

        if (left24 & 0x800000) left24 |= 0xFF000000;    // Sign extend
        if (right24 & 0x800000) right24 |= 0xFF000000;  // Sign extend

        int32_t mixed = (left24 + right24) / 2;
        buffer.data[i] = mixed >> 8;  // Convert to 16-bit
      }
    }
  }

  file.close();

  buffer.length = numSamples;
  buffer.position = 0;
  buffer.playing = false;
  buffer.loaded = true;

  return true;
}

// Button debouncing and trigger detection
void updateButtons() {
  // Process trigger buttons
  for (int i = 0; i < 4; i++) {
    int reading = digitalRead(buttons[i].pin);

    if (reading != buttons[i].lastState) {
      buttons[i].lastDebounceTime = millis();
    }

    if ((millis() - buttons[i].lastDebounceTime) > DEBOUNCE_DELAY) {
      if (reading != buttons[i].currentState) {
        buttons[i].currentState = reading;

        if (buttons[i].currentState == LOW) {
          buttons[i].triggered = true;
          Serial.printf("Button %d (%s) triggered!\n", i + 1, buttons[i].name);
        }
      }
    }

    buttons[i].lastState = reading;
  }

  // Process navigation buttons
  for (int i = 0; i < 3; i++) {
    int reading = digitalRead(navButtons[i].pin);

    if (reading != navButtons[i].lastState) {
      navButtons[i].lastDebounceTime = millis();
    }

    if ((millis() - navButtons[i].lastDebounceTime) > DEBOUNCE_DELAY) {
      if (reading != navButtons[i].currentState) {
        navButtons[i].currentState = reading;

        if (navButtons[i].currentState == LOW) {
          navButtons[i].triggered = true;
          Serial.printf("Nav button %s triggered!\n", navButtons[i].name);
        }
      }
    }

    navButtons[i].lastState = reading;
  }
}

// Process button triggers
void processButtonTriggers() {
  // Process trigger buttons
  for (int i = 0; i < 4; i++) {
    if (buttons[i].triggered) {
      buttons[i].triggered = false;
      triggerSample(i);
    }
  }

  // Process navigation buttons
  if (navButtons[0].triggered) {  // Up
    navButtons[0].triggered = false;
    currentMenuSample = (currentMenuSample - 1 + 4) % 4;
    Serial.printf("Selected: %s\n",
                  samplePlayers[currentMenuSample].folderName);
  }

  if (navButtons[1].triggered) {  // Down
    navButtons[1].triggered = false;
    currentMenuSample = (currentMenuSample + 1) % 4;
    Serial.printf("Selected: %s\n",
                  samplePlayers[currentMenuSample].folderName);
  }

  if (navButtons[2].triggered) {  // Select
    navButtons[2].triggered = false;
    if (samplePlayers[currentMenuSample].totalSamples > 0) {
      int nextIndex =
          (samplePlayers[currentMenuSample].currentSampleIndex + 1) %
          samplePlayers[currentMenuSample].totalSamples;
      loadSample(currentMenuSample, nextIndex);
    }
  }
}

// Trigger a specific sample
void triggerSample(int sampleIndex) {
  if (sampleIndex >= 0 && sampleIndex < 4) {
    if (samplePlayers[sampleIndex].buffer.loaded) {
      samplePlayers[sampleIndex].buffer.position = 0;
      samplePlayers[sampleIndex].buffer.playing = true;
      lastTriggeredSample = sampleIndex;

      Serial.printf("Playing %s: %s\n", samplePlayers[sampleIndex].folderName,
                    samplePlayers[sampleIndex].buffer.filename.c_str());
    } else {
      Serial.printf("No sample loaded for %s\n",
                    samplePlayers[sampleIndex].folderName);
    }
  }
}

// Display functions
void updateDisplay() {
  if (!oledWorking) return;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  if (!sdCardWorking) {
    display.println("SD Card Error");
    display.println("Check wiring");
    display.display();
    return;
  }

  // Title
  display.println("Drum Machine");

  // Show currently selected sample type
  display.printf("Sel: %s\n", samplePlayers[currentMenuSample].folderName);

  // Show playing samples or current sample info
  bool anyPlaying = false;
  for (int i = 0; i < 4; i++) {
    if (samplePlayers[i].buffer.playing) {
      anyPlaying = true;
      break;
    }
  }

  if (anyPlaying) {
    display.print("Playing: ");
    for (int i = 0; i < 4; i++) {
      if (samplePlayers[i].buffer.playing) {
        display.printf("%c", samplePlayers[i].folderName[0]);  // First letter
      }
    }
  } else {
    // Show current sample for selected type
    if (samplePlayers[currentMenuSample].buffer.loaded) {
      String shortName = samplePlayers[currentMenuSample].buffer.filename;
      if (shortName.length() > 10) {
        shortName = shortName.substring(0, 10) + "...";
      }
      display.printf("Cur: %s", shortName.c_str());
    } else {
      display.print("No samples");
    }
  }

  display.display();
}
