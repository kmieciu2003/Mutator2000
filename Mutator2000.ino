/**
 * Mutator2000 - Bluetooth Microphone Mute Button
 * For Seeed Xiao BLE nRF52840 Sense
 * 
 * LED States:
 * - Red solid: Error
 * - Green pulsing: Pairing mode (no bonded devices)
 * - Cyan pulsing: Waiting for connection with bonded device
 * - Cyan solid: Connected
 * - Green 3x flash: Confirmation of device name reset
 */

#include <bluefruit.h>

// Pin configuration
#define BUTTON_PIN 1
// LED_RED, LED_GREEN, LED_BLUE are already defined in variant.h

// Keyboard shortcut for mute (Win+Alt+K - Windows default mic mute)
// For Linux: set custom shortcut in Settings -> Keyboard
#define MUTE_KEY HID_KEY_K
#define MUTE_MODIFIERS (KEYBOARD_MODIFIER_LEFTGUI | KEYBOARD_MODIFIER_LEFTALT)

// Bluetooth objects
BLEDis bledis;
BLEHidAdafruit blehid;
BLEBas blebas;  // Battery Service

// Button state
bool lastButtonState = HIGH;
bool buttonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// LED pulsing
unsigned long lastPulseUpdate = 0;
float pulsePhase = 0;
const float pulseSpeed = 0.05; // Pulse speed

// Connection state and device name
bool hasBondedDevices = false;
bool forceNewPairing = false; // Flag to reject old bondings
char deviceName[32] = "Mutator2000"; // Base name or Mutator2000_hostname

// Connection timeout - 10 minutes
const unsigned long CONNECTION_TIMEOUT = 10 * 60 * 1000; // 10 minutes in milliseconds
unsigned long advertisingStartTime = 0;
bool timeoutEnabled = false;

// Battery monitoring
#define VBAT_PIN PIN_VBAT  // Built-in battery voltage divider pin
#define VBAT_MV_PER_LSB (0.73242188F)  // 3.0V ADC range and 12-bit ADC (3000mV/4096)
#define VBAT_DIVIDER (0.71275837F)     // Voltage divider 2M + 0.806M
#define VBAT_DIVIDER_COMP (1.403F)     // Compensation factor

// Battery update interval (1 minute)
const unsigned long BATTERY_UPDATE_INTERVAL = 60 * 1000;
unsigned long lastBatteryUpdate = 0;

// Function declarations
void setupBluetooth();
void startAdvertising();
void connect_callback(uint16_t conn_handle);
void disconnect_callback(uint16_t conn_handle, uint8_t reason);
void toggleMute();
void updateLED();
void setLED(uint8_t r, uint8_t g, uint8_t b);
float readBatteryVoltage();
uint8_t readBatteryPercent();

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) delay(10);
  
  Serial.println("=== Mutator2000 Start ===");
  
  // POWER SAVING - disable unnecessary peripherals
  // Disable NFC (if it was enabled)
  NRF_NFCT->TASKS_DISABLE = 1;
  
  // Setup battery monitoring
  analogReference(AR_INTERNAL_3_0);  // 3.0V reference for battery monitoring
  analogReadResolution(12);          // 12-bit ADC
  
  // Read and display battery level
  float voltage = readBatteryVoltage();
  uint8_t percent = readBatteryPercent();
  Serial.print("Battery: ");
  Serial.print(voltage);
  Serial.print("V (");
  Serial.print(percent);
  Serial.println("%)");
  
  // Pin configuration
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setLED(0, 0, 0); // All off
  
  // Check if button is pressed during startup (name reset)
  delay(100);
  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("========================================");
    Serial.println("BUTTON PRESSED - NAME RESET!");
    Serial.println("========================================");
    
    // Green 3x flash - confirmation of name reset
    for (int i = 0; i < 3; i++) {
      setLED(0, 255, 0); // Green
      delay(200);
      setLED(0, 0, 0);
      delay(200);
    }
    
    // Set flag forcing new pairing
    forceNewPairing = true;
    hasBondedDevices = false;
    
    Serial.println("FORCE NEW PAIRING mode active");
    Serial.println("Old bondings will be REJECTED");
    Serial.println("Windows will show error - then remove device and pair again");
    Serial.println("========================================");
    
    // Wait until user releases button
    while (digitalRead(BUTTON_PIN) == LOW) {
      delay(10);
    }
    delay(100);
  } else {
    Serial.println("Normal start (no button)");
  }
  
  // Normal startup
  Serial.println("Initializing Bluetooth...");
  setupBluetooth();
  
  Serial.println("=== Mutator2000 Ready! ===");
}

void loop() {
  // Button handling with debouncing
  int reading = digitalRead(BUTTON_PIN);
  
  // If state changed, reset debounce timer
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  lastButtonState = reading;
  
  // If debounce time passed, update button state
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If state changed
    if (reading != buttonState) {
      buttonState = reading;
      
      // Button pressed (change HIGH -> LOW)
      if (buttonState == LOW) {
        if (Bluefruit.connected()) {
          toggleMute();
          Serial.println("Mute toggled!");
        }
      }
    }
  }
  
  // Update LED based on state
  updateLED();
  
  // Update battery level periodically (every 1 minute)
  if (millis() - lastBatteryUpdate > BATTERY_UPDATE_INTERVAL) {
    lastBatteryUpdate = millis();
    uint8_t batteryPercent = readBatteryPercent();
    blebas.write(batteryPercent);
    
    Serial.print("Battery update: ");
    Serial.print(readBatteryVoltage());
    Serial.print("V (");
    Serial.print(batteryPercent);
    Serial.println("%)");
  }
  
  // Check connection timeout (only when waiting for bonded device)
  if (timeoutEnabled && !Bluefruit.connected()) {
    if (millis() - advertisingStartTime > CONNECTION_TIMEOUT) {
      Serial.println("Connection timeout - entering deep sleep");
      Serial.println("Press RESET button to wake up");
      
      // Flash red 3x to indicate going to sleep
      for (int i = 0; i < 3; i++) {
        setLED(255, 0, 0); // Red
        delay(200);
        setLED(0, 0, 0);
        delay(200);
      }
      
      // Enter deep sleep (wake only by reset button)
      sd_power_system_off();
    }
  }
  
  delay(10);
}

void setupBluetooth() {
  Serial.println("Initializing Bluetooth...");
  
  Bluefruit.begin();
  Bluefruit.setTxPower(4);  // Max power +4dBm for good range
  Bluefruit.setName(deviceName);
  
  // Power saving mode for connection
  Bluefruit.Periph.setConnInterval(9, 16); // 11.25ms - 20ms (fast response, low latency)
  
  Serial.print("Device name: ");
  Serial.println(deviceName);
  
  bledis.setManufacturer("DIY");
  bledis.setModel("Mutator2000");
  bledis.begin();
  
  // Initialize Battery Service
  blebas.begin();
  blebas.write(readBatteryPercent());  // Initial battery level
  Serial.print("Battery service started at: ");
  Serial.print(readBatteryPercent());
  Serial.println("%");
  
  blehid.begin();
  
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);
  
  startAdvertising();
}

void startAdvertising() {
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);
  Bluefruit.Advertising.addService(blehid);
  Bluefruit.Advertising.addName();
  
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);
  Bluefruit.Advertising.setFastTimeout(30);
  
  if (hasBondedDevices) {
    Serial.println("Have saved device - waiting for connection...");
  } else {
    Serial.println("Pairing mode - no saved devices");
  }
  
  Bluefruit.Advertising.start(0);
  
  // Start timeout timer if we have bonded devices (waiting for connection)
  if (hasBondedDevices) {
    advertisingStartTime = millis();
    timeoutEnabled = true;
    Serial.println("Connection timeout started - 10 minutes");
  }
}

void connect_callback(uint16_t conn_handle) {
  Serial.println("Connection attempt...");
  Serial.print("forceNewPairing = ");
  Serial.println(forceNewPairing ? "TRUE" : "FALSE");
  
  // If forcing new pairing AND this connection uses old keys
  // Problem: we can't distinguish old from new pairing at this point
  // Solution: Disable flag after first ANY connection
  
  if (forceNewPairing) {
    Serial.println("Force pairing flag was active - disabling after first connection");
    forceNewPairing = false;
  }
  
  Serial.println("Connected!");
  
  // Disable timeout - we're connected
  timeoutEnabled = false;
  
  BLEConnection* connection = Bluefruit.Connection(conn_handle);
  if (connection) {
    char central_name[32] = {0};
    connection->getPeerName(central_name, sizeof(central_name));
    Serial.print("Connected to: ");
    Serial.println(central_name);
  }
  
  // Update status
  hasBondedDevices = true;
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  Serial.println("Disconnected!");
}

void toggleMute() {
  // Send key press with longer delays for Windows compatibility
  uint8_t keycodes[6] = {MUTE_KEY, 0, 0, 0, 0, 0};
  blehid.keyboardReport(MUTE_MODIFIERS, keycodes);
  delay(50);  // Increased from 10ms - Windows needs more time
  blehid.keyRelease();
  delay(50);  // Increased from 10ms
}

void updateLED() {
  if (Bluefruit.connected()) {
    // Connected - cyan solid
    setLED(0, 255, 255); // Cyan
  } else {
    // Disconnected - pulsing
    pulsePhase += pulseSpeed;
    if (pulsePhase > TWO_PI) pulsePhase -= TWO_PI;
    
    // Pulse value (0-255)
    uint8_t brightness = (uint8_t)((sin(pulsePhase) + 1.0) * 127.5);
    
    if (hasBondedDevices) {
      // Have saved device - cyan pulsing
      setLED(0, brightness, brightness); // Cyan pulsing
    } else {
      // Pairing mode - green pulsing
      setLED(0, brightness, 0); // Green pulsing
    }
  }
}

void setLED(uint8_t r, uint8_t g, uint8_t b) {
  // LEDs are active LOW - invert values
  analogWrite(LED_RED, 255 - r);
  analogWrite(LED_GREEN, 255 - g);
  analogWrite(LED_BLUE, 255 - b);
}

// Read battery voltage in volts
float readBatteryVoltage() {
  float raw = analogRead(VBAT_PIN);
  return raw * VBAT_MV_PER_LSB * VBAT_DIVIDER_COMP / 1000.0;
}

// Read battery percentage (0-100%)
// Li-Po: 4.2V = 100%, 3.7V = 50%, 3.3V = 0%
uint8_t readBatteryPercent() {
  float voltage = readBatteryVoltage();
  
  // Li-Po discharge curve approximation
  if (voltage >= 4.2) return 100;
  if (voltage <= 3.3) return 0;
  
  // Linear approximation between 3.3V and 4.2V
  return (uint8_t)((voltage - 3.3) * 111.11);
}
