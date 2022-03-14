#include "log.h"

Log::Log()
{
    time_t now = time(NULL);
    struct tm *pnow = localtime(&now);
    sprintf(fileName, "./log/%02d-%02d-%02d-%02d-%02d.txt", pnow->tm_mon + 1, pnow->tm_mday, pnow->tm_hour, pnow->tm_min, pnow->tm_sec);
}

bool Log::openLog()
{
    fout.open(fileName);
    if (fout.fail())
    {
        std::cout << "fout.open() failed." << std::endl;
        return false;
    }
    return true;
}

void Log::writeLog(std::string str)
{
    fout << str << std::endl;
}

Log::~Log()
{
    fout.close();
}
