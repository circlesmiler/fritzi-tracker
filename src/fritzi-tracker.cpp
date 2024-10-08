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
#define BUTTON_ACK_PIN D4
#define BUTTON_MUTE_PIN D5
#define MUTE_LED_PIN D3
#define HOME_LED_PIN D2

Servo myservo;
ClickButton ackButton(BUTTON_ACK_PIN, LOW, CLICKBTN_PULLUP);
ClickButton muteButton(BUTTON_MUTE_PIN, LOW, CLICKBTN_PULLUP);

const double HOME_DISTANCE = 5.0;
const double MAX_DISTANCE = 600.0;
const double MAX_SERVO_VALUE = 180.0;
double distance = MAX_DISTANCE;

Cooldown catPosition(5 * 60 * 1000, requestCatPosition);
Cooldown homeLed(500, blinkHomeLed);
Cooldown servo(5000, updateServo);
Cooldown alarmSound(5000, playTone);

bool homeAcknowledged = false;
bool mute = false;

void setup()
{
  Log.info("Setup running...");

  // Subscribe to the integration response event
  Particle.subscribe("hook-response/fritzi", updateDistance);
  Particle.variable("distance", distance);
  Particle.variable("acknowledged", homeAcknowledged);
  Particle.variable("mute", mute);

  Particle.function("setDistance", setDistance);
  Particle.function("setAcknowledged", setAcknowledged);
  Particle.function("setMute", setMute);

  Log.info("Particle cloud setup done.");

  myservo.attach(SERVO_PIN);
  pinMode(HOME_LED_PIN, OUTPUT);
  pinMode(MUTE_LED_PIN, OUTPUT);
  pinMode(BUTTON_ACK_PIN, INPUT_PULLUP);
  pinMode(BUTTON_MUTE_PIN, INPUT_PULLUP);

  Log.info("Testing Servo");
  myservo.write(25);

  Log.info("Setup done.");

  // Wait for cloud connected
  while (!Particle.connected())
  {
    Particle.process();
  }
  Log.info("Cloud connected...");
  requestCatPosition();
}

void loop()
{
  // Update button state
  checkAckButton();
  checkMuteButton();

  updateMuteLed();

  resetAcknowledgeStateIfHome();

  catPosition.update();
  servo.update();
  alarmSound.update();
  homeLed.update();
}

void resetAcknowledgeStateIfHome()
{
  if (!isHome())
  {
    homeAcknowledged = false;
  }
}

void checkMuteButton()
{
  muteButton.Update();
  int muteButtonClicks = muteButton.clicks;
  if (muteButtonClicks == 1)
  {
    Log.info("Mute button clicked");
    mute = !mute;
  }
}

void checkAckButton()
{
  ackButton.Update();
  int ackButtonClicks = ackButton.clicks;
  if (ackButtonClicks == 1)
  {
    Log.info("Acknowledge button clicked");
    homeAcknowledged = true;
  }
}

void updateMuteLed()
{
  int muteLedState = mute ? HIGH : LOW;
  digitalWrite(MUTE_LED_PIN, muteLedState);
}

void updateServo()
{
  // Distance must be between 0 and MAX-DISTANCE
  double myDist = distance < 0 ? 0 : distance;
  myDist = myDist > MAX_DISTANCE ? MAX_DISTANCE : myDist;

  // Calculate servo position (plus correction for meeting the middle)
  double servoValue = (180.0 - (myDist / MAX_DISTANCE * MAX_SERVO_VALUE)) - 11.0;

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
  if (isHome() && !homeAcknowledged && !mute)
  {
    tone(SPEAKER_PIN, 300, 500L);
  }
}

int homeLedState = LOW;
void blinkHomeLed()
{
  if (isHome() && !homeAcknowledged)
  {
    homeLedState = (homeLedState == LOW) ? HIGH : LOW;
    digitalWrite(HOME_LED_PIN, homeLedState);
  }
  else if (isHome())
  {
    digitalWrite(HOME_LED_PIN, HIGH);
  }
  else
  {
    digitalWrite(HOME_LED_PIN, LOW);
  }
}

void requestCatPosition()
{
  Log.info("Requesting cat position");
  Particle.publish("fritzi");
}

/// @brief Callback function the receives data from tracker API call
/// @param event ?
/// @param data the distance data
void updateDistance(const char *event, const char *data)
{
  setDistance(data);
}

int setDistance(String distanceStr)
{
  distance = atof(distanceStr);
  Log.info("Setting distance to: %f", distance);
  return atoi(distanceStr);
}

int setMute(String muteStr)
{
  int muteInt = strToBoolInt(muteStr);
  mute = muteInt == 1 ? true : false;
  return muteInt;
}

int setAcknowledged(String acknowledgeStr)
{
  int ackInt = strToBoolInt(acknowledgeStr);
  homeAcknowledged = ackInt == 1 ? true : false;
  return ackInt;
}

int setApiRequestIntervalMinutes(String minutes)
{
  int min = atoi(minutes);
  catPosition.setInterval(min * 60 * 1000);
  return min;
}

/// @brief Tells if cat is nearby home, see #HOME_DISTANCE
/// @return true, if cat is nearby home, otherwise false
bool isHome()
{
  return distance <= HOME_DISTANCE;
}

int strToBoolInt(String &muteStr)
{
  if (muteStr.equalsIgnoreCase("true"))
  {
    return 1;
  }
  else if (muteStr.equalsIgnoreCase("false"))
  {
    return 0;
  }
  else
  {
    // Try to parse as int
    int m = atoi(muteStr);
    return m;
  }
}
