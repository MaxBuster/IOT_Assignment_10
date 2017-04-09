// The Following LINE ADDED - it disables WiFi
SYSTEM_MODE(MANUAL);

int fadeTimeInMillis = 5000;
int numIntervalsPerSec = 50;
int updateIntervalInMillis = fadeTimeInMillis/numIntervalsPerSec;

int defaultOnColor[] = {255, 255, 255};
int defaultOffColor[] = {0, 0, 0};

int currentColor[3] = {0, 0, 0};
int targetColor[3] = {0, 0, 0};
float colorDeltas[3] = {0, 0 , 0};

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
  if (arraysEqual(targetColor, currentColor)) {
    return;
  }
  currentColor[0] += colorDeltas[0];
  currentColor[1] += colorDeltas[1];
  currentColor[2] += colorDeltas[2];
  RGB.color(currentColor[0], currentColor[1], currentColor[2]);
}

Timer updateColors(updateIntervalInMillis, updateColor);

void crossFade(int r, int g, int b) {
  setArray(targetColor, r, g, b);
  colorDeltas[0] = (targetColor[0] - currentColor[0])/numIntervalsPerSec;
  colorDeltas[1] = (targetColor[1] - currentColor[1])/numIntervalsPerSec;
  colorDeltas[2] = (targetColor[2] - currentColor[2])/numIntervalsPerSec;
}

void setup() {
  Serial.begin(9600);
  updateColors.start();
  RGB.control(true);
  RGB.color(defaultOnColor[0], defaultOnColor[1], defaultOnColor[2]);
}

void loop() {
  crossFade(255, 0, 0);
  delay(5000);
  crossFade(0, 255, 0);
  delay(5000);
  crossFade(0, 0, 255);
  delay(5000);
}
