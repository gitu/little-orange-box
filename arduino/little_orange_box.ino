
#include <Arduino.h>
#include <Bounce2.h>    // https://github.com/thomasfredericks/Bounce2
#include <BleGamepad.h> // https://github.com/lemmingDev/ESP32-BLE-Gamepad
#include <Adafruit_NeoPixel.h>

#define PIXEL GPIO_NUM_3
#define PIXEL_POWER GPIO_NUM_2
#define BUTTON_PIN GPIO_NUM_1
#define MAX_IDLE_MS 1000*60*2

Bounce b = Bounce();
BleGamepad bleGamepad("Little Orange Box", "flo");
BleGamepadConfiguration bleGamepadConfig;
Adafruit_NeoPixel pixels(1, PIXEL, NEO_GRB + NEO_KHZ800);

uint8_t curr_val = 0;

unsigned long nextSwitch = 1000;
unsigned long currentInterval = 1000;
bool currentlyOn = false;
bool currentTarget = false;

void setup() {
  pinMode(PIXEL_POWER, OUTPUT);
  digitalWrite(PIXEL_POWER, HIGH);
  pinMode(BUTTON_PIN, INPUT); // Setup the button with an internal pull-up

  pixels.begin();

  pixels.setPixelColor(0, pixels.Color(0,255,0));
  pixels.show();
  delay(1000);

  bleGamepad.begin(); // Begin the gamepad

  b.attach(BUTTON_PIN, INPUT);
  b.interval(25);

  pixels.clear();
}

bool initialConnection = false;
bool launched = false;

void loop() {
  if (bleGamepad.isConnected())
  {
    if (!initialConnection) {
      pixels.setPixelColor(0, pixels.Color(0,0,255));
      pixels.show();
      initialConnection = true;
    }


      b.update();

      if (b.rose()){
        nextSwitch = 1000;
        currentInterval = 1000;
        currentTarget = true;
        currentlyOn = false;

        bleGamepad.press(BUTTON_1);
        bleGamepad.release(BUTTON_2);
        launched = false;
      }

      if (b.fell()) {
          bleGamepad.release(BUTTON_1);
          if (!launched) {
            bleGamepad.release(BUTTON_2);
            pixels.setPixelColor(0, pixels.Color(0,0,255));
            pixels.show();
          }
      }

      if (b.read() == HIGH) {
        if (!launched) {
          if (b.currentDuration()>nextSwitch) {
            nextSwitch += currentInterval;
            currentTarget = !currentTarget;
            currentInterval = (2*currentInterval)/3;

            if (currentInterval<20) {
              bleGamepad.press(BUTTON_2);
              launched = true;
              currentTarget = true;
            }
          }
          if (currentTarget != currentlyOn) {
            if (currentTarget) {
                pixels.setPixelColor(0, pixels.Color(255,0,0));
            } else {
                pixels.setPixelColor(0, pixels.Color(0,0,0));
            }
            pixels.show();
            currentlyOn = currentTarget;
          }
        }
      }
  } else {
    initialConnection = false;
    for(long pixelHue = 0; pixelHue < 65536; pixelHue += 256) {
      pixels.setPixelColor(0, pixels.gamma32(pixels.ColorHSV(pixelHue)));
      pixels.show();
      delay(20);
      if (bleGamepad.isConnected()) {
        pixelHue = 65536;
      }
    }

    if (millis() > MAX_IDLE_MS) {
       digitalWrite(PIXEL_POWER, LOW);
       esp_sleep_enable_ext0_wakeup(BUTTON_PIN, 1);
       esp_deep_sleep_start();
    }
  }
}