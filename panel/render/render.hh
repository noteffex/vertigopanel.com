#pragma once
#include <windows.h>

class crender {
public:
    void init();
    void exit();
public:
    void loop();
private:
    WNDCLASSEX wc;
};

inline crender render;