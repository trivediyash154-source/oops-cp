#ifndef PASSAGEMANAGER_H
#define PASSAGEMANAGER_H

#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
using namespace std;

class PassageManager
{
private:
    vector<string> passages;

public:
    void loadPassages(string filename)
    {
        ifstream file(filename);

        if(!file)
            throw runtime_error("Error: prompts.txt file not found!");

        string line;

        while(getline(file, line))
        {
            if(!line.empty())
                passages.push_back(line);
        }

        file.close();

        if(passages.empty())
            throw runtime_error("Error: No passages found!");
    }

    string getPassage(int index)
    {
        if(index < 0 || index >= passages.size())
            throw runtime_error("Invalid passage number!");

        return passages[index];
    }

    int getCount()
    {
        return passages.size();
    }
};

#endif