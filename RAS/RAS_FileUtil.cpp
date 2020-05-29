#include "RAS_FileUtil.h"

bool readLine(File& _file, char* _buffer, unsigned int _bufferSize) {
    memset(_buffer, 0, _bufferSize);
    unsigned int bufferIndex = 0;
    while (true) {
        char value = _file.read();

        // Returns false if empty line. Returns true of non-empty line.
        if (value == -1) { return (bufferIndex != 0); }

        // End of line.
        if (value == '\n') { return true; }

        // Even if the buffer size is insufficient, this function will load whatever it can into the buffer, and clear the rest of the line.
        if (bufferIndex < _bufferSize) {
            _buffer[bufferIndex++] = value;
        }
    }
}