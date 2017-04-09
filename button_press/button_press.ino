// The Following LINE ADDED - it disables WiFi
SYSTEM_MODE(MANUAL);

const int BUTTON_PIN = D1;

void recheckButton() {
  if (digitalRead(BUTTON_PIN) == 0) {
    // Still down
  } else {
    Serial.println("Button pressed");
  }
}

Timer buttonChecker(300, recheckButton, true);

// Interrupt handler for when the button is changed
void buttonChanged() {
  buttonChecker.startFromISR();
}

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(BUTTON_PIN, buttonChanged, FALLING);
}

void loop() {
  delay(1000);
}
