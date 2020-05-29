#ifndef RAS_FILE_UTIL_H
#define RAS_FILE_UTIL_H

// Include Arduino Libraries
#include <SD.h>

bool readLine(File& _file, char* _buffer, unsigned int _bufferSize);

#endif