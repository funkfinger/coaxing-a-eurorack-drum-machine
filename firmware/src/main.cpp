/**
 * Eurorack Drum Machine - Flash Streaming Version
 *
 * Features:
 * - 4-voice polyphonic sample playback with flash streaming
 * - Samples stored in flash filesystem (1MB available)
 * - Small RAM buffers for streaming (2KB per voice)
 * - Much longer samples supported (up to 5+ seconds each)
 * - SD card → Flash → Streaming playback workflow
 * - OLED display with sample status and navigation
 * - Button triggers for manual playback
 * - I2S audio output via PCM5102A
 */

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <I2S.h>
#include <LittleFS.h>
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

// Navigation buttons
#define NAV_UP_PIN 10      // GPIO10 - Navigate up
#define NAV_DOWN_PIN 11    // GPIO11 - Navigate down
#define NAV_SELECT_PIN 12  // GPIO12 - Select sample

// Audio parameters
#define SAMPLE_RATE 48000        // Match your 48kHz samples
#define DEBOUNCE_DELAY 20        // 20ms debounce delay
#define STREAM_BUFFER_SIZE 2048  // 2KB streaming buffer per voice
#define REFILL_THRESHOLD 512     // Refill when buffer has < 512 samples
#define MAX_FLASH_SAMPLE_SIZE \
  524288  // 512KB max per sample (~5.5 seconds at 48kHz)

// Flash-based streaming sample buffer
struct StreamingSample {
  int16_t* buffer;           // Small RAM buffer for streaming
  uint32_t bufferSize;       // Size of RAM buffer (in samples)
  uint32_t bufferHead;       // Current read position in buffer
  uint32_t bufferTail;       // Current write position in buffer
  uint32_t samplesInBuffer;  // Number of samples currently in buffer

  File flashFile;          // Open file handle for streaming
  uint32_t totalSamples;   // Total samples in flash file
  uint32_t samplesPlayed;  // Samples played so far

  bool playing;
  bool loaded;
  bool endOfFile;
  String filename;
  String flashPath;
};

// Sample player structure
struct SamplePlayer {
  StreamingSample stream;
  const char* folderName;
  int currentSampleIndex;
  int totalSamples;
  String sampleList[16];  // Max 16 samples per folder
};

// Initialize sample players for each drum type
SamplePlayer samplePlayers[4] = {
    {{nullptr, 0, 0, 0, 0, File(), 0, 0, false, false, false, "", ""},
     "kick",
     0,
     0,
     {}},
    {{nullptr, 0, 0, 0, 0, File(), 0, 0, false, false, false, "", ""},
     "snare",
     0,
     0,
     {}},
    {{nullptr, 0, 0, 0, 0, File(), 0, 0, false, false, false, "", ""},
     "hihat",
     0,
     0,
     {}},
    {{nullptr, 0, 0, 0, 0, File(), 0, 0, false, false, false, "", ""},
     "tom",
     0,
     0,
     {}}};

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
bool flashWorking = false;
int lastTriggeredSample = 0;
int currentMenuSample = 0;

// Forward declarations
void initializeFlash();
void initializeStreamBuffers();
void initializeSDCard();
void scanSampleFolders();
void loadSampleToFlash(int playerIndex, int sampleIndex);
void triggerSample(int sampleIndex);
void refillStreamBuffer(int playerIndex);
int16_t getNextSample(int playerIndex);
void updateButtons();
void processButtonTriggers();
void updateDisplay();
bool copyWAVToFlash(const String& sdPath, const String& flashPath);

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("=== Eurorack Drum Machine - Flash Streaming ===");
  Serial.printf("Sample Rate: %d Hz\n", SAMPLE_RATE);
  Serial.printf("Stream Buffer Size: %d samples per voice\n",
                STREAM_BUFFER_SIZE / 2);
  Serial.printf("Max Flash Sample Size: %d bytes (~%.1f seconds)\n",
                MAX_FLASH_SAMPLE_SIZE,
                (float)MAX_FLASH_SAMPLE_SIZE / (SAMPLE_RATE * 2));
  Serial.printf("Total RAM for streaming: %d bytes\n", 4 * STREAM_BUFFER_SIZE);
  Serial.println();

  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize button pins
  for (int i = 0; i < 4; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
    Serial.printf("Initialized trigger button %d (%s) on GPIO%d\n", i + 1,
                  buttons[i].name, buttons[i].pin);
  }

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
    display.println("Flash Streaming");
    display.println("Initializing...");
    display.display();
    delay(1000);
  }

  // Initialize Flash filesystem
  initializeFlash();

  // Initialize stream buffers
  initializeStreamBuffers();

  // Initialize SD Card
  initializeSDCard();

  // Initialize I2S
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
  Serial.println("  s: Select sample (copy SD→Flash)");
  Serial.println("  l: List samples");
  Serial.println("Flash streaming ready!");

  if (oledWorking) {
    updateDisplay();
  }
}

void loop() {
  // Process button inputs
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
      case 's':  // Select sample (copy SD to Flash)
        if (samplePlayers[currentMenuSample].totalSamples > 0) {
          int nextIndex =
              (samplePlayers[currentMenuSample].currentSampleIndex + 1) %
              samplePlayers[currentMenuSample].totalSamples;
          loadSampleToFlash(currentMenuSample, nextIndex);
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
    }
  }

  // Generate and output audio samples continuously
  for (int i = 0; i < 32; i++) {
    int32_t mixedSample = 0;

    // Mix all playing samples
    for (int j = 0; j < 4; j++) {
      if (samplePlayers[j].stream.playing && samplePlayers[j].stream.loaded) {
        int16_t sample = getNextSample(j);
        mixedSample += sample;
      }
    }

    // Clamp mixed sample to 16-bit range
    mixedSample = max(-32767, min(32767, mixedSample));

    // Write stereo samples
    i2s.write16((int16_t)mixedSample, (int16_t)mixedSample);
  }

  // Refill stream buffers as needed
  for (int i = 0; i < 4; i++) {
    if (samplePlayers[i].stream.playing &&
        samplePlayers[i].stream.samplesInBuffer < REFILL_THRESHOLD) {
      refillStreamBuffer(i);
    }
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

// Initialize flash filesystem
void initializeFlash() {
  Serial.println("Initializing flash filesystem...");

  if (!LittleFS.begin()) {
    Serial.println("Flash filesystem initialization failed!");
    flashWorking = false;
    return;
  }

  Serial.println("Flash filesystem initialized successfully");
  flashWorking = true;

  // Create sample directories in flash
  const char* dirs[] = {"/kick", "/snare", "/hihat", "/tom"};
  for (int i = 0; i < 4; i++) {
    if (!LittleFS.exists(dirs[i])) {
      LittleFS.mkdir(dirs[i]);
      Serial.printf("Created flash directory: %s\n", dirs[i]);
    }
  }
}

// Initialize stream buffers
void initializeStreamBuffers() {
  Serial.println("Initializing stream buffers...");

  for (int i = 0; i < 4; i++) {
    samplePlayers[i].stream.buffer = (int16_t*)malloc(STREAM_BUFFER_SIZE);
    if (!samplePlayers[i].stream.buffer) {
      Serial.printf("Failed to allocate stream buffer for %s\n",
                    samplePlayers[i].folderName);
      while (1) {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        delay(100);
      }
    }

    samplePlayers[i].stream.bufferSize =
        STREAM_BUFFER_SIZE / 2;  // Convert bytes to samples
    samplePlayers[i].stream.bufferHead = 0;
    samplePlayers[i].stream.bufferTail = 0;
    samplePlayers[i].stream.samplesInBuffer = 0;
    samplePlayers[i].stream.totalSamples = 0;
    samplePlayers[i].stream.samplesPlayed = 0;
    samplePlayers[i].stream.playing = false;
    samplePlayers[i].stream.loaded = false;
    samplePlayers[i].stream.endOfFile = false;

    Serial.printf("Allocated %d sample buffer for %s\n",
                  samplePlayers[i].stream.bufferSize,
                  samplePlayers[i].folderName);
  }
}

// Trigger a sample to start playing
void triggerSample(int sampleIndex) {
  if (sampleIndex < 0 || sampleIndex >= 4) return;

  if (samplePlayers[sampleIndex].stream.loaded) {
    // Reset playback position
    samplePlayers[sampleIndex].stream.samplesPlayed = 0;
    samplePlayers[sampleIndex].stream.bufferHead = 0;
    samplePlayers[sampleIndex].stream.samplesInBuffer = 0;
    samplePlayers[sampleIndex].stream.endOfFile = false;
    samplePlayers[sampleIndex].stream.playing = true;

    // Reopen flash file for streaming
    if (samplePlayers[sampleIndex].stream.flashFile) {
      samplePlayers[sampleIndex].stream.flashFile.close();
    }
    samplePlayers[sampleIndex].stream.flashFile =
        LittleFS.open(samplePlayers[sampleIndex].stream.flashPath, "r");

    if (samplePlayers[sampleIndex].stream.flashFile) {
      // Skip WAV header (44 bytes)
      samplePlayers[sampleIndex].stream.flashFile.seek(44);

      // Fill initial buffer
      refillStreamBuffer(sampleIndex);

      Serial.printf("Playing %s: %s\n", samplePlayers[sampleIndex].folderName,
                    samplePlayers[sampleIndex].stream.filename.c_str());
    } else {
      Serial.printf("Failed to open flash file: %s\n",
                    samplePlayers[sampleIndex].stream.flashPath.c_str());
    }
  } else {
    Serial.printf("No sample loaded for %s\n",
                  samplePlayers[sampleIndex].folderName);
  }
}

// Get next sample from stream buffer
int16_t getNextSample(int playerIndex) {
  StreamingSample& stream = samplePlayers[playerIndex].stream;

  if (!stream.playing || stream.samplesInBuffer == 0) {
    return 0;
  }

  // Get sample from circular buffer
  int16_t sample = stream.buffer[stream.bufferHead];
  stream.bufferHead = (stream.bufferHead + 1) % stream.bufferSize;
  stream.samplesInBuffer--;
  stream.samplesPlayed++;

  // Check if sample is finished
  if (stream.samplesPlayed >= stream.totalSamples) {
    stream.playing = false;
    if (stream.flashFile) {
      stream.flashFile.close();
    }
  }

  return sample;
}

// Refill stream buffer from flash file
void refillStreamBuffer(int playerIndex) {
  StreamingSample& stream = samplePlayers[playerIndex].stream;

  if (!stream.flashFile || stream.endOfFile) return;

  // Fill buffer to capacity
  while (stream.samplesInBuffer < stream.bufferSize && !stream.endOfFile) {
    uint8_t bytes[2];
    size_t bytesRead = stream.flashFile.read(bytes, 2);

    if (bytesRead == 2) {
      // Convert bytes to 16-bit sample (little-endian)
      int16_t sample = (int16_t)(bytes[0] | (bytes[1] << 8));

      // Add to circular buffer
      stream.buffer[stream.bufferTail] = sample;
      stream.bufferTail = (stream.bufferTail + 1) % stream.bufferSize;
      stream.samplesInBuffer++;
    } else {
      stream.endOfFile = true;
    }
  }
}

// Initialize SD Card
void initializeSDCard() {
  Serial.println("Initializing SD card...");

  // Configure SPI pins
  SPI.setRX(SD_MISO_PIN);
  SPI.setTX(SD_MOSI_PIN);
  SPI.setSCK(SD_SCK_PIN);

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
    sdCardWorking = false;
    return;
  }

  Serial.println("SD card initialized successfully");
  sdCardWorking = true;

  // Scan for sample folders
  scanSampleFolders();
}

// Scan SD card for sample folders and files
void scanSampleFolders() {
  if (!sdCardWorking) return;

  Serial.println("Scanning for sample folders...");

  for (int i = 0; i < 4; i++) {
    String folderPath = "/" + String(samplePlayers[i].folderName);
    File folder = SD.open(folderPath);

    if (!folder || !folder.isDirectory()) {
      Serial.printf("Folder %s not found\n", folderPath.c_str());
      samplePlayers[i].totalSamples = 0;
      continue;
    }

    int sampleCount = 0;
    File file = folder.openNextFile();

    while (file && sampleCount < 16) {
      if (!file.isDirectory()) {
        String filename = file.name();

        // Skip hidden files
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

    samplePlayers[i].totalSamples = sampleCount;
    Serial.printf("Folder %s: %d samples found\n", samplePlayers[i].folderName,
                  sampleCount);

    folder.close();
  }
}

// Load sample from SD card to flash storage
void loadSampleToFlash(int playerIndex, int sampleIndex) {
  if (playerIndex < 0 || playerIndex >= 4) return;
  if (sampleIndex < 0 || sampleIndex >= samplePlayers[playerIndex].totalSamples)
    return;

  String filename = samplePlayers[playerIndex].sampleList[sampleIndex];
  String sdPath =
      "/" + String(samplePlayers[playerIndex].folderName) + "/" + filename;
  String flashPath =
      "/" + String(samplePlayers[playerIndex].folderName) + "/" + filename;

  Serial.printf("Loading sample from SD to Flash: %s\n", sdPath.c_str());

  // Close any existing flash file
  if (samplePlayers[playerIndex].stream.flashFile) {
    samplePlayers[playerIndex].stream.flashFile.close();
  }

  // Copy WAV file from SD to flash
  if (copyWAVToFlash(sdPath, flashPath)) {
    samplePlayers[playerIndex].stream.flashPath = flashPath;
    samplePlayers[playerIndex].stream.filename = filename;
    samplePlayers[playerIndex].stream.loaded = true;
    samplePlayers[playerIndex].currentSampleIndex = sampleIndex;

    Serial.printf("Sample loaded to flash: %s\n", filename.c_str());

    // Get sample info from flash file
    File flashFile = LittleFS.open(flashPath, "r");
    if (flashFile) {
      // Read WAV header to get sample count
      flashFile.seek(40);  // Data size is at offset 40
      uint32_t dataSize = 0;
      flashFile.read((uint8_t*)&dataSize, 4);
      samplePlayers[playerIndex].stream.totalSamples =
          dataSize / 2;  // 16-bit samples
      flashFile.close();

      Serial.printf(
          "Flash sample info: %d samples (%.2f seconds)\n",
          samplePlayers[playerIndex].stream.totalSamples,
          (float)samplePlayers[playerIndex].stream.totalSamples / SAMPLE_RATE);
    }
  } else {
    Serial.printf("Failed to load sample: %s\n", filename.c_str());
  }
}

// Copy WAV file from SD to flash with format conversion
bool copyWAVToFlash(const String& sdPath, const String& flashPath) {
  File sdFile = SD.open(sdPath);
  if (!sdFile) {
    Serial.printf("Failed to open SD file: %s\n", sdPath.c_str());
    return false;
  }

  // Read WAV header
  uint8_t header[44];
  if (sdFile.read(header, 44) != 44) {
    Serial.println("Failed to read WAV header");
    sdFile.close();
    return false;
  }

  // Parse WAV header
  uint32_t sampleRate = *(uint32_t*)(header + 24);
  uint16_t bitsPerSample = *(uint16_t*)(header + 34);
  uint16_t numChannels = *(uint16_t*)(header + 22);
  uint32_t dataSize = *(uint32_t*)(header + 40);

  Serial.printf("WAV: %dHz, %d-bit, %d channels, %d bytes\n", sampleRate,
                bitsPerSample, numChannels, dataSize);

  // Check if sample is too large
  if (dataSize > MAX_FLASH_SAMPLE_SIZE) {
    Serial.printf("Sample too large: %d bytes (max %d)\n", dataSize,
                  MAX_FLASH_SAMPLE_SIZE);
    dataSize = MAX_FLASH_SAMPLE_SIZE;
  }

  // Create flash file
  File flashFile = LittleFS.open(flashPath, "w");
  if (!flashFile) {
    Serial.printf("Failed to create flash file: %s\n", flashPath.c_str());
    sdFile.close();
    return false;
  }

  // Write modified WAV header (convert to 16-bit mono)
  header[22] = 1;
  header[23] = 0;  // 1 channel
  header[34] = 16;
  header[35] = 0;  // 16 bits per sample
  uint32_t newDataSize = dataSize / (bitsPerSample / 8) / numChannels *
                         2;  // Convert to 16-bit mono
  *(uint32_t*)(header + 40) = newDataSize;
  *(uint32_t*)(header + 4) = newDataSize + 36;  // File size

  flashFile.write(header, 44);

  // Copy and convert audio data
  uint32_t samplesProcessed = 0;
  uint32_t totalSamples = dataSize / (bitsPerSample / 8) / numChannels;

  while (samplesProcessed < totalSamples && sdFile.available()) {
    int16_t outputSample = 0;

    if (bitsPerSample == 16) {
      if (numChannels == 1) {
        // 16-bit mono - direct copy
        uint8_t bytes[2];
        if (sdFile.read(bytes, 2) == 2) {
          outputSample = (int16_t)(bytes[0] | (bytes[1] << 8));
        }
      } else {
        // 16-bit stereo - mix to mono
        uint8_t bytes[4];
        if (sdFile.read(bytes, 4) == 4) {
          int16_t left = (int16_t)(bytes[0] | (bytes[1] << 8));
          int16_t right = (int16_t)(bytes[2] | (bytes[3] << 8));
          outputSample = (left + right) / 2;
        }
      }
    } else if (bitsPerSample == 24) {
      if (numChannels == 1) {
        // 24-bit mono - convert to 16-bit
        uint8_t bytes[3];
        if (sdFile.read(bytes, 3) == 3) {
          int32_t sample24 =
              (int32_t)(bytes[0] | (bytes[1] << 8) | (bytes[2] << 16));
          if (sample24 & 0x800000) sample24 |= 0xFF000000;  // Sign extend
          outputSample = sample24 >> 8;
        }
      } else {
        // 24-bit stereo - mix to mono and convert to 16-bit
        uint8_t bytes[6];
        if (sdFile.read(bytes, 6) == 6) {
          int32_t left24 =
              (int32_t)(bytes[0] | (bytes[1] << 8) | (bytes[2] << 16));
          int32_t right24 =
              (int32_t)(bytes[3] | (bytes[4] << 8) | (bytes[5] << 16));
          if (left24 & 0x800000) left24 |= 0xFF000000;
          if (right24 & 0x800000) right24 |= 0xFF000000;
          int32_t mixed = (left24 + right24) / 2;
          outputSample = mixed >> 8;
        }
      }
    }

    // Write 16-bit sample to flash
    uint8_t outputBytes[2] = {(uint8_t)(outputSample & 0xFF),
                              (uint8_t)(outputSample >> 8)};
    flashFile.write(outputBytes, 2);

    samplesProcessed++;
  }

  sdFile.close();
  flashFile.close();

  Serial.printf("Copied %d samples to flash: %s\n", samplesProcessed,
                flashPath.c_str());
  return true;
}

// Update button states with debouncing
void updateButtons() {
  unsigned long currentTime = millis();

  // Update trigger buttons
  for (int i = 0; i < 4; i++) {
    bool reading = digitalRead(buttons[i].pin) == LOW;  // Active low

    if (reading != buttons[i].lastState) {
      buttons[i].lastDebounceTime = currentTime;
    }

    if ((currentTime - buttons[i].lastDebounceTime) > DEBOUNCE_DELAY) {
      if (reading != buttons[i].currentState) {
        buttons[i].currentState = reading;

        if (buttons[i].currentState) {
          buttons[i].triggered = true;
        }
      }
    }

    buttons[i].lastState = reading;
  }

  // Update navigation buttons
  for (int i = 0; i < 3; i++) {
    bool reading = digitalRead(navButtons[i].pin) == LOW;  // Active low

    if (reading != navButtons[i].lastState) {
      navButtons[i].lastDebounceTime = currentTime;
    }

    if ((currentTime - navButtons[i].lastDebounceTime) > DEBOUNCE_DELAY) {
      if (reading != navButtons[i].currentState) {
        navButtons[i].currentState = reading;

        if (navButtons[i].currentState) {
          navButtons[i].triggered = true;
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
      Serial.printf("Button %d (%s) triggered!\n", i + 1, buttons[i].name);
      triggerSample(i);
      lastTriggeredSample = i;
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
      loadSampleToFlash(currentMenuSample, nextIndex);
    }
  }
}

// Update OLED display
void updateDisplay() {
  if (!oledWorking) return;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  // Title
  display.println("Flash Streaming");

  // Show current sample info
  if (samplePlayers[currentMenuSample].stream.loaded) {
    display.printf("%s: %s\n", samplePlayers[currentMenuSample].folderName,
                   samplePlayers[currentMenuSample].stream.filename.c_str());

    float duration =
        (float)samplePlayers[currentMenuSample].stream.totalSamples /
        SAMPLE_RATE;
    display.printf("%.1fs", duration);

    if (samplePlayers[currentMenuSample].stream.playing) {
      display.print(" PLAYING");
    }
  } else {
    display.printf("%s: No sample",
                   samplePlayers[currentMenuSample].folderName);
  }

  // Show memory usage
  display.setCursor(0, 24);
  display.printf("Free: %dKB", rp2040.getFreeHeap() / 1024);

  display.display();
}
