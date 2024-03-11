#include <Wire.h> // Needed for I2C

#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h> // Click here to get the library: http://librarymanager/All#SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library
SFE_MAX1704X lipo(MAX1704X_MAX17048); // Allow access to all the 17048 features

#include <esp_sleep.h>
#include <Bounce2.h>    // https://github.com/thomasfredericks/Bounce2
#include <BleKeyboard.h> //https://github.com/wakwak-koba/ESP32-NimBLE-Keyboard


#define LED_PIN  GPIO_NUM_14
#define MAIN_PIN GPIO_NUM_32
#define MODE_A_PIN GPIO_NUM_33
#define MODE_B_PIN GPIO_NUM_15
#define POWER_PIN GPIO_NUM_12

#define FADE_STEPS 2
#define DELAY 10

Bounce2::Button button = Bounce2::Button();
BleKeyboard bleKeyboard("Orange Box", "flo", 100);


#define FADING 1
#define SOLID 2
#define FAST_BLINKING 3
volatile uint8_t ledMode = FADING;

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}
void setup()
{
  pinMode(LED_PIN, OUTPUT);
  pinMode(POWER_PIN, OUTPUT);
  pinMode(MAIN_PIN, INPUT_PULLUP);
  pinMode(MODE_A_PIN, INPUT_PULLUP);
  pinMode(MODE_B_PIN, INPUT_PULLUP);
  
	Serial.begin(115200); // Start serial, to output debug data

  Wire.begin();


  button.attach(MAIN_PIN, INPUT_PULLUP);
  button.setPressedState( LOW ); 
  button.interval(20);  

  bleKeyboard.begin();

  // Set up the MAX17048 LiPo fuel gauge:
  if (lipo.begin() == false) // Connect to the MAX17048 using the default wire port
  {
    Serial.println(F("MAX17048 not detected. Please check wiring. Freezing."));
    while (1)
      ;
  }

  delay(1000);

  print_wakeup_reason();


}

bool m = 0;
bool a = 0;
bool b = 0;
bool p = 0;

int s = 0;
int ls = 0;

int l = 0;
int mode = 0;


int brightness = 0;  // how bright the LED is
int fadeAmount = FADE_STEPS;  // how many points to fade the LED by


void ledHandler() {
  if (ledMode==FADING) {
    analogWrite(LED_PIN, brightness);
    brightness = brightness + fadeAmount;
    if (brightness <= 0 || brightness >= 255) {
      fadeAmount = -fadeAmount;
    }
  } else if (ledMode==SOLID) {
    analogWrite(LED_PIN, 255);
    brightness = 0;
  } else if (ledMode == FAST_BLINKING) {
    if (brightness%128 > 64) {
      analogWrite(LED_PIN, 255);
    } else {
      analogWrite(LED_PIN, 0);
    }
    brightness = brightness + fadeAmount;
    if (brightness <= 0 || brightness >= 255) {
      fadeAmount = -fadeAmount;
    }    
  }
}

void loop()
{
  l++;

  button.update();

  p = digitalRead(POWER_PIN);
  m = digitalRead(MAIN_PIN);
  a = digitalRead(MODE_A_PIN);
  b = digitalRead(MODE_B_PIN);

  mode = (a<< 2) + (b<<1);

  s = (p<<4) + (m<<3) +  (a<< 2) + (b<<1);

  if (s!=ls) {
    ls = s;    

    Serial.print(s);
    Serial.print("  MODE: ");
    Serial.print(mode);
    Serial.print("  MAIN_PIN: ");
    Serial.print(m);
    Serial.print("  MODE_A_PIN: ");
    Serial.print(a);
    Serial.print("  MODE_B_PIN: ");
    Serial.print(b);
    Serial.print("  POWER_PIN: ");
    Serial.print(p);



    // Print the variables:
    Serial.print("   Voltage: ");
    Serial.print(lipo.getVoltage());  // Print the battery voltage
    Serial.print("V");

    Serial.print(" Percentage: ");
    Serial.print(lipo.getSOC(), 2); // Print the battery state of charge with 2 decimal places
    Serial.print("%");

    Serial.print(" Change Rate: ");
    Serial.print(lipo.getChangeRate(), 2); // Print the battery change rate with 2 decimal places
    Serial.print("%/hr");
    Serial.println();

    //bleKeyboard.setBatteryLevel(lipo.getSOC());

    if (!p) {
       Serial.println("going for deep sleep!");

      esp_sleep_enable_ext0_wakeup(POWER_PIN, 1);
      Serial.println("Going to sleep now");
      delay(200);
      esp_deep_sleep_start();
    }
  }

  // if mode = MODE_A
  if (bleKeyboard.isConnected())
  {
    if (button.pressed()){
      if (!a) {
        Serial.println("MODE A");
        bleKeyboard.press(KEY_RETURN);
      } else if (!b) {
        Serial.println("MODE B");
        bleKeyboard.press(KEY_MEDIA_PLAY_PAUSE);
      }
      ledMode = FAST_BLINKING;
    } else if (button.released()) {
      bleKeyboard.releaseAll();
      ledMode = SOLID;
    } else {
      if (ledMode != SOLID || ledMode != FAST_BLINKING) {
        ledMode = SOLID;
        ledHandler();
      }
      if (ledMode != SOLID) {
        ledHandler();
      }
    }
  } else {
    ledMode = FADING;
    ledHandler();
  }
  delay(DELAY);
}