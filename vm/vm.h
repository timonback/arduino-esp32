#ifndef VM_H_
#define VM_H_

#include <Arduino.h>
#include "register.h"
#include "ringbuffer.h"
#include "instruction.h"

class VM
{
protected:
    Register reg;
    RingBuffer<Instruction> instructions;

public:
    VM()
    {
        init();
    };

    void queue(std::unique_ptr<Instruction> instruction)
    {
        if (instruction)
        {
            instructions.push(std::move(instruction));
        }
        else
        {
            Serial.println(F("Received null instruction"));
        }
    }

    void run()
    {
        Serial.println(F("Running VM..."));
        while (!instructions.isEmpty())
        {
            std::unique_ptr<Instruction> instruction = instructions.pop();
            if (instruction)
            {
                Serial.printf(F("Executing instruction: %s\n"), instruction->name());
                instruction->execute(reg);
            }
        }
        Serial.println(F("VM run completed"));
    }

private:
    void init()
    {
        instructions.push(std::make_unique<ConsolePrintlnInstruction>("VM initialized"));
        instructions.push(std::make_unique<DisplayPrintlnInstruction>("VM initialized"));
        instructions.push(std::make_unique<DelayInstruction>("1000"));
    }
};

#endif
