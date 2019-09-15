#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#include <avr/sleep.h>                  // スリープライブラリ
#include <avr/wdt.h>                    // ウォッチドッグタイマー ライブラリ

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
  Serial.println(digitalRead(PIR_PIN));
  delayWDT2(6);
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


// ここから下を全て使う
void delayWDT2(unsigned long t) {       // パワーダウンモードでdelayを実行
  Serial.flush();                       // シリアルバッファが空になるまで待つ
  delayWDT_setup(t);                    // ウォッチドッグタイマー割り込み条件設定

  // ADCを停止（消費電流 147→27μA）
  ADCSRA &= ~(1 << ADEN);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // パワーダウンモード指定
  sleep_enable();

  // BODを停止（消費電流 27→6.5μA）
  MCUCR |= (1 << BODSE) | (1 << BODS);   // MCUCRのBODSとBODSEに1をセット
  MCUCR = (MCUCR & ~(1 << BODSE)) | (1 << BODS);  // すぐに（4クロック以内）BODSSEを0, BODSを1に設定

  asm("sleep");                         // 3クロック以内にスリープ sleep_mode();では間に合わなかった

  sleep_disable();                      // WDTがタイムアップでここから動作再開
  ADCSRA |= (1 << ADEN);                // ADCの電源をON（BODはハードウエアで自動再開される）
}

void delayWDT_setup(unsigned int ii) {  // ウォッチドッグタイマーをセット。
  // 引数はWDTCSRにセットするWDP0-WDP3の値。設定値と動作時間は概略下記
  // 0=16ms, 1=32ms, 2=64ms, 3=128ms, 4=250ms, 5=500ms
  // 6=1sec, 7=2sec, 8=4sec, 9=8sec
  byte bb;
  if (ii > 9 ) {                        // 変な値を排除
    ii = 9;
  }
  bb = ii & 7;                          // 下位3ビットをbbに
  if (ii > 7) {                         // 7以上（7.8,9）なら
    bb |= (1 << 5);                     // bbの5ビット目(WDP3)を1にする
  }
  bb |= ( 1 << WDCE );

  MCUSR &= ~(1 << WDRF);                // MCU Status Reg. Watchdog Reset Flag ->0
  // start timed sequence
  WDTCSR |= (1 << WDCE) | (1 << WDE);   // ウォッチドッグ変更許可（WDCEは4サイクルで自動リセット）
  // set new watchdog timeout value
  WDTCSR = bb;                          // 制御レジスタを設定
  WDTCSR |= _BV(WDIE);
}

ISR(WDT_vect) {                         // WDTがタイムアップした時に実行される処理
  //  wdt_cycle++;                      // 必要ならコメントアウトを外す
}
