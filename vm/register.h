#ifndef REGISTER_H
#define REGISTER_H

#include <Arduino.h>

class Register
{
protected:
    String data;

public:
    Register() = default;

    void set(const String &value)
    {
        data = value;
    }
    String get() const
    {
        return data;
    }
};

#endif
