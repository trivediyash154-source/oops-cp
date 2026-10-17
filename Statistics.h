#ifndef STATISTICS_H
#define STATISTICS_H

#include <string>
using namespace std;

class Statistics
{
private:
    float wpm;
    float accuracy;
    int correct;
    int wrong;

public:
    Statistics()
    {
        wpm = 0;
        accuracy = 0;
        correct = 0;
        wrong = 0;
    }

    void calculate(string target, string typed, float timeSeconds)
    {
        correct = 0;
        wrong = 0;

        for(int i = 0; i < typed.length(); i++)
        {
            if(i < target.length() && typed[i] == target[i])
                correct++;
            else
                wrong++;
        }

        int totalCharacters = typed.length();

        if(totalCharacters > 0)
            accuracy = (float)correct / totalCharacters * 100;
        else
            accuracy = 0;

        float minutes = timeSeconds / 60.0;

        if(minutes > 0)
            wpm = (typed.length() / 5.0) / minutes;
        else
            wpm = 0;
    }

    float getWPM()
    {
        return wpm;
    }

    float getAccuracy()
    {
        return accuracy;
    }

    int getCorrect()
    {
        return correct;
    }

    int getWrong()
    {
        return wrong;
    }
};

#endif