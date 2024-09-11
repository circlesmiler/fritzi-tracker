#pragma once

void updateDistance(const char *event, const char *data);
int setDistance(String distanceStr);
int setAcknowledged(String acknowledgeStr);
int setMute(String muteStr);
int strToBoolInt(String &muteStr);
int setApiRequestIntervalMinutes(String minutes);

bool isHome();

void playTone();
void updateServo();
void updateMuteLed();
void checkAckButton();
void checkMuteButton();
void resetAcknowledgeStateIfHome();
void blinkHomeLed();

void requestCatPosition();
