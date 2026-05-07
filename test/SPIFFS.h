#pragma once
#include <Arduino.h>

class File {
public:
    File() : _content(""), _isOpen(false), _pos(0) {}
    File(const String& content) : _content(content), _isOpen(true), _pos(0) {}
    
    operator bool() const { return _isOpen; }
    String readString() { return _content; }
    void close() { _isOpen = false; }
    
    int read() {
        if (_pos < _content.length()) {
            return _content[_pos++];
        }
        return -1;
    }

    size_t available() {
        return _isOpen ? (_content.length() - _pos) : 0;
    }

private:
    String _content;
    bool _isOpen;
    size_t _pos;
};

class SPIFFSClass {
public:
    bool exists(const char* path) { return _exists; }
    File open(const char* path, const char* mode) {
        return _exists ? File(_content) : File();
    }
    void setMockData(bool exists, const String& content = "") {
        _exists = exists; _content = content;
    }
private:
    bool _exists = false;
    String _content = "";
};

extern SPIFFSClass SPIFFS;