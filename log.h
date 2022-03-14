#pragma once

#include <iostream>
#include <fstream>
#include <string>

class Log
{
private:
    std::ofstream fout;
    char fileName[128];

public:
    Log();
    bool openLog();
    void writeLog(std::string str);
    ~Log();
};
