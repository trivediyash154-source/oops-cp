#ifndef TYPINGSESSION_H
#define TYPINGSESSION_H

#include <string>
using namespace std;

class TypingSession
{
protected:
    string targetText;
    string userInput;
    int correctCharacters;
    int wrongCharacters;

public:
    TypingSession(string text)
    {
        targetText = text;
        userInput = "";
        correctCharacters = 0;
        wrongCharacters = 0;
    }

    virtual void startSession() = 0;

    // Each mode returns its own name (used on screen and in the CSV file)
    virtual string getModeName() = 0;

    // Practice has no time limit, so by default time is never up
    virtual bool isTimeUp(float timeSeconds)
    {
        return false;
    }

    virtual void calculateResult(float timeSeconds)
    {
        correctCharacters = 0;
        wrongCharacters = 0;

        int total = userInput.length();

        for(int i = 0; i < total; i++)
        {
            if(i < targetText.length() && userInput[i] == targetText[i])
                correctCharacters++;
            else
                wrongCharacters++;
        }
    }

    string getTargetText()
    {
        return targetText;
    }

    string getUserInput()
    {
        return userInput;
    }

    int getCorrect()
    {
        return correctCharacters;
    }

    int getWrong()
    {
        return wrongCharacters;
    }

    void setUserInput(string input)
    {
        userInput = input;
    }

    virtual ~TypingSession() {}
};

#endif