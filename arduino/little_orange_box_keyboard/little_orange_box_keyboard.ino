
#include <Arduino.h>
#include <Bounce2.h>    // https://github.com/thomasfredericks/Bounce2
#include <BleKeyboard.h> // https://github.com/wakwak-koba/ESP32-NimBLE-Keyboard
#include <Adafruit_NeoPixel.h>

#define PIXEL GPIO_NUM_3
#define PIXEL_POWER GPIO_NUM_2
#define BUTTON_PIN GPIO_NUM_1
#define MAX_IDLE_MS 1000*60*2
#define INTERVAL 200
#define MUL_RED 4
#define DIV_RED 5

Bounce b = Bounce();
BleKeyboard bleKeyboard("Little Orange Box", "flo");
Adafruit_NeoPixel pixels(1, PIXEL, NEO_GRB + NEO_KHZ800);

uint8_t curr_val = 0;

unsigned long nextSwitch = INTERVAL;
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

  bleKeyboard.begin(); // Begin the gamepad

  b.attach(BUTTON_PIN, INPUT);
  b.interval(25);  

  pixels.clear();
}

bool initialConnection = false;
bool launched = false;

void loop() {
  if (bleKeyboard.isConnected())
  {
    if (!initialConnection) {
      pixels.setPixelColor(0, pixels.Color(0,0,255));
      pixels.show();
      initialConnection = true;
    }
    
    
      b.update();
      
      if (b.rose()){
        nextSwitch = INTERVAL;
        currentTarget = true;
        currentlyOn = false;

        launched = false;
      }

      if (b.fell()) {
          if (!launched) {
            pixels.setPixelColor(0, pixels.Color(0,0,255));
            pixels.show();
          }
      }
      
      if (b.read() == HIGH) {
        if (!launched) {
          if (b.currentDuration()>nextSwitch) {
            nextSwitch += INTERVAL;
            currentTarget = !currentTarget;

            if (nextSwitch > 2000) {
              bleKeyboard.press(KEY_RETURN);
              delay(100);
              bleKeyboard.releaseAll();
              launched = true;
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
      if (bleKeyboard.isConnected()) {
        return;
      }
    }

    if (millis() > MAX_IDLE_MS) {
       digitalWrite(PIXEL_POWER, LOW);
       esp_sleep_enable_ext0_wakeup(BUTTON_PIN, 1);
       esp_deep_sleep_start();
    }
  }
}
