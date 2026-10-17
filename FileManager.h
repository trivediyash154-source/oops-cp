#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
using namespace std;

class FileManager
{
public:
    void saveResult(string mode, float wpm, float accuracy, int correct, int wrong)
    {
        ofstream file("user_stats.csv", ios::app);

        if(file)
        {
            file << mode << ","
                 << wpm << ","
                 << accuracy << ","
                 << correct << ","
                 << wrong << endl;

            file.close();
        }
    }

    // Reads user_stats.csv and returns the best WPM saved for a mode
    float getBestWPM(string mode)
    {
        ifstream file("user_stats.csv");
        float best = 0;
        string line;

        while(getline(file, line))
        {
            stringstream row(line);
            string rowMode, wpmText;

            getline(row, rowMode, ',');
            getline(row, wpmText, ',');

            if(rowMode != mode)
                continue;

            try
            {
                float wpm = stof(wpmText);

                if(wpm > best)
                    best = wpm;
            }
            catch(exception &e)
            {
                // skip a broken row instead of crashing
            }
        }

        return best;
    }
};

#endif