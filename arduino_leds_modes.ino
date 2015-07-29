// Arduino code to light up LEDS
// and perform various effects with those leds. 
// Based on various online resources: 
// http://www.instructables.com/id/Arduino-Button-Tutorial/
// http://arduino.cc/en/Tutorial/BlinkWithoutDelay
// http://forum.arduino.cc/index.php?topic=282942.0
// http://www.thebox.myzen.co.uk/Tutorial/State_Machine.html
#define BUTTON1_PIN              12  // Button 1
#define DEFAULT_LONGPRESS_LEN    25  // Min nr of loops for a long press
#define DELAY                    20  // Delay per loop in ms
const int indicatorLed         = 13;
enum { EV_NONE=0, EV_SHORTPRESS, EV_LONGPRESS };
// modes ////////////////////////////////////////////////////////////////////////
const int fadeUpGrowOrRetractMode =  0;
const int larsonMode           =  1;
const int shootMode            =  2;
const int fadeUpAndDownMode    =  3;
const int basicMode            =  4;
int currentMode                =  fadeUpGrowOrRetractMode;
int allModes[] = {fadeUpGrowOrRetractMode, larsonMode, shootMode, fadeUpAndDownMode, basicMode};
#define NUMBER_OF_MODES (sizeof(allModes)/sizeof(int))
// main LEDs ///////////////////////////////////////////////////////////////////
// if you want to use the fade in and out mode, pick pins that have pwm
// otherwise all the led segments can use any of the other pins
int led01  = 3;
int led02  = 5;
int led03  = 6;
int led04  = 9;
int led05  = 10;
int led06  = 11;
int allLEDs[]  = {led01, led02, led03, led04, led05, led06};
#define NUMBER_OF_LEDS (sizeof(allLEDs)/sizeof(int))
boolean ledsAreCurrentlyOn = false;
////////////////////////////////////////////////////////////////////////////////
// For Larson Code
unsigned long currentNowTime;
long previousNowTime = 0; // will store last time LED was updated
long larsonInterval = 60; // interval at which to switch to next LED
int larsonUpAndDown[] = {led01, led02, led03, led04, led05, led06, led05, led04, led03, led02};
#define LARSON_LEDS (sizeof(larsonUpAndDown)/sizeof(int))
////////////////////////////////////////////////////////////////////////////////
// Button Code, most of this I left intact from the instructables website
////////////////////////////////////////////////////////////////////////////////
// Class definition
class ButtonHandler {
  public:
    // Constructor
    ButtonHandler(int pin, int longpress_len=DEFAULT_LONGPRESS_LEN);

    // Initialization done after construction, to permit static instances
    void init();

    // Handler, to be called in the loop()
    int handle();

  protected:
    boolean was_pressed;     // previous state
    int pressed_counter;     // press running duration
    const int pin;           // pin to which button is connected
    const int longpress_len; // longpress duration
};

ButtonHandler::ButtonHandler(int p, int lp)
: pin(p), longpress_len(lp)
{
}

void ButtonHandler::init()
{
  pinMode(pin, INPUT);
  digitalWrite(pin, HIGH); // pull-up
  was_pressed = false;
  pressed_counter = 0;
}

int ButtonHandler::handle()
{
  int event;
  int now_pressed = !digitalRead(pin);

  if (!now_pressed && was_pressed) {
    // handle release event
    if (pressed_counter < longpress_len)
      event = EV_SHORTPRESS;
    else
      event = EV_LONGPRESS;
  }
  else
    event = EV_NONE;

  // update press running duration
  if (now_pressed)
    ++pressed_counter;
  else
    pressed_counter = 0;

  // remember state, and we're done
  was_pressed = now_pressed;
  return event;
}

////////////////////////////////////////////////////////////////////////////////
ButtonHandler button1(BUTTON1_PIN); // Instantiate button object
// End of Majority of button code, setup has a button init and loop has the button listening 
////////////////////////////////////////////////////////////////////////////////
void setup() {
  pinMode(indicatorLed, OUTPUT);
  for (int led=0; led<NUMBER_OF_LEDS; led++) {
    pinMode(allLEDs[led], OUTPUT);
  }
  button1.init(); // init buttons pins; I suppose it's best to do here
  indicatorBlinking(2, 50); // A visual way to see when this thing starts
  Serial.begin(9600);
}

void loop() {
  // handle button
  int event1 = button1.handle();
  if (event1 == EV_SHORTPRESS) {
    short_press01();
  } else if (event1 == EV_LONGPRESS) {
    long_press01();
  }
  delay(DELAY);
  //Serial.println("back in the loop!");
}

void short_press01() {
  switch (currentMode) {
    case basicMode: // ignite or withdraw
      if (ledsAreCurrentlyOn == false) {
        grow();
      } else {
        retract();
      }
      ledsAreCurrentlyOn = !ledsAreCurrentlyOn;
      break;
    case larsonMode:
      ledsAreCurrentlyOn = !ledsAreCurrentlyOn;
      larsonScan();
      break;
    case shootMode: 
      //shootOnceEffect();
      threeShotBurstEffect();
      break;
    case fadeUpAndDownMode:
      ledsAreCurrentlyOn = !ledsAreCurrentlyOn;
      fadeUpAndDownEffect();
      break;
    case fadeUpGrowOrRetractMode:
      fadeUpGrowOrRetract();
      // ledsAreCurrentlyOn = !ledsAreCurrentlyOn; // I'm handling this in the code
      // fadeUpGrowOrRetract();
      break;      
  }
}

void long_press01() { // user long presses to activate next mode
  currentMode = currentMode + 1; 
  if (currentMode >= NUMBER_OF_MODES) { // reset back to first mode (0)
    currentMode = 0;
  }
  indicatorBlinking(3, 50); // visually indicate that mode is changed
  turnAllLEDsOff();
}

void indicatorBlinking(int timesToRepeat, int delayTime) {
  for (int i = 0; i < timesToRepeat; i++) {
    digitalWrite(indicatorLed, HIGH); 
    delay(delayTime);
    digitalWrite(indicatorLed, LOW);
    delay(delayTime);
  }
}

void turnAllLEDsOff() {
  for (int led=0; led<NUMBER_OF_LEDS; led++) {
    digitalWrite(allLEDs[led], LOW);
  }
  ledsAreCurrentlyOn = false;
}

void grow() {
  // I like 15/35/55/75/95
  // Weird, backwards works too,
  // 85/65/45/25/5
  // a little fast: 50/40/30/20/10
  digitalWrite(led01, HIGH);
  delay(50);
  digitalWrite(led02, HIGH);
  delay(40);
  digitalWrite(led03, HIGH);
  delay(30);
  digitalWrite(led04, HIGH);
  delay(20);
  digitalWrite(led05, HIGH);
  delay(10);
  digitalWrite(led06, HIGH);
}

void retract() {
  digitalWrite(led06, LOW);
  delay(15);
  digitalWrite(led05, LOW);
  delay(35);
  digitalWrite(led04, LOW);
  delay(55);
  digitalWrite(led03, LOW);
  delay(75);
  digitalWrite(led02, LOW);
  delay(105);
  digitalWrite(led01, LOW);
}

void larsonScan() {
  while ((currentMode == larsonMode) && (ledsAreCurrentlyOn == true)) {
    for (int led=0; led<LARSON_LEDS; led++) {
      digitalWrite(larsonUpAndDown[led], HIGH);
      boolean ledIsOn = true;
      while (ledIsOn == true) { // keep checking if it's time to turn off yet
        currentNowTime = millis();
        long timeCheck = currentNowTime - previousNowTime;
        if(currentNowTime - previousNowTime > larsonInterval) { // is it time to change?
          previousNowTime = currentNowTime; // save the last time you blinked the LED 
          digitalWrite(larsonUpAndDown[led], LOW);
          ledIsOn = false;
        }
      }
      // Gotta put a button handler in here too, because I think
      // the larsonScan code runs too fast for the main loop 
      // to get access to user actions
      int event01 = button1.handle();
      if (event01 == EV_SHORTPRESS) {
        short_press01();
        return;
      } else if (event01 == EV_LONGPRESS) {
        long_press01();
        return;
      }
      delay(DELAY);
    }
  }
}

void shootOnceEffect() {
  // Probably don't need a button handler here
  // since the code in here runs fast
  long shootInterval = 6;
  previousNowTime = millis();
  if (currentMode == shootMode) {
    for (int led=0; led<NUMBER_OF_LEDS; led++) {
      boolean ledIsOn = true;
      digitalWrite(allLEDs[led], HIGH);
      while (ledIsOn == true) { // keep checking if it's time to turn off yet
        currentNowTime = millis();
        if(currentNowTime - previousNowTime > shootInterval) { // is it time to change?
          previousNowTime = currentNowTime; // store the time now for the next LED to compare to
          digitalWrite(allLEDs[led], LOW);
          ledIsOn = false; // get out of this whloop
        }
      }
    }
  }
}

void threeShotBurstEffect() {
  // Probably don't need a button handler here
  // since the code in here runs fast
  long shootInterval = 10; 
  for (int i=0; i<3; i++) {
    previousNowTime = millis();
    if (currentMode == shootMode) {
      for (int led=0; led<NUMBER_OF_LEDS; led++) {
        boolean ledIsOn = true;
        digitalWrite(allLEDs[led], HIGH);
        while (ledIsOn == true) { // keep checking if it's time to turn off yet
          currentNowTime = millis();
          if(currentNowTime - previousNowTime > shootInterval) { // is it time to change?
            previousNowTime = currentNowTime; // store the time now for the next LED to compare to
            digitalWrite(allLEDs[led], LOW);
            ledIsOn = false; // get out of this whloop
          }
        }
      }
    }
    delay(shootInterval);
  }
}

void fadeUpAndDownEffect() {
  // NOTE: This only works if the leds 
  // are connected to PWM pins
  int brightness            = 20;    // how bright the LED is
  int fadePointAmount       = 5;    // how many points to fade the LED by
  long delayBetweenEachFade = 60;   // note: I also add a real dealy for button handler, so account for 20ms
  // for example, if you really want 30ms delay, you'll need to state 50
  while ((currentMode == fadeUpAndDownMode) && (ledsAreCurrentlyOn == true)) {
    currentNowTime = millis();
    if(currentNowTime - previousNowTime > delayBetweenEachFade) { // is it time to change?
      previousNowTime = currentNowTime; // save the last time you blinked the LED 
      for (int led=0; led<NUMBER_OF_LEDS; led++) {
        analogWrite(allLEDs[led], brightness);
      }
      brightness = brightness + fadePointAmount; // change brightness for next time through loop:
    
      // reverse the direction of the fading at the ends of the fade: 
      // if (brightness == 0 || brightness == 255) {
      if (brightness < 20 || brightness == 255) { // I don't like it when it goes completely off, however be careful with the #s
        fadePointAmount = -fadePointAmount;
      }
    }
    //////////////////////////////////////////////////////////////////////
    // Gotta put a button handler in here too, because I think
    // the code inside here runs too fast for the main loop 
    // to get access to user actions
    int event001 = button1.handle();
    if (event001 == EV_SHORTPRESS) { // not sure why this isn't acting like Larson Scan
      short_press01();
      turnAllLEDsOff();
      return;
    } else if (event001 == EV_LONGPRESS) {
      long_press01();
      return;
    }
    delay(DELAY);
    //////////////////////////////////////////////////////////////////////
  }
}

void fadeUpGrowOrRetract () {
  // NOTE: This only works if the leds are connected to PWM pins
  
  // start at 50 brightness because it look way better visually than when starting at 0
  // 0 makes it look like the string is taking forever to 'grow'  
  int startingBrightnessLevel;
  
  int brightness_forLED0, brightness_forLED1, brightness_forLED2, brightness_forLED3, brightness_forLED4, brightness_forLED5, brightness_forLED6;
  int brightness[] = {brightness_forLED1, brightness_forLED2, brightness_forLED3, brightness_forLED4, brightness_forLED5, brightness_forLED6};
  const int TOTAL_BRIGHTNESS_SETTINGS (sizeof(brightness)/sizeof(int));

  int previousBrightness_forLED0, previousBrightness_forLED1, previousBrightness_forLED2, previousBrightness_forLED3, previousBrightness_forLED4, previousBrightness_forLED5, previousBrightness_forLED6;
  int previousBrightness[] = {previousBrightness_forLED1, previousBrightness_forLED2, previousBrightness_forLED3, previousBrightness_forLED4, previousBrightness_forLED5, previousBrightness_forLED6};
  const int TOTAL_PREVIOUS_BRIGHTNESS_SETTINGS (sizeof(previousBrightness)/sizeof(int));

  long startTime_forLEDSequence, startTime_forLED1, startTime_forLED2, startTime_forLED3, startTime_forLED4, startTime_forLED5, startTime_forLED6;
  long startTime[] = {startTime_forLED1, startTime_forLED2, startTime_forLED3, startTime_forLED4, startTime_forLED5, startTime_forLED6};
  
  long previousTime_forLED0, previousTime_forLED1, previousTime_forLED2, previousTime_forLED3, previousTime_forLED4, previousTime_forLED5, previousTime_forLED6;
  long previousTime[] = {previousTime_forLED1, previousTime_forLED2, previousTime_forLED3, previousTime_forLED4, previousTime_forLED5, previousTime_forLED6};
  const int TOTAL_PREVIOUS_STARTING_TIMES (sizeof(previousTime)/sizeof(long));
  for (int n = 0; n<TOTAL_PREVIOUS_STARTING_TIMES; n++) {
    previousTime[n] = 0;
  }
  // Note, there seems to be a limit on how short the dealy between leds can be, due to the slow code. Slower and slower times work fine  
  long delayBetweenLEDs = 35; 

  boolean ledSequenceStarted, led1Triggered, led2Triggered, led3Triggered, led4Triggered, led5Triggered, led6Triggered;
  boolean ledTriggered[] = {led1Triggered, led2Triggered, led3Triggered, led4Triggered, led5Triggered, led6Triggered};
  const int TOTAL_LED_TRIGGERS (sizeof(ledTriggered)/sizeof(boolean));
  for (int n = 0; n<TOTAL_LED_TRIGGERS; n++) {
    ledTriggered[n] = 0;
  }

  boolean loopingAndLooping = true;

  // LIGHT UP THE LEDS oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
  if (ledsAreCurrentlyOn == false) { // this chunk of code starts the leds to light up
    startingBrightnessLevel = 50;
    for (int n = 0; n<TOTAL_BRIGHTNESS_SETTINGS; n++) {
      brightness[n] = startingBrightnessLevel;
    }
    for (int n = 0; n<TOTAL_PREVIOUS_BRIGHTNESS_SETTINGS; n++) {
      previousBrightness[n] = startingBrightnessLevel;
    }
    ledTriggered[0] = true;
    startTime[0] = millis();
    previousTime[0] = millis(); // this will prevent the first led from triggering immediately
    delayBetweenLEDs = 35;
    int fadeUpPointAmount = 5;
    long delayBetweenEachFadeUp = 10;
    while (loopingAndLooping) {
      for (int n=0; n<(NUMBER_OF_LEDS); n++) {
        if (ledTriggered[n] == true) {
          if (brightness[n] < 255) {
            brightness[n] = newBrightness(allLEDs[n], previousTime[n], brightness[n], fadeUpPointAmount, delayBetweenEachFadeUp, false);
            if (previousBrightness[n] != brightness[n]) {
              previousBrightness[n] = brightness[n];
              previousTime[n] = millis();
            }
          } else {
            brightness[n] = 255; // Just ensure the brightness is exactly 255
            analogWrite(allLEDs[n], brightness[n]);
            if (n == (NUMBER_OF_LEDS-1)) {
              // loopingAndLooping = true; // This is killing other booleans for some reason!
              ledsAreCurrentlyOn = true;
              return; // this exits the function altogether, thank god since loopingAndLooping fucks things up
            } 
          }
        } else { // check if it's time to trigger it
          if (ledTriggered[n-1] == true) { // check if the previous LED was triggered
            if ( (millis() - startTime[(n-1)]) >= delayBetweenLEDs ) { // has enough time elapsed?
              startTime[n] = millis(); // if yes, then trigger this LED as started
              ledTriggered[n] = true;
            } // end of checking delay between leds...
          }
        }
      } // end of for loop
    } // while (loopingAndLooping)
  } // end of if ledsAreCurrentlyOn == false; 
  // END OF LEDs starting to light up
  /////////////////////////////////////////////////////////////////////////////////////////////////////////
  // RETRACTING AND FADING the leds out at the same time ____________________________________________________
  if (ledsAreCurrentlyOn == true) { // then fade out the leds, sequentially
    startingBrightnessLevel = 200;
    for (int n = 0; n<TOTAL_BRIGHTNESS_SETTINGS; n++) {
      brightness[n] = startingBrightnessLevel;
    }
    for (int n = 0; n<TOTAL_PREVIOUS_BRIGHTNESS_SETTINGS; n++) {
      previousBrightness[n] = startingBrightnessLevel;
    }
    ledTriggered[(NUMBER_OF_LEDS-1)] = true; // trigger last LED to start
    startTime[(NUMBER_OF_LEDS-1)] = millis();
    previousTime[(NUMBER_OF_LEDS-1)] = millis(); // this will prevent the first led from triggering immediately   
    delayBetweenLEDs = 35;
    int fadeDownPointAmount = 50;
    long delayBetweenEachFadeDown = 10;
    for (int n=(NUMBER_OF_LEDS-1); n>=0; n--) {
      analogWrite(allLEDs[n], brightness[n]);
    }
 
    while (loopingAndLooping) {
      for (int n=(NUMBER_OF_LEDS-1); n>=0; n--) { // Start from the end of the array and work backwards
        if (ledTriggered[n] == true) {
          if (brightness[n] > 0) {
            brightness[n] = newBrightness(allLEDs[n], previousTime[n], brightness[n], fadeDownPointAmount, delayBetweenEachFadeDown, true);
            if (previousBrightness[n] != brightness[n]) {
              previousBrightness[n] = brightness[n];
              previousTime[n] = millis();
            }
          } else {
            brightness[n] = 0; // Just ensure the brightness is exactly 0
            analogWrite(allLEDs[n], brightness[n]);
            if (n == 0) { //that is, the 'first' LED...
              // loopingAndLooping = true; // This is killing other booleans for some reason!
              ledsAreCurrentlyOn = false;
              return; // this exits the function altogether, thank god since loopingAndLooping fucks things up
            } 
          }
        } else { // check if it's time to trigger it
          if (ledTriggered[n+1] == true) { // check if the LED 'after' it was triggered to fade out already
            if ( (millis() - startTime[(n+1)]) >= delayBetweenLEDs ) { // has enough time elapsed?
              startTime[n] = millis(); // if yes, then trigger this LED as started
              ledTriggered[n] = true;
            } // end of checking delay between leds...
          }
        }
      } // end of for loop
      Serial.println("_______________________________________________________");
    } // while (loopingAndLooping)
  } // end of if ledsAreCurrentlyOn == true;  
  // END OF LEDs turning off
    
} // end of void fadeUpGrowOrRetract ()

int newBrightness(int led, long previousNowTime, int brightness, int fadePointAmount00, long delayBetweenEachFade00, boolean fadeOut) {
  if((millis() - previousNowTime) > delayBetweenEachFade00){
    // analogWrite(led, brightness);
    if (fadeOut == true) {
      brightness = brightness - fadePointAmount00; // change brightness for next time through loop:
    } else { // fade up
      brightness = brightness + fadePointAmount00; // change brightness for next time through loop:
    }
    if(brightness < 0) { // negative numbers seem to turn the LED full on
      brightness = 0;
    }
    analogWrite(led, brightness); // I think this needs to be after the new brightness, not sure why I put before
  }
 return brightness;
}

boolean timeForAction (long momentInTimeToCheckFrom, long timeIntervalRequiredForNextAction) {
  if ((millis() - momentInTimeToCheckFrom) >= timeIntervalRequiredForNextAction) {
    return true;
  } else {
    return false;
  }
}
