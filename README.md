# RFID Attendance System (RAS)
Code written by Lim Ngian Xin Terry (Software Guy) with assistance from Eswaran s/o Saravanan (Hardware Guy).

Built for the Arduino Uno.

Demo Video: https://www.youtube.com/watch?v=MoV-WNHIaI4

How to update Nominal Roll:
1. In the SD card, there is a file called NR.csv. In it contains 3 columns.
2. The 1st column contains the card's RFID numbers. Unless new cards are addd, there should not be any need to update this. Otherwise, simply type their RFID into this column.
3. The 2nd column contains the personnels' names. The names MUST BE 10 CHARACTER(INCLUDING SPACES) OR BELOW! This is due to the hardware's RAM limitations! Recommended to use the names as shown on their uniforms.
4. The 3rd column contains the last 4 digits of the personnels' NRIC.
5. ANY SPACES ARE ALSO CONSIDERED A CHARACTER!
6. DO NOT RENAME THE FILE!
7. THE DEVICE CAN ONLY SUPPORT UP TO 20 PEOPLE MAXIMUM! (Arduino Uno RAM Limitation)
8. If you are using a blank SD card, copy RAS/NR.csv, edit it and upload it to the root folder of the SD card.

How to read logs:
1. All logs are stored in the SD Card's LOGS folder.
2. The logs are stored in a text file with their date on it, in a YYYYMMDD format. For example, 20210629.txt would be the logs for the date 2021-06-29.
3. The logs will store who is absent, signed in, signed out, and their time.

How to set time (When the clock module's battery has run out and changed):
1. Connect the Arduino to your PC, and upload the TimeSet/TimeSet.ino file to it.
2. Once upload is successful, IMMEDIATELY open the Serial to run the program.
3. The time will be set to the time you compiled and uploaded the code.
4. The shorter the time between you compiling and running the program, the more accurate the time will be.
5. After updating the time, upload RAS/RAS.ino into the Arduino. You are good to go.

What is MidnightSet/MidnightSet.ino for?
1. It is what I used to set the time to almost midnight to test if the midnight reboot works without issues. You should not need it.

Important Links (If you want to change the code or something.):
This assumes that you have a fairly good knowledge in C++ or C programming. 
1. Where is this repository hosted? [https://github.com/TypeDefinition/RAS]
2. What is a SPI? [https://www.arduino.cc/en/reference/SPI]
3. What is the naming convention of files stored in the SD Card? 8.3 Filename. [https://www.arduino.cc/en/Reference/SDCardNotes]
4. What is the SD Card Format? FAT16 & FAT32. [https://www.arduino.cc/en/Reference/SDCardNotes]

Important Notes:
1. Do not power cycle or restart the device in the middle of the day!
2. The signed in and signed out status for the personnel are reseted when the device reboots.
3. The log file for the day is deleted and re-created each time someone signs in.
4. This means that if person A signs in, and the device reboots, person A will be treated as absent. And when person B signs in, the log file is re-created, and person A will be logged as absent.
5. The Arduino will automatically reboot at midnight everyday!

Additional Notes:
1. The ArduinoIDE folder contains all that you need to compile and upload the code to the Arduino.

Hardware Components:
1. Arduino Uno
2. DS3231 Real-Time-Clock Module
3. SD Card Module
4. I2C LCD Module
5. MFRC522 RFID Reader Module
6. LED Lights
