// The Following LINE ADDED - it disables WiFi
SYSTEM_MODE(MANUAL);

void print() {
  Serial.println("Printed");
}

Timer onTimer(0, print, true);
Timer offTimer(0, print, true);

void setup() {
  Serial.begin(9600);
}

void loop() {
  onTimer = Timer(1000, print, true);
  onTimer.start();
  delay(3000);
}
