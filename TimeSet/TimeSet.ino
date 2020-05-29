#include <RTClib.h>
#include <Wire.h>

RTC_DS3231 RTC;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  RTC.adjust(DateTime(__DATE__, __TIME__));
  Serial.println("Time has been set. You may now close the program.");
}

void loop() {
}
