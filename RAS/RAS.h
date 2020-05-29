#ifndef RAS_H
#define RAS_H

// Include Arduino Libraries
#include <EEPROM.h>
#include <WString.h>
#include <MFRC522.h>
#include <SPI.h>
#include <DS3231.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>

// Include RAS
#include "RAS_FileUtil.h"
#include "RAS_MathUtil.h"
#include "RAS_Buffer.h"

// Physical Pin(s) For SD Card Module
#define PIN_SD_CS 4

// Physical Pin(s) For RFID
#define PIN_RFID_SS 10
#define PIN_RFID_RST 9

// Physical Pin(s) For LED
#define LED_BLUE 5
#define LED_YELLOW 6
#define LED_RED 7

// LCD Screen Size
#define LCD_COLUMNS 16
#define LCD_ROWS 2

// Maximum number of people the Arduino can handle. Limited by RAM.
#define MAX_PERSON 20
// Cooldown in seconds before the same card can scan again.
#define COOLDOWN_DURATION 30

// Duration in seconds between LCD message changing.
#define LCD_MESSAGE_DURATION 3

/* The device is set to reboot every midnight. In the event that it was processing something,
it might miss the exact midnight mark during the update loop, and fail to trigger the reboot.
Therefore we will check for midnight to midnight and 3 seconds to trigger the reboot. */
#define RESET_DURATION 3

// File Path to Load & Save
#define NR_FILE_PATH "NR.csv"
#define LOGS_DIRECTORY_PATH "LOGS/"

class RAS {
private:
    RAS() {}
    ~RAS() {}

    // RFID Sensor
    static MFRC522 s_RFIDSensor;

    // RTC Module
    static DS3231 s_RTC;

    // LCD Screen
    static LiquidCrystal_I2C s_LCD;

    // Nominal Roll
    static unsigned int s_NumPerson; // Number of people loaded from the nominal roll.
    static unsigned int s_NumIn; // Number of people who signed in.

    // Scanning
    static unsigned int s_PreviousCard; // What was the last card to be scanned?
    static unsigned long s_PreviousScanTime; // When was the previous card scanned?

    // LCD
    static unsigned int s_NameRotationIndex; // The name of the signed-in personnel to display on the LCD screen.
    static unsigned long s_PreviousLCDPrintTime; // Time since the last message was displayed on the LCD. Measured in seconds.

    static RFIDBuffer GetRFIDBuffer(unsigned int _index);
    static NameBuffer GetNameBuffer(unsigned int _index);
    static NRICBuffer GetNRICBuffer(unsigned int _index);
    static StatusBuffer GetStatusBuffer(unsigned int _index);

    static bool CheckRFID(unsigned int _index);
    static void SignIn(unsigned int _index);
    static void SignOut(unsigned int _index);

    static bool LoadNominalRoll();
    static bool SaveLogs();

    static String GetRFID(unsigned int _index);
    static String GetName(unsigned int _index);
    static String GetNRIC(unsigned int _index);
    static String GetStatus(unsigned int _index);

    static void LightLED(bool _blue, bool _yellow, bool _red);

    static void OnRFIDScan();
    static void LCDNameRotation();
    
public:
    static void Begin();
    static void Update();
};

#endif
