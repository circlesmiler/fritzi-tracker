#pragma once

void updateDistance(const char *event, const char *data);
int setDistance(String distanceStr);

bool isHome();

void playTone();
void updateServo();
void blinkHomeLed();

void requestCatPosition();
