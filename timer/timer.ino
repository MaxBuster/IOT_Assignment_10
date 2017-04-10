// The Following LINE ADDED - it disables WiFi
SYSTEM_MODE(MANUAL);

void setup() {
  Serial.begin(9600);
  Time.setTime(1491772044);
}

void loop() {
  if (Time.second() % 3 == 0) {
    Serial.println(Time.second());
    Serial.println(Time.minute());
    Serial.println(Time.weekday());
    Serial.println(Time.month());
    Serial.println(Time.year());
  }
  delay(1000);
}
