#include <RTClib.h>
#include <Wire.h>

RTC_DS3231 RTC;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  RTC.adjust(DateTime(2020, 5, 29, 23, 58, 0));
  Serial.println("Time has been set to 2020-5-29, 2358H. You may now close the program.");
}

void loop() {
}
