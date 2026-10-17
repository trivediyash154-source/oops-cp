#ifndef PRACTICEMODE_H
#define PRACTICEMODE_H

#include "TypingSession.h"

class PracticeMode : public TypingSession
{
public:
    PracticeMode(string text) : TypingSession(text)
    {
    }

    void startSession() override
    {
        userInput = "";
    }
};

#endif