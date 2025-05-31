#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <Arduino.h>
#include "register.h"
#include "../lib_display.h"

class Instruction
{
public:
    virtual void execute(Register &reg) = 0;
    virtual ~Instruction() = default;

    virtual const char *name();
    static const char *NAME;
};

class ConsolePrintlnInstruction : public Instruction
{
private:
    String message;

public:
    ConsolePrintlnInstruction(const String &msg) : message(msg) {}

    void execute(Register &reg) override
    {
        Serial.println(message);
    }
    static constexpr const char *NAME = "console_println";
    const char *name() override
    {
        return NAME;
    }
};

class DisplayPrintlnInstruction : public Instruction
{
private:
    String message;

public:
    DisplayPrintlnInstruction(const String &msg) : message(msg) {}

    void execute(Register &reg) override
    {
        tft.println(message);
    }

    static constexpr const char *NAME = "display_println";
    const char *name() override
    {
        return NAME;
    }
};

class DisplayBrightnessInstruction : public Instruction
{
private:
    uint8_t brightness;

public:
    DisplayBrightnessInstruction(const String &brightnessStr) : brightness(brightnessStr.toInt()) {}

    void execute(Register &reg) override
    {
        display_brightness_set(brightness);
    }

    static constexpr const char *NAME = "display_brightness";
    const char *name() override
    {
        return NAME;
    }
};

class DisplayTextHexColorInstruction : public Instruction
{
private:
    u_int16_t color; // 16bit 565 rgb color
public:
    DisplayTextHexColorInstruction(const String &colorStr) : color(strtoul(colorStr.c_str(), nullptr, 16)) {}

    void execute(Register &reg) override
    {
        tft.setTextColor(color);
    }

    static constexpr const char *NAME = "display_text_hexcolor";
    const char *name() override
    {
        return NAME;
    }
};

class DisplayTextColorInstruction : public Instruction
{
private:
    uint16_t color; // 16bit 565 rgb color
public:
    DisplayTextColorInstruction(const String &colorStr)
    {
        if (colorStr.equals("red"))
        {
            color = ST77XX_RED;
        }
        else if (colorStr.equals("green"))
        {
            color = ST77XX_GREEN;
        }
        else if (colorStr.equals("blue"))
        {
            color = ST77XX_BLUE;
        }
        else if (colorStr.equals("white"))
        {
            color = ST77XX_WHITE;
        }
        else if (colorStr.equals("black"))
        {
            color = ST77XX_BLACK;
        }
        else
        {
            color = ST77XX_WHITE; // default to white
        }
    }

    void execute(Register &reg) override
    {
        tft.setTextColor(color);
    }

    static constexpr const char *NAME = "display_text_color";
    const char *name() override
    {
        return NAME;
    }
};

class DisplayTextSizeInstruction : public Instruction
{
private:
    uint8_t size;

public:
    DisplayTextSizeInstruction(const String &sizeStr) : size(sizeStr.toInt()) {}

    void execute(Register &reg) override
    {
        tft.setTextSize(size);
    }

    static constexpr const char *NAME = "display_text_size";
    const char *name() override
    {
        return NAME;
    }
};

class DisplayFillScreenInstruction : public Instruction
{
private:
    uint16_t color; // 16bit 565 rgb color
public:
    DisplayFillScreenInstruction(const String &colorStr)
    {
        if (colorStr.equals("red"))
        {
            color = ST77XX_RED;
        }
        else if (colorStr.equals("green"))
        {
            color = ST77XX_GREEN;
        }
        else if (colorStr.equals("blue"))
        {
            color = ST77XX_BLUE;
        }
        else if (colorStr.equals("white"))
        {
            color = ST77XX_WHITE;
        }
        else if (colorStr.equals("black"))
        {
            color = ST77XX_BLACK;
        }
        else
        {
            color = ST77XX_BLACK; // default to black
        }
    }

    void execute(Register &reg) override
    {
        tft.fillScreen(color);
    }

    static constexpr const char *NAME = "display_fill_screen";
    const char *name() override
    {
        return NAME;
    }
};

class DisplayCursorInstruction : public Instruction
{
private:
    int16_t x, y;

public:
    DisplayCursorInstruction(const String &coords)
    {
        x = -1;
        y = -1;

        int colonIndex = coords.indexOf(',');
        if (colonIndex != -1)
        {
            if (coords.substring(0, colonIndex).toInt() > 0)
            {
                x = coords.substring(0, colonIndex).toInt();
            }
            if (coords.substring(colonIndex + 1).toInt() > 0)
            {
                y = coords.substring(colonIndex + 1).toInt();
            }
        }
    }

    void execute(Register &reg) override
    {
        if (x > 0)
        {
            tft.setCursor(x, tft.getCursorY());
        }
        if (y > 0)
        {
            tft.setCursor(tft.getCursorX(), y);
        }
    }

    static constexpr const char *NAME = "display_cursor";
    const char *name() override
    {
        return NAME;
    }
};

class DelayInstruction : public Instruction
{
private:
    unsigned long delayTime;

public:
    DelayInstruction(String time) : delayTime(time.toInt()) {}

    void execute(Register &reg) override
    {
        delay_display(delayTime);
    }

    static constexpr const char *NAME = "delay";
    const char *name() override
    {
        return NAME;
    }
};

class WriteRegisterInstruction : public Instruction
{
private:
    String value;

public:
    WriteRegisterInstruction(const String &val) : value(val) {}

    void execute(Register &reg) override
    {
        reg.set(value);
    }

    static constexpr const char *NAME = "write_register";
    const char *name() override
    {
        return NAME;
    }
};

class WriteFileInstruction : public Instruction
{
private:
    String filePath;

public:
    WriteFileInstruction(const String &path) : filePath(path) {}
    void execute(Register &reg) override
    {
        writeFile(SPIFFS, filePath.c_str(), reg.get().c_str());
    }

    static constexpr const char *NAME = "write_file";
    const char *name() override
    {
        return NAME;
    }
};

Instruction *instructionFromString(const String &instructionStr)
{
    if (instructionStr.indexOf(':') == -1)
    {
        Serial.printf(F("Invalid instruction format: %s\n"), instructionStr.c_str());
        return nullptr;
    }

    String commandStr = instructionStr.substring(0, instructionStr.indexOf(':'));
    commandStr.trim();
    const char *command = commandStr.c_str();

    String value = instructionStr.substring(instructionStr.indexOf(':') + 1);
    value.trim();
    Serial.printf("Parsed command: %s, value: %s\n", command, value.c_str());

    if (strcmp(command, ConsolePrintlnInstruction::NAME) == 0)
    {
        return new ConsolePrintlnInstruction(value);
    }
    else if (strcmp(command, DisplayPrintlnInstruction::NAME) == 0)
    {
        return new DisplayPrintlnInstruction(value);
    }
    else if (strcmp(command, DisplayBrightnessInstruction::NAME) == 0)
    {
        return new DisplayBrightnessInstruction(value);
    }
    else if (strcmp(command, DisplayTextHexColorInstruction::NAME) == 0)
    {
        return new DisplayTextHexColorInstruction(value);
    }
    else if (strcmp(command, DisplayTextColorInstruction::NAME) == 0)
    {
        return new DisplayTextColorInstruction(value);
    }
    else if (strcmp(command, DisplayTextSizeInstruction::NAME) == 0)
    {
        return new DisplayTextSizeInstruction(value);
    }
    else if (strcmp(command, DisplayFillScreenInstruction::NAME) == 0)
    {
        return new DisplayFillScreenInstruction(value);
    }
    else if (strcmp(command, DisplayCursorInstruction::NAME) == 0)
    {
        return new DisplayCursorInstruction(value);
    }
    else if (strcmp(command, DelayInstruction::NAME) == 0)
    {
        return new DelayInstruction(value);
    }
    else if (strcmp(command, WriteRegisterInstruction::NAME) == 0)
    {
        return new WriteRegisterInstruction(value);
    }
    else if (strcmp(command, WriteFileInstruction::NAME) == 0)
    {
        return new WriteFileInstruction(value);
    }

    Serial.println(F("Unable to parse, instructions available:"));
    Serial.println(ConsolePrintlnInstruction::NAME);
    Serial.println(DisplayPrintlnInstruction::NAME);
    Serial.println(DisplayBrightnessInstruction::NAME);
    Serial.println(DisplayTextHexColorInstruction::NAME);
    Serial.println(DisplayTextColorInstruction::NAME);
    Serial.println(DisplayTextSizeInstruction::NAME);
    Serial.println(DisplayFillScreenInstruction::NAME);
    Serial.println(DisplayCursorInstruction::NAME);
    Serial.println(DelayInstruction::NAME);
    Serial.println(WriteRegisterInstruction::NAME);
    Serial.println(WriteFileInstruction::NAME);
    return nullptr;
}

#endif