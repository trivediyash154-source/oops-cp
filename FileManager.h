#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <fstream>
#include <string>
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
};

#endif