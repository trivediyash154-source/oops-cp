#ifndef TIMEDCHALLENGE_H
#define TIMEDCHALLENGE_H

#include "TypingSession.h"

class TimedChallenge : public TypingSession
{
private:
    int timeLimit;

public:
    TimedChallenge(string text, int limit = 60) : TypingSession(text)
    {
        timeLimit = limit;
    }

    void startSession() override
    {
        userInput = "";
    }

    int getTimeLimit()
    {
        return timeLimit;
    }
};

#endif