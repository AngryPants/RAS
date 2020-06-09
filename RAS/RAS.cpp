// Include RAS Files
#include "RAS.h"

DS3231 RAS::s_RTC;
MFRC522 RAS::s_RFIDSensor(PIN_RFID_SS, PIN_RFID_RST);
LiquidCrystal_I2C RAS::s_LCD(0x3F, LCD_COLUMNS, LCD_ROWS);

// Nominal Roll
unsigned int RAS::s_NumPerson = 0;
unsigned int RAS::s_NumIn = 0;

// Scanning
unsigned int RAS::s_PreviousCard = 0;
unsigned long RAS::s_PreviousScanTime = 0;

// LCD
unsigned int RAS::s_NameRotationIndex;
unsigned long RAS::s_PreviousLCDPrintTime;

void RAS::Begin() {
    // Initialise Serial
    Serial.begin(9600);
    while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4) Not relevant at all for our target platform, Arduino Uno.

    // Initialise SPI Bus
    SPI.begin();

    // Initialise RTC Modules
    Wire.begin();
    s_RTC.setClockMode(false); // Set the clock mode to 24h.

    // Initialise LED(s)
    pinMode(LED_BLUE, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED, OUTPUT);

    // Initialise LCD
    s_LCD.begin();

    // LCD Variable(s)
    s_NameRotationIndex = 0;
    s_PreviousLCDPrintTime = RTClib::now().unixtime() - LCD_MESSAGE_DURATION;
    s_LCD.clear();
    s_LCD.setCursor(0, 0);
    s_LCD.print(String(F("System Boot")));
    LightLED(true, true, true);

    /* By default, the default digitalWrite state of the pins are set to LOW.
    When that happens, for some reason both the SD Card Reader & RFID Scanner are using the same SPI Bus or some shit and RFID Sensor will fail.
    One remedy is to use digitalWrite(PIN_SD_CS, HIGH) BEFORE initialising RFID Sensor. Other way is to just initialise SD Reader first, since it
    automatically calls digitalWrite(PIN_SD_CS, HIGH). Or we can do both. */
    // Set all the Chip Select/Slave Select pins to high.
    digitalWrite(PIN_SD_CS, HIGH);
    digitalWrite(PIN_RFID_SS, HIGH);
    
    // Initialise SD Card Reader
    if (!SD.begin(PIN_SD_CS)) {
        s_LCD.clear();
        s_LCD.setCursor(0, 0);
        s_LCD.print(String(F("SD Init Fail")));
        s_PreviousLCDPrintTime = RTClib::now().unixtime();
        Serial.println(F("SD Card initialisation failed."));
        while (true) {} // If this fails, do not continue.
        return;
    }

    // Initialise RFID Sensor
    s_RFIDSensor.PCD_Init();
    delay(4); // Optional delay. Some board do need more time after init to be ready, see MFRC522 Readme.
    Serial.print(F("MFRC522 Card Reader Details: "));
    s_RFIDSensor.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
    
    // RFID Scanning Variable(s)
    s_PreviousScanTime = RTClib::now().unixtime() - COOLDOWN_DURATION;
    s_PreviousCard = 0;

    // Load nominal roll from SD Card.
    LoadNominalRoll();
    
    Serial.println(F("RAS Initialisation OK."));

    // We add a delay here so that we do not cause the reset check to run again if we booted up too fast.
    delay(RESET_DURATION * 1000);
}

void RAS::Update() {
    // Reset the Arduino at midnight. RESET_DURATION is to give it a time buffer, incase it was doing something exactly at midnight, and it misses the reset call.
    DateTime timeNow = RTClib::now();
    if (timeNow.hour() == 0 && timeNow.minute() == 0 && timeNow.second() < RESET_DURATION) {
        /* This reset function looks like some nullptr shit that will crash in a normal computer, but somehow this is a special way to reset shit in Arduino.
        Don't ask me why. Arduino's documentation is shit and basically non existent. It doesn't even tell you its function's return types. Fuck. */
        void (*resetFunc)() = 0;
        resetFunc();
        return;
    }

    LCDNameRotation();

    // Call OnRFIDScan only if we detect a card.
    if (!s_RFIDSensor.PICC_IsNewCardPresent()) { return; }
    if (!s_RFIDSensor.PICC_ReadCardSerial()) { return; }
    OnRFIDScan();
}

RFIDBuffer RAS::GetRFIDBuffer(unsigned int _index) {
    size_t memOffset = _index * TOTAL_BUFFER_SIZE;
    RFIDBuffer buffer;
    EEPROM.get(memOffset, buffer);
    return buffer;
}

NameBuffer RAS::GetNameBuffer(unsigned int _index) {
    size_t memOffset = _index * TOTAL_BUFFER_SIZE + RFID_BUFFER_SIZE;
    NameBuffer buffer;
    EEPROM.get(memOffset, buffer);
    return buffer;
}

NRICBuffer RAS::GetNRICBuffer(unsigned int _index) {
    size_t memOffset = _index * TOTAL_BUFFER_SIZE + RFID_BUFFER_SIZE + NAME_BUFFER_SIZE;
    NRICBuffer buffer;
    EEPROM.get(memOffset, buffer);
    return buffer;
}

StatusBuffer RAS::GetStatusBuffer(unsigned int _index) {
    size_t memOffset = _index * TOTAL_BUFFER_SIZE + RFID_BUFFER_SIZE + NAME_BUFFER_SIZE + NRIC_BUFFER_SIZE;
    StatusBuffer buffer;
    EEPROM.get(memOffset, buffer);
    return buffer;
}

bool RAS::CheckRFID(unsigned int _index) {
    RFIDBuffer rfidBuffer = GetRFIDBuffer(_index);
    return (s_RFIDSensor.uid.uidByte[0] == rfidBuffer.m_Buffer[0]) && (s_RFIDSensor.uid.uidByte[1] == rfidBuffer.m_Buffer[1]) && (s_RFIDSensor.uid.uidByte[2] == rfidBuffer.m_Buffer[2]) && (s_RFIDSensor.uid.uidByte[3] == rfidBuffer.m_Buffer[3]);
}

void RAS::SignIn(unsigned int _index) {
    size_t memOffset = _index * TOTAL_BUFFER_SIZE + RFID_BUFFER_SIZE + NAME_BUFFER_SIZE + NRIC_BUFFER_SIZE;

    // Get the time.
    DateTime timeNow = RTClib::now();
    unsigned short hour = (unsigned short)timeNow.hour();
    unsigned short minute = (unsigned short)timeNow.minute();
    unsigned short time = hour * 100 + minute;

    // Create Mask for Status
    unsigned short mask = 1;
    mask = (mask << 15);

    EEPROM.put(memOffset, (time | mask));
}

void RAS::SignOut(unsigned int _index) {
    size_t memOffset = _index * TOTAL_BUFFER_SIZE + RFID_BUFFER_SIZE + NAME_BUFFER_SIZE + NRIC_BUFFER_SIZE + (STATUS_BUFFER_SIZE / 2);

    // Get the time.
    DateTime timeNow = RTClib::now();
    unsigned short hour = (unsigned short)timeNow.hour();
    unsigned short minute = (unsigned short)timeNow.minute();
    unsigned short time = hour * 100 + minute;

    // Create Mask for Status
    unsigned short mask = 1;
    mask = (mask << 15);

    EEPROM.put(memOffset, (time | mask));
}

bool RAS::LoadNominalRoll() {
    s_NumPerson = 0;
    s_NumIn = 0;

    // Clear our EEPROM so that we can store our NR inside.
    for (int i = 0; i < EEPROM.length(); i++) { EEPROM.write(i, 0); }
    
    // Open File
    File file = SD.open(NR_FILE_PATH, FILE_READ);
    if (!file) {
        s_LCD.clear();
        s_LCD.setCursor(0, 0);
        s_LCD.print(String(F("NR Load Fail")));
        s_PreviousLCDPrintTime = RTClib::now().unixtime();
        Serial.println(F("Nominal roll load failed."));
        while (true) {} // If this fails, do not continue.
        // return false;
    }

    Serial.println(F("Nominal roll loading. Please wait..."));

    for (unsigned int i = 0; i < MAX_PERSON; ++i) {
        // Read our file and store the data into the EEPROM.
        char lineBuffer[64] = { 0 };
        // We use LINE_BUFFER_SIZE - 1 instead of LINE_BUFFER_SIZE because we want the line to end in a null terminator.
        if (!readLine(file, &lineBuffer[0], sizeof(lineBuffer) - 1)) {
            break;
        }

        char* token = 0;
        size_t memOffset = i * TOTAL_BUFFER_SIZE;
        size_t copySize = 0;

        // We are reading a .csv file, so we need to use strtok to seperate the values based on the comma.
        { // Store RFID
            token = strtok(&lineBuffer[0], ",\n");
            RFIDBuffer rfidBuffer;
            sscanf(token, "%d%d%d%d", &rfidBuffer.m_Buffer[0], &rfidBuffer.m_Buffer[1], &rfidBuffer.m_Buffer[2], &rfidBuffer.m_Buffer[3]);
            EEPROM.put(memOffset, rfidBuffer);
        }

        { // Store Name
            token = strtok(NULL, ",\n");
            NameBuffer nameBuffer;
            copySize = Min<size_t>(strlen(token), NAME_BUFFER_SIZE);
            memcpy(&nameBuffer, token, copySize);
            EEPROM.put(memOffset + RFID_BUFFER_SIZE, nameBuffer);
        }

        { // Store NRIC
            token = strtok(NULL, ",\n");
            NRICBuffer nricBuffer;
            copySize = Min<size_t>(strlen(token), NRIC_BUFFER_SIZE);
            memcpy(&nricBuffer, token, copySize);
            EEPROM.put(memOffset + RFID_BUFFER_SIZE + NAME_BUFFER_SIZE, nricBuffer);
        }

        ++s_NumPerson;
        Serial.println(String(F("Loaded ")) + GetName(i) + String(F(".")));
    }

    file.close();
    Serial.println(String(F("Nominal roll load OK. ")) + String(s_NumPerson) + String(F(" people loaded.")));

    return true;
}

bool RAS::SaveLogs() {
    DateTime timeNow = RTClib::now();

    /* File name must follow the 8.3 naming convention! That means a maximum length of 8 characters plus a file extension of a dot with maximum 3 characters!
    Furthermore, in these systems file and directory names are uppercase, although systems that use the 8.3 standard are usually case-insensitive.
    We are setting the filename to be YYYYMMDD.txt. That means if today is 2020-06-24, the file will be 20200624. */
    String filePath = LOGS_DIRECTORY_PATH;
    filePath += String(timeNow.year());
    filePath += (timeNow.month() > 9) ? String(timeNow.month()) : ("0" + String(timeNow.month()));
    filePath += (timeNow.day() > 9) ? String(timeNow.day()) : ("0" + String(timeNow.day()));
    filePath += String(F(".txt"));
    Serial.println(String(F("File Path: ")) + filePath);

    if (!SD.exists(LOGS_DIRECTORY_PATH)) {
        SD.mkdir(LOGS_DIRECTORY_PATH);
    }

    if (SD.exists(filePath)) {
        SD.remove(filePath);
        Serial.println(F("Existing log file found. Deleted existing log file."));
    }

    File file = SD.open(filePath.c_str(), FILE_WRITE);
    if (!file) {
        Serial.println(F("Log file created failed."));
        return false;
    }

    Serial.println(F("Log file created OK."));

    for (unsigned int i = 0; i < s_NumPerson; ++i) {
        file.println(GetNRIC(i) + String(F(" ")) + GetName(i) + String(F(" ")) + GetStatus(i));
    }

    file.close();

    Serial.println(F("Logs save OK."));
    return true;
}

String RAS::GetRFID(unsigned int _index) {
    return GetRFIDBuffer(_index).GetString();
}

String RAS::GetName(unsigned int _index) {
    return GetNameBuffer(_index).GetString();
}

String RAS::GetNRIC(unsigned int _index) {
    return GetNRICBuffer(_index).GetString();
}

String RAS::GetStatus(unsigned int _index) {
    return GetStatusBuffer(_index).GetString();
}

static void RAS::LightLED(bool _blue, bool _yellow, bool _red) {
    digitalWrite(LED_BLUE, _blue ? HIGH : LOW);
    digitalWrite(LED_YELLOW, _yellow ? HIGH : LOW);
    digitalWrite(LED_RED, _red ? HIGH : LOW);
}

void RAS::OnRFIDScan() {
    for (unsigned int i = 0; i < s_NumPerson; ++i) {
        if (!CheckRFID(i)) { continue; }

        /* If the same card has been scanned less then COOLDOWN_DURATION seconds ago, ignore it. Otherwise the same card will instantly sign in this frame and sign out the next frame.
        However, once we detect a different card has been scanned, we know that the first card has been lifted from the scanner and can scan again without waiting for the cooldown.*/        
        long timeSinceLastScan = RTClib::now().unixtime() - s_PreviousScanTime;
        if (i == s_PreviousCard && timeSinceLastScan < COOLDOWN_DURATION) { return; }

        s_PreviousCard = i;
        s_PreviousScanTime = RTClib::now().unixtime();
        
        switch (GetStatusBuffer(i).GetStatus()) {
        case Status::ABSENT:
            SignIn(i);
            SaveLogs();
            ++s_NumIn;

            s_LCD.clear();
            s_LCD.setCursor(0, 0);
            s_LCD.print(String(F("Welcome")));
            s_LCD.setCursor(0, 1);
            s_LCD.print(GetName(i));
            s_PreviousLCDPrintTime = RTClib::now().unixtime();
            LightLED(true, false, false);
            Serial.println(GetName(i) + String(F(" signed in OK.")));
            return;
        case Status::IN:
            SignOut(i);
            SaveLogs();
            --s_NumIn;

            s_LCD.clear();
            s_LCD.setCursor(0, 0);
            s_LCD.print(String(F("Goodbye")));
            s_LCD.setCursor(0, 1);
            s_LCD.print(GetName(i));
            s_PreviousLCDPrintTime = RTClib::now().unixtime();
            LightLED(false, true, false);
            Serial.println(GetName(i) + String(F(" signed out OK.")));
            return;
        default:
            s_LCD.clear();
            s_LCD.setCursor(0, 0);
            s_LCD.print(String(F("Already Out")));
            s_LCD.setCursor(0, 1);
            s_LCD.print(GetName(i));
            s_PreviousLCDPrintTime = RTClib::now().unixtime();
            LightLED(true, true, true);
            Serial.println(GetName(i) + String(F(" has already signed out.")));
            return;
        }
    }
    Serial.println(String(F("Invalid Card.")));
}

void RAS::LCDNameRotation() {
    /* By default, unless someone signs in or out, we want to display the names of the people who have signed in.
    LCD_MESSAGE_DURATION is used to know when to update the LCD so that the user has some time to read the LCD. */
    long timeSinceLastLCDPrint = RTClib::now().unixtime() - s_PreviousLCDPrintTime; // Check how long it has been since the last message was displayed.
    if (timeSinceLastLCDPrint < LCD_MESSAGE_DURATION) { return; }
    s_PreviousLCDPrintTime = RTClib::now().unixtime();

    s_LCD.clear();
    s_LCD.setCursor(0,0);
    s_LCD.print(String(s_NumIn) + String(F(" Signed In.")));
    LightLED(false, false, true);

    // If nobody has signed in, no need to display names.
    if (s_NumIn == 0) { return; }

    while (GetStatusBuffer(s_NameRotationIndex).GetStatus() != Status::IN) {
        s_NameRotationIndex = (s_NameRotationIndex + 1) % s_NumPerson;
    }

    s_LCD.setCursor(0, 1);
    s_LCD.print(GetName(s_NameRotationIndex));
    s_NameRotationIndex = (s_NameRotationIndex + 1) % s_NumPerson;
}
