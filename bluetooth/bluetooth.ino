#include <DuoBLE.h>

// Disable WiFi
SYSTEM_MODE(MANUAL);

// Name Device
const char * const deviceName = "BUSTA";
const int BUTTON_PIN = D1;

// Create Service
BLEService lightService("d010a8e8-281f-4c78-9b31-9db227353f3d");

// Create Characteristics 
BLECharacteristic targetColorCharacteristic("FF01", ATT_PROPERTY_READ | ATT_PROPERTY_WRITE);
BLECharacteristic currentColorCharacteristic("FF02", ATT_PROPERTY_READ | ATT_PROPERTY_WRITE);
BLECharacteristic defaultColorCharacteristic("FF03", ATT_PROPERTY_READ | ATT_PROPERTY_WRITE);
BLECharacteristic changeInCharacteristic("FF04", ATT_PROPERTY_READ | ATT_PROPERTY_WRITE);
BLECharacteristic changeAtCharacteristic("FF05", ATT_PROPERTY_READ | ATT_PROPERTY_WRITE);

/* Global Variables */

int fadeTimeInMillis = 5000;
int numIntervalsPerSec = 50;
int updateIntervalInMillis = fadeTimeInMillis/numIntervalsPerSec;

int targetColor[3] = {255, 255, 255};
int currentColor[3] = {255, 255, 255};
int defaultOnColor[3] = {255, 255, 255};
int defaultOffColor[3] = {0, 0, 0};
float colorDeltas[3] = {0, 0, 0};

/* Helper Functions */

boolean arraysEqual(int first[], int second[]) {
  boolean equal = true;
  for (int n=0;n<3;n++) {
    if (first[n]!=second[n]) {
      equal = false;
    }
  }
  return equal;
}

void setArray(int array[], int a, int b, int c) {
  array[0] = a;
  array[1] = b;
  array[2] = c;
}

void updateColor() {
  if (!arraysEqual(targetColor, currentColor)) {
    Serial.print("Target r: ");
    Serial.print(targetColor[0]);
    Serial.println();
    Serial.print("Current r: ");
    Serial.print(currentColor[0]);
    Serial.println();
    Serial.println();
    if (abs(colorDeltas[0]) > abs(targetColor[0]-currentColor[0]) ||
        abs(colorDeltas[1]) > abs(targetColor[1]-currentColor[1]) ||
        abs(colorDeltas[2]) > abs(targetColor[2]-currentColor[2])) {
      currentColor[0] = targetColor[0];
      currentColor[1] = targetColor[1];
      currentColor[2] = targetColor[2];
    } else {
      currentColor[0] += colorDeltas[0];
      currentColor[1] += colorDeltas[1];
      currentColor[2] += colorDeltas[2];
    }
    RGB.color(currentColor[0], currentColor[1], currentColor[2]);
  }
}

void crossFade(int r, int g, int b) {
  setArray(targetColor, r, g, b);
  colorDeltas[0] = (targetColor[0] - currentColor[0])/numIntervalsPerSec;
  colorDeltas[1] = (targetColor[1] - currentColor[1])/numIntervalsPerSec;
  colorDeltas[2] = (targetColor[2] - currentColor[2])/numIntervalsPerSec;
}

Timer updateColors(updateIntervalInMillis, updateColor);

bool lightOn() {
  return currentColor[0] > 0 || currentColor[1] > 0 || currentColor[2] > 0;
}

void recheckButton() {
  if (digitalRead(BUTTON_PIN) == 0) {
    // Still down
  } else {
    Serial.println("Button pressed");
    updateColors.stop();
    if (lightOn()) {
      crossFade(defaultOffColor[0], defaultOffColor[1], defaultOffColor[2]);
    } else {
      crossFade(defaultOnColor[0], defaultOnColor[1], defaultOnColor[2]);
    }
    updateColors.start();
  }
}

Timer buttonChecker(300, recheckButton, true);

// Interrupt handler for when the button is changed
void buttonChanged() {
  buttonChecker.startFromISR();
}

/* Characteristic Callbacks */

/**
 * Used to set the target color for crossfade or read the current target
 */
void targetColorCallback(BLERecipient recipient, BLECharacteristicCallbackReason reason) {
  Serial.print("Target Color Characteristic; Reason: ");
  Serial.println(reason);
  if (reason == POSTWRITE) {
    byte value[20];
    int bytes = targetColorCharacteristic.getValue(value, 20);
    if (bytes >= 3) {
      updateColors.stop();
      crossFade(value[0], value[1], value[2]);
      updateColors.start();
    } else {
      byte newValues[3] = {targetColor[0], targetColor[1], targetColor[2]};
      targetColorCharacteristic.setValue(newValues, 3); 
    }
  } else if (reason == PREREAD) {
    byte newValues[3] = {targetColor[0], targetColor[1], targetColor[2]};
    targetColorCharacteristic.setValue(newValues, 3); 
  }
}

/**
 * Used to set the current color for immediate color change or read the current color of the light
 */
void currentColorCallback(BLERecipient recipient, BLECharacteristicCallbackReason reason) {
  Serial.print("Current Color Characteristic; Reason: ");
  Serial.println(reason);
  if (reason == POSTWRITE) {
    byte value[20];
    int bytes = currentColorCharacteristic.getValue(value, 20);
    if (bytes >= 3) {
      updateColors.stop();
      setArray(targetColor, value[0], value[1], value[2]);
      setArray(currentColor, value[0], value[1], value[2]);
      RGB.color(currentColor[0], currentColor[1], currentColor[2]);
      updateColors.start();
    } else {
      byte newValues[3] = {currentColor[0], currentColor[1], currentColor[2]};
      currentColorCharacteristic.setValue(newValues, 3); 
    }
  } else if (reason == PREREAD) {
    byte newValues[3] = {currentColor[0], currentColor[1], currentColor[2]};
    currentColorCharacteristic.setValue(newValues, 3);  
  }
}

/**
 * Used to read and set the default on color
 */
void defaultColorCallback(BLERecipient recipient, BLECharacteristicCallbackReason reason) {
  Serial.print("Default Color Characteristic; Reason: ");
  Serial.println(reason);
  if (reason == POSTWRITE) {
    byte value[20];
    int bytes = defaultColorCharacteristic.getValue(value, 20);
    if (bytes >= 3) {
      defaultOnColor[0] = value[0];
      defaultOnColor[1] = value[1];
      defaultOnColor[2] = value[2];
    } else {
      byte newValues[3] = {defaultOnColor[0], defaultOnColor[1], defaultOnColor[2]};
      defaultColorCharacteristic.setValue(newValues, 3); 
    }
  } else if (reason == PREREAD) {
    byte newValues[3] = {defaultOnColor[0], defaultOnColor[1], defaultOnColor[2]};
    defaultColorCharacteristic.setValue(newValues, 3);  
  }
}

int inTimer[2] = {-50, 0}; // First is time in millis, second is 0 | 1 for off | on

/**
 * Used to read and set the "in" timer time and value
 */
void changeInCallback(BLERecipient recipient, BLECharacteristicCallbackReason reason) {
  Serial.print("Change In Characteristic; Reason: ");
  Serial.println(reason);
  if (reason == POSTWRITE) {
    byte value[20];
    int bytes = changeInCharacteristic.getValue(value, 20);
    if (bytes >= 2) {
      inTimer[0] = value[0];
      inTimer[1] = value[1];
    } else {
      byte newValues[2] = {inTimer[0], inTimer[1]};
      changeInCharacteristic.setValue(newValues, 2); 
    }
  } else if (reason == PREREAD) {
    byte newValues[2] = {inTimer[0], inTimer[1]};
    changeInCharacteristic.setValue(newValues, 2); 
  }
}

int atTimer[4] = {0, 0, 0, 0}; // Hour, minute, second, on/off

/**
 * Used to read and set the "at" timer time and value
 */
void changeAtCallback(BLERecipient recipient, BLECharacteristicCallbackReason reason) {
  Serial.print("Change At Characteristic; Reason: ");
  Serial.println(reason);
  if (reason == POSTWRITE) {
    byte value[20];
    int bytes = changeAtCharacteristic.getValue(value, 20);
    if (bytes >= 4) {
      atTimer[0] = value[0];
      atTimer[1] = value[1];
      atTimer[2] = value[2];
      atTimer[3] = value[3];
    } else {
      byte newValues[4] = {atTimer[0], atTimer[1], atTimer[2], atTimer[3]};
      changeInCharacteristic.setValue(newValues, 4); 
    }
  } else if (reason == PREREAD) {
    byte newValues[4] = {atTimer[0], atTimer[1], atTimer[2], atTimer[3]};
    changeInCharacteristic.setValue(newValues, 4); 
  }
}

void setup() {
 Serial.begin(9600);
  
 // Set the data in the custom characteristics:
 byte initTargetColorValue[3] = {255, 255, 255};  // A 3 byte array of colors
 targetColorCharacteristic.setValue(initTargetColorValue, 3);
 targetColorCharacteristic.setCallback(targetColorCallback);
 lightService.addCharacteristic(targetColorCharacteristic);
 
 byte initCurrentColorValue[3] = {255, 255, 255};  // A 3 byte array of colors
 currentColorCharacteristic.setValue(initCurrentColorValue, 3);
 currentColorCharacteristic.setCallback(currentColorCallback);
 lightService.addCharacteristic(currentColorCharacteristic);
 
 byte initDefaultColorValue[3] = {255, 255, 255};  // A 3 byte array of colors
 defaultColorCharacteristic.setValue(initDefaultColorValue, 3);
 defaultColorCharacteristic.setCallback(defaultColorCallback);
 lightService.addCharacteristic(defaultColorCharacteristic);
 
 byte initChangeInValue[2] = {-50, 0};  // Negative time, off change
 changeInCharacteristic.setValue(initChangeInValue, 2);
 changeInCharacteristic.setCallback(changeInCallback);
 lightService.addCharacteristic(changeInCharacteristic);
 
 byte initChangeAtValue[4] = {0, 0, 0, 0};  // Zero times, off value
 changeAtCharacteristic.setValue(initChangeAtValue, 4);
 changeAtCharacteristic.setCallback(changeAtCallback);
 lightService.addCharacteristic(changeAtCharacteristic);
 
 // Add the Service
 DuoBLE.addService(lightService);
 
 DuoBLE.begin();
 DuoBLE.advertisingDataAddName(ADVERTISEMENT, deviceName);
 DuoBLE.setName(deviceName);
 // Start advertising.
 DuoBLE.startAdvertising();
 Serial.println("BLE start advertising.");
 
 RGB.control(true);
 RGB.color(defaultOnColor[0], defaultOnColor[1], defaultOnColor[2]);
 
 pinMode(BUTTON_PIN, INPUT_PULLUP);
 attachInterrupt(BUTTON_PIN, buttonChanged, FALLING);
  
 updateColors.start();
}

void loop() {
  delay(1000);
}

void BLE_connected() {
 Serial.println("Central Connected");
}
void BLE_disconnected() {
 Serial.println("Central Disconnected");
}