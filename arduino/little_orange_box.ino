
#include <Arduino.h>
#include <Bounce2.h>    // https://github.com/thomasfredericks/Bounce2
#include <BleGamepad.h> // https://github.com/lemmingDev/ESP32-BLE-Gamepad
#include <Adafruit_NeoPixel.h>

#define PIXEL GPIO_NUM_3
#define PIXEL_POWER GPIO_NUM_2
#define BUTTON_PIN GPIO_NUM_1
#define MAX_IDLE_MS 1000*60*30

Bounce debouncer = Bounce();
BleGamepad bleGamepad("Little Orange Box", "flo");
BleGamepadConfiguration bleGamepadConfig;
Adafruit_NeoPixel pixels(1, PIXEL, NEO_GRB + NEO_KHZ800);

uint8_t curr_val = 0;

unsigned long lastInteraction = 0;

void setup() {
  pixels.begin();
  pinMode(PIXEL_POWER, OUTPUT);
  digitalWrite(PIXEL_POWER, HIGH);

  pixels.setPixelColor(0, pixels.Color(0,255,0));
  pixels.show();
  delay(2000);

  bleGamepad

  bleGamepad.begin(); // Begin the gamepad

  pinMode(BUTTON_PIN, INPUT); // Setup the button with an internal pull-up

  debouncer.attach(BUTTON_PIN); // After setting up the button, setup the Bounce instance :
  debouncer.interval(5);        // interval in ms

}

void loop() {
  pixels.clear();

  if (bleGamepad.isConnected())
  {
      debouncer.update(); // Update the Bounce instance

      int value = debouncer.read(); // Get the updated value

      // Press/release gamepad button and turn on or off the LED as determined by the state
      if (value == LOW)
      {
          pixels.setPixelColor(0, pixels.Color(0,0,255));
          pixels.show();
          delay(2);
          bleGamepad.release(BUTTON_1);
      }
      else
      {
          lastInteraction = millis();
          pixels.setPixelColor(0, pixels.Color(255,0,0));
          pixels.show();
          delay(2);
          bleGamepad.press(BUTTON_1);
      }


  } else {
    for(long pixelHue = 0; pixelHue < 65536 && !bleGamepad.isConnected(); pixelHue += 256) {
      pixels.setPixelColor(0, pixels.gamma32(pixels.ColorHSV(pixelHue)));
      pixels.show();
      delay(20);
    }
  }

  if (millis()-lastInteraction > MAX_IDLE_MS) {
      digitalWrite(PIXEL_POWER, LOW);
      esp_sleep_enable_ext0_wakeup(BUTTON_PIN,HIGH);
      esp_deep_sleep_start();
  }
}