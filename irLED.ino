#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define BUTTON_PIN 2
#define LED_PIN 13  // debug
#define PIXEL_PIN 6 //tapeLEDPin
#define PIXEL_COUNT 4 //tapeLED Count
#define PIR_PIN 4
#define PIXEL_UPDATE_TIME 10
#define SERIAL_SPEED 9600

Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

int buttonState = 0;
boolean mode = 0;
boolean oldState = HIGH;
boolean _oldState = HIGH;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(PIR_PIN, INPUT);
  // Initialize NeoPixel
  strip.begin();
  strip.show();

  Serial.begin(SERIAL_SPEED);
}

void loop() {
  buttonState = digitalRead(PIR_PIN);
  Serial.println(digitalRead(PIR_PIN));

  switchLedPin();
  switchButtonTapeLed();
  switchIrTapeLed();
}

void switchLedPin() {
  digitalWrite(LED_PIN, buttonState);
}

int swLEDColor(boolean s) {
  if (s) {
    return 127;
  } else {
    return 0;
  }
}

void switchIrTapeLed() {
  boolean _nowState = digitalRead(PIR_PIN);
  if (_nowState != _oldState ) {
    mode = _nowState;
    colorWipe(strip.Color(swLEDColor(mode), swLEDColor(mode), swLEDColor(mode)), PIXEL_UPDATE_TIME);
    mode = !mode;
  }
  _oldState = digitalRead(PIR_PIN);
}

void switchButtonTapeLed() {
  boolean newState = digitalRead(BUTTON_PIN);
  if ((newState == LOW) && (oldState == HIGH)) {
    delay(20);
    newState = digitalRead(BUTTON_PIN);
    if (newState == LOW) {
      colorWipe(strip.Color(swLEDColor(mode), swLEDColor(mode), swLEDColor(mode)), PIXEL_UPDATE_TIME);
      mode = !mode;
    }
  }
  oldState = newState;
}

void colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(wait);
  }
}
