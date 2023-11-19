/*
 * Project Fritzi Tracker
 * Author: André Schade
 * Date: 2023-11-18
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "clickButton.h"
#include "Cooldown.h"
#include "fritzi-tracker.h"

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Run the application and system concurrently in separate threads
SYSTEM_THREAD(ENABLED);

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
SerialLogHandler logHandler(LOG_LEVEL_INFO);

#define SERVO_PIN D0
#define SPEAKER_PIN D1
#define BUTTON_PIN D4
#define HOME_LED_PIN D7

Servo myservo;
ClickButton button(BUTTON_PIN, LOW, CLICKBTN_PULLUP);

const double HOME_DISTANCE = 5.0;
const double MAX_DISTANCE = 50.0;
const double MAX_SERVO_VALUE = 180.0;
double distance = MAX_DISTANCE;

Cooldown catPosition(5 * 60 * 1000, requestCatPosition);
Cooldown homeLed(500, blinkHomeLed);
Cooldown servo(5000, updateServo);
Cooldown alarmSound(5000, playTone);

bool homeAcknowledged = false;

void setup()
{
  Log.info("Setup running...");

  // Subscribe to the integration response event
  Particle.subscribe("hook-response/fritzi", updateDistance, MY_DEVICES);
  Particle.variable("distance", distance);
  Particle.function("setDistance", setDistance);
  Log.info("Particle cloud setup done.");

  myservo.attach(SERVO_PIN);     // attach the servo on the D0 pin to the servo object
  pinMode(HOME_LED_PIN, OUTPUT); // set D7 as an output so we can flash the onboard LED
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Setup button timers (all in milliseconds / ms)
  // (These are default if not set, but changeable for convenience)
  button.debounceTime = 20;    // Debounce timer in ms
  button.multiclickTime = 250; // Time limit for multi clicks
  button.longClickTime = 1000; // time until "held-down clicks" register

  Log.info("Testing Servo");
  myservo.write(25);

  Log.info("Setup done.");
}

void loop()
{
  // Update button state
  button.Update();
  int buttonClicks = button.clicks;
  if (buttonClicks == 1)
  {
    Log.info("Button clicked");
    homeAcknowledged = true;
  }
  if (buttonClicks == 2)
  {
    Log.info("Button clicked twice");
    homeAcknowledged = false;
  }

  catPosition.update();
  servo.update();
  alarmSound.update();
  homeLed.update();
}

void updateServo()
{
  // Distance must be between 0 and MAX-DISTANCE
  double myDist = distance < 0 ? 0 : distance;
  myDist = myDist > MAX_DISTANCE ? MAX_DISTANCE : myDist;

  // Calculate servo position (plus correction for meeting the middle)
  double servoValue = (myDist / MAX_DISTANCE * MAX_SERVO_VALUE) - 11.0;

  // Left and right is not exactly 180° and 0°... fixing it here
  int fixedServoValue = (int)servoValue;
  if (fixedServoValue <= 2)
  {
    fixedServoValue = 2;
  }
  if (fixedServoValue >= 165)
  {
    fixedServoValue = 165;
  }
  Log.info("Update servo to: %d", fixedServoValue);
  myservo.write(fixedServoValue);
}

void playTone()
{
  if (isHome() && !homeAcknowledged)
  {
    tone(SPEAKER_PIN, 300, 2000L);
  }
}

int homeLedState = LOW;
void blinkHomeLed()
{
  if (isHome() && !homeAcknowledged)
  {
    homeLedState = (homeLedState == LOW) ? HIGH : LOW;
    digitalWrite(HOME_LED_PIN, homeLedState);
  } else if (isHome()){
    digitalWrite(HOME_LED_PIN, HIGH);
  } else {
    digitalWrite(HOME_LED_PIN, LOW);
  }
}

void requestCatPosition()
{
  Particle.publish("fritzi");
}

/// @brief Callback function the receives data from tracker API call
/// @param event ?
/// @param data the distance data
void updateDistance(const char *event, const char *data)
{
  setDistance(data);
}

/// @brief Converts the given string to a number and sets the distance value
/// @param distanceStr distance as string
/// @return 0, if setting was OK
int setDistance(String distanceStr)
{
  distance = atoi(distanceStr);
  Log.info("Setting distance to: %f", distance);
  return 0;
}

/// @brief Tells if cat is nearby home, see #HOME_DISTANCE
/// @return true, if cat is nearby home, otherwise false
bool isHome()
{
  return distance <= HOME_DISTANCE;
}
