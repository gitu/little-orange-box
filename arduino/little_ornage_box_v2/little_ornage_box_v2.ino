
#define USE_NIMBLE

#include <Arduino.h>
#include <Bounce2.h>    // https://github.com/thomasfredericks/Bounce2
#include <BleKeyboard.h> //https://github.com/wakwak-koba/ESP32-NimBLE-Keyboard

#define MODE_POWER GPIO_NUM_8
#define MODE_A_PIN GPIO_NUM_7
#define MODE_B_PIN GPIO_NUM_9

#define LED_POWER GPIO_NUM_1
#define LED_GND GPIO_NUM_2

#define BUTTON_POWER GPIO_NUM_4
#define BUTTON GPIO_NUM_5
#define BUTTON_PRESSED_STATE LOW

Bounce2::Button button = Bounce2::Button();
BleKeyboard bleKeyboard("Orange Box", "flo", 100);

uint8_t mode = 0;
#define MODE_A 1
#define MODE_B 2

#define BLINKING 1
#define SOLID 2
volatile uint8_t ledMode = BLINKING;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_POWER, OUTPUT);
  digitalWrite(BUTTON_POWER, LOW);
  pinMode(LED_GND, OUTPUT);
  digitalWrite(LED_GND, LOW);
  pinMode(LED_POWER, ANALOG);
  analogWrite(LED_POWER, 0);
  pinMode(MODE_POWER, OUTPUT);
  digitalWrite(MODE_POWER, LOW);


  pinMode(MODE_A_PIN, INPUT_PULLUP);
  pinMode(MODE_B_PIN, INPUT_PULLUP);

  button.attach(BUTTON, INPUT_PULLUP);
  button.setPressedState( BUTTON_PRESSED_STATE ); 
  button.interval(5);  

  bleKeyboard.begin();

  Serial.println("setup done!");
}

int brightness = 0;  
int fadeAmount = 5;

void ledHandler() {
  if (ledMode==BLINKING) {
    analogWrite(LED_POWER, brightness);
    brightness = brightness + fadeAmount;
    if (brightness <= 0 || brightness >= 255) {
      fadeAmount = -fadeAmount;
    }
  } else if (ledMode==SOLID) {
    analogWrite(LED_POWER, 255);
    brightness = 0;
  }
}

void loop() {
  button.update();

  // if mode = MODE_A
  if (bleKeyboard.isConnected())
  {
    if (ledMode != SOLID) {
      ledMode = SOLID;
      ledHandler();
    }
    if (button.pressed()){
      if (digitalRead(MODE_A_PIN)) {
        Serial.println("MODE A");
        bleKeyboard.press(KEY_RETURN);
      } else if (digitalRead(MODE_B_PIN)) {
        Serial.println("MODE B");
        bleKeyboard.press(KEY_MEDIA_PLAY_PAUSE);
      }
    } else if (button.released()) {
      bleKeyboard.releaseAll();
    }
  } else {
    ledMode = BLINKING;
    ledHandler();
  }
  delay(10);
}
