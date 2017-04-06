// The Following LINE ADDED - it disables WiFi
SYSTEM_MODE(MANUAL);

int[] defaultOnColor = new int[3];
int[] defaultOffColor = new int[3];

int[] currentColor = new int[3];
int[] targetColor = new int[3];

void setup() {
  Serial.begin(9600);
  RGB.control(true);
  RGB.color(100, 100, 100);
}

void loop() {
  // put your main code here, to run repeatedly:

}
