#ifndef RAS_BUFFER_H
#define RAS_BUFFER_H

#define RFID_BUFFER_SIZE 4
#define NAME_BUFFER_SIZE 12
#define NRIC_BUFFER_SIZE 4
#define STATUS_BUFFER_SIZE 4
#define TOTAL_BUFFER_SIZE (RFID_BUFFER_SIZE + NAME_BUFFER_SIZE + NRIC_BUFFER_SIZE + STATUS_BUFFER_SIZE)

/***
  We are going to store our Nominal Roll in our EEPROM.
  The EEPROM is a big non-volatile memory buffer.
  We are going to pack our data as such:
    RFID, NAME, NRIC, TIME_IN, TIME_OUT, RFID, NAME, NRIC, TIME_IN, TIME_OUT, RFID, NAME, NRIC, TIME_IN, TIME_OUT, RFID, NAME, NRIC, TIME_IN, TIME_OUT,...
***/
enum class Status {
    ABSENT,
    IN,
    OUT,
};

struct RFIDBuffer {
    byte m_Buffer[RFID_BUFFER_SIZE] = { 0 };

    String GetString() {
        String string;
        for (int i = 0; i < RFID_BUFFER_SIZE; ++i) {
            string += String((int)m_Buffer[i]);
            if (i != RFID_BUFFER_SIZE - 1) {
                string += " ";
            }
        }
        return string;
    }
};

struct NameBuffer {
    byte m_Buffer[NAME_BUFFER_SIZE] = { 0 };

    String GetString() {
        char stringBuffer[NAME_BUFFER_SIZE + 1] = { 0 };
        memcpy(&stringBuffer[0], &m_Buffer[0], NAME_BUFFER_SIZE);
        return String(stringBuffer);
    }
};

struct NRICBuffer {
    byte m_Buffer[NRIC_BUFFER_SIZE] = { 0 };

    String GetString() {
        char stringBuffer[NRIC_BUFFER_SIZE + 1] = { 0 };
        memcpy(&stringBuffer[0], &m_Buffer[0], NRIC_BUFFER_SIZE);
        return String(stringBuffer);
    }
};

/* The StatusBuffer is used to store if someone has signed in, signed out, and the time for the events.
Status buffer consists of 4 bytes. Of the first 2 bytes, the leftmost bit is used as a boolean to store if the person has signed in.
The next 7 bits are used to store the time the person signed in as a 4 digit integer. So if GetTimeIn() returns the number 1640, it means that the person
signed in at 1640Hrs. We do this since we do not need the whole 8 bits to store 0000 to 2359, so we can use the left most bit as a boolean.
The next 2 bytes of the buffer is used to store the sign-out status in the same manner. */
struct StatusBuffer {
  private:
    String TimeToString(unsigned short _time) {
        if (_time < 10) {
            return String(F("000")) + String(_time) + String(F("H"));
        }
        if (_time < 100) {
            return String(F("00")) + String(_time) + String(F("H"));
        }
        if (_time < 1000) {
            return String(F("0")) + String(_time) + String(F("H"));
        }

        return String(_time) + String(F("H"));
    }
    
  public:
    byte m_Buffer[STATUS_BUFFER_SIZE] = { 0 };

    // Retuns a 4 digit short, representing Hour & Minute. Eg. 1234 means 1234Hrs. Use GetStatus to check the person's status before getting the time.
    unsigned short GetTimeIn() {
        unsigned short mask = 1;
        mask = (mask << 15);
        mask = ~mask;

        unsigned short time = 0;
        memcpy(&time, &m_Buffer[0], sizeof(time));
        time = (time & mask);

        return time;
    }

    // Retuns a 4 digit short, representing Hour & Minute. Eg. 54 means 0054Hrs. Use GetStatus to check the person's status before getting the time.
    unsigned short GetTimeOut() {
        unsigned short mask = 1;
        mask = (mask << 15);
        mask = ~mask;

        unsigned short time = 0;
        memcpy(&time, &m_Buffer[STATUS_BUFFER_SIZE / 2], sizeof(time));
        time = (time & mask);

        return time;
    }

    Status GetStatus() {
        unsigned short mask = 1;
        mask = (mask << 15);

        unsigned short inStatus = 0;
        memcpy(&inStatus, &m_Buffer[0], sizeof(inStatus));
        inStatus = (inStatus & mask);

        unsigned short outStatus = 0;
        memcpy(&outStatus, &m_Buffer[STATUS_BUFFER_SIZE / 2], sizeof(outStatus));
        outStatus = (outStatus & mask);

        if (inStatus && outStatus) { return Status::OUT; }
        if (inStatus && !outStatus) { return Status::IN; }
        return Status::ABSENT;
    }

    String GetString() {
        String string;
        switch (GetStatus()) {
        case Status::IN:
            string = String(F("In: ")) + TimeToString(GetTimeIn());
            break;
        case Status::OUT:
            string = String(F("In: ")) + TimeToString(GetTimeIn()) + String(F(" Out: ")) + TimeToString(GetTimeOut());
            break;
        default:
            string = F("Absent");
            break;
        }
        return string;
    }
};

#endif
