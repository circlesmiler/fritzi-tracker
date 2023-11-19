/*
 * Project Fritzi Tracker
 * Author: André Schade
 * Date: 2023-11-18
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "clickButton.h"

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

Servo myservo;
ClickButton button(BUTTON_PIN, LOW, CLICKBTN_PULLUP);

const double HOME_DISTANCE = 5.0;
const double MAX_DISTANCE = 50.0;
const double MAX_SERVO_VALUE = 180.0;
double distance = MAX_DISTANCE;

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousApiCall = 0;            // will store last time LED was updated
const long API_CALL_INTERVAL = 5 * 60 * 1000; // millis

unsigned long previousUpdate = 0;      // will store last time LED was updated
const long UPDATE_INTERVAL = 5 * 1000; // millis

bool homeAcknowledged = false;

int setDistance(String distanceStr)
{
  distance = atoi(distanceStr);
  Log.info("Setting distance to: %f", distance);
  return 0;
}

void updateData(const char *event, const char *data)
{
  setDistance(data);
}

void setup()
{
  Log.info("Setup running...");

  // Subscribe to the integration response event
  Particle.subscribe("hook-response/fritzi", updateData, MY_DEVICES);
  Particle.variable("distance", distance);
  Particle.variable("previousMillis", previousApiCall);
  Particle.function("setDistance", setDistance);
  Log.info("Particle cloud setup done.");

  myservo.attach(SERVO_PIN); // attach the servo on the D0 pin to the servo object
  pinMode(D7, OUTPUT);       // set D7 as an output so we can flash the onboard LED
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Setup button timers (all in milliseconds / ms)
  // (These are default if not set, but changeable for convenience)
  button.debounceTime = 20;    // Debounce timer in ms
  button.multiclickTime = 250; // Time limit for multi clicks
  button.longClickTime = 1000; // time until "held-down clicks" register

  Log.info("Testing Servo");
  myservo.write(0);
  delay(1000);
  myservo.write(90); // test the servo by moving it to 90°
  delay(1000);
  myservo.write(180);

  Log.info("Setup done.");
}

void updateLight()
{
  // Light on if Fritzi @ Home
  if (distance < HOME_DISTANCE)
  {
    digitalWrite(D7, HIGH);
  }
  else
  {
    digitalWrite(D7, LOW);
  }
}

void playTone()
{
  if (!homeAcknowledged && distance < HOME_DISTANCE)
  {
    tone(SPEAKER_PIN, 440, 2000L);
  }
}

void updateServo()
{
  // Distance must be between 0 and MAX-DISTANCE
  double myDist = distance < 0 ? 0 : distance;
  myDist = myDist > MAX_DISTANCE ? MAX_DISTANCE : myDist;

  // Calculate servo position (plus correction for meeting the middle)
  double servoValue = (myDist / MAX_DISTANCE * MAX_SERVO_VALUE) - 11.0;

  // Left and right is not exactly 180° and 0°... fixing it here
  int fixedServoValue = (int) servoValue;
  if (fixedServoValue <= 2) {
    fixedServoValue = 2;
  }
  if (fixedServoValue >= 165) {
    fixedServoValue = 165;
  }
  Log.info("Update servo to: %d", fixedServoValue);
  myservo.write(fixedServoValue);
}

void loop()
{
  unsigned long currentMillis = millis();

  // Update button state
  button.Update();
  int buttonClicks = button.clicks;
  if (buttonClicks == 1)
  {
    Log.info("Button clicked");
    homeAcknowledged = true;
  }
  if (buttonClicks == 2) {
    Log.info("Button clicked twice");
    homeAcknowledged = false;
  }

  if (currentMillis - previousApiCall >= API_CALL_INTERVAL)
  {
    previousApiCall = currentMillis;
    Particle.publish("fritzi");
  }

  if (currentMillis - previousUpdate >= UPDATE_INTERVAL)
  {
    previousUpdate = currentMillis;
    updateServo();
    updateLight();
    playTone();
  }
}
