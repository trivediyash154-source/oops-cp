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

    string getModeName() override
    {
        return "Timed";
    }

    // Overrides the base class: the test ends when the time limit is reached
    bool isTimeUp(float timeSeconds) override
    {
        return timeSeconds >= timeLimit;
    }

    int getTimeLimit()
    {
        return timeLimit;
    }
};

#endif