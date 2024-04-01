#include <Wire.h> // Needed for I2C

#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h> // Click here to get the library: http://librarymanager/All#SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library
SFE_MAX1704X lipo(MAX1704X_MAX17048); // Allow access to all the 17048 features

#include <esp_sleep.h>
#include <Bounce2.h>    // https://github.com/thomasfredericks/Bounce2
#include <BleKeyboard.h> //https://github.com/wakwak-koba/ESP32-NimBLE-Keyboard
#include <math.h>

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
#define SLOW_BLINKING 4
#define BREATHING 5
#define WAVE 6

volatile uint8_t ledMode = SLOW_BLINKING;

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  pinMode(POWER_PIN, INPUT);
  pinMode(MAIN_PIN, INPUT_PULLUP);
  pinMode(MODE_A_PIN, INPUT_PULLUP);
  pinMode(MODE_B_PIN, INPUT_PULLUP);
  
	Serial.begin(115200); // Start serial, to output debug data

  Wire.begin();


  button.attach(MAIN_PIN, INPUT_PULLUP);
  button.setPressedState( LOW ); 
  button.interval(20);  


  // Set up the MAX17048 LiPo fuel gauge:
  if (lipo.begin() == false) // Connect to the MAX17048 using the default wire port
  {
    Serial.println(F("MAX17048 not detected. Please check wiring. Freezing."));
    while (1)
      ;
  }

  bleKeyboard.setBatteryLevel(lipo.getSOC());
  bleKeyboard.begin();
}

bool mainButtonPressed = 0;
bool toggleA = 0;
bool toggleB = 0;
bool toggleOn = 0;

int fullState = 0;
int lastState = 0;

int mode = 0;


int brightness = 0;  // how bright the LED is
int fadeAmount = FADE_STEPS;  // how many points to fade the LED by
uint8_t batteryLevel =0;


void loop()
{
  updateLoopState();

  if (fullState!=lastState) {
    lastState = fullState;    

    printState();

    if (!toggleOn) {
      startDeepSleep();
    }
  }

  if (bleKeyboard.isConnected()) {
    handleKeyboard();
  } else {
    if (button.pressed()) {
      ledMode = (ledMode%6)+1;
    }
  }
  ledHandler();
  delay(DELAY);
}

void handleKeyboard() {
  if (button.pressed()){
    if (!toggleA) {
      Serial.println("MODE A");
      bleKeyboard.press(KEY_RETURN);
    } else if (!toggleB) {
      Serial.println("MODE B");
      bleKeyboard.press(KEY_MEDIA_PLAY_PAUSE);      
    }
    ledMode = FAST_BLINKING;
  } else if (button.released()) {
    bleKeyboard.releaseAll();
  } else {
    ledMode = WAVE;
  }
}

void startDeepSleep() {
  esp_sleep_enable_ext0_wakeup(POWER_PIN, 1);
  Serial.println("Going to sleep now");
  delay(10);
  esp_deep_sleep_start();
}

void updateLoopState() {
  button.update();

  toggleOn = digitalRead(POWER_PIN);
  mainButtonPressed = digitalRead(MAIN_PIN);
  toggleA = digitalRead(MODE_A_PIN);
  toggleB = digitalRead(MODE_B_PIN);

  mode = (toggleA<< 2) + (toggleB<<1);
  fullState = (button.changed()<<6) + (button.isPressed()<<5) + (toggleOn<<4) + (mainButtonPressed<<3) +  (toggleA<< 2) + (toggleB<<1);
  
  batteryLevel = lipo.getSOC();
  bleKeyboard.setBatteryLevel(batteryLevel);
}

void printState() {
    Serial.print(fullState);
    Serial.print("  MODE: ");
    Serial.print(mode);
    Serial.print("  button.changed: ");
    Serial.print(button.changed());
    Serial.print("  button.isPressed: ");
    Serial.print(button.isPressed());
    Serial.print("  MAIN_PIN: ");
    Serial.print(mainButtonPressed);
    Serial.print("  MODE_A_PIN: ");
    Serial.print(toggleA);
    Serial.print("  MODE_B_PIN: ");
    Serial.print(toggleB);
    Serial.print("  POWER_PIN: ");
    Serial.print(toggleOn);

    // Print the variables:
    Serial.print("   Voltage: ");
    Serial.print(lipo.getVoltage());  // Print the battery voltage
    Serial.print("V");


    Serial.print(" Percentage: ");
    Serial.print(lipo.getSOC(), 2); // Print the battery state of charge with 2 decimal places
    Serial.print("%");
    Serial.print(" (");
    Serial.print(batteryLevel);
    Serial.print(")");

    Serial.print(" Change Rate: ");
    Serial.print(lipo.getChangeRate(), 2); // Print the battery change rate with 2 decimal places
    Serial.print("%/hr");
    Serial.print(" - ");
    Serial.print(ledMode);
    Serial.println();
}


int blinkCounter = 0;
float wavePosition = 0.0;
bool blinkOn = false;
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
    blinkCounter++;
    if (blinkCounter % 50 == 0) {
      digitalWrite(LED_PIN, blinkOn);
      blinkOn = !blinkOn;
    }
  } else if (ledMode == SLOW_BLINKING) {
    blinkCounter++;
    if (blinkCounter % 100 == 0) {
      digitalWrite(LED_PIN, blinkOn);
      blinkOn = !blinkOn;
    }
  } else if (ledMode == BREATHING) {
    analogWrite(LED_PIN, brightness);
    blinkCounter++;
    // Calculate the brightness based on a sine wave, using breathCounter as the time variable
    float cycleTime = 400.0; // The duration of one breathing cycle in "ticks" of breathCounter
    float breathPhase = (blinkCounter % (unsigned long)cycleTime) / cycleTime; // Normalized phase of the cycle
    int brightness = (sin(breathPhase * 2 * PI) + 1) * 127.5; // sine wave for breathing effect
    
    analogWrite(LED_PIN, brightness);
  } else if (ledMode == WAVE) {
    // WAVE mode logic
    wavePosition += 0.15; // Increment wave position to progress through the sine wave
    if (wavePosition > 2 * PI) { // Reset wave position after completing a cycle
      wavePosition -= 2 * PI;
    }
    // Calculate brightness based on sine wave, scaling it to the 0-255 range
    brightness = (sin(wavePosition) + 1) * 127.5; // sin() returns a value between -1 and 1
    analogWrite(LED_PIN, brightness);
  }
}