#include "Cross.h"
#include <fstream>



#ifdef Cao_Windows
static const char *_logFile_filename = "log.txt";
#elif Cao_Linux
// @todo prob a better place for log files on linux
static const char *_logFile_filename = "log";
#endif

static std::ofstream _logFile(_logFile_filename, std::ofstream::app);



void
__Log(std::string message, std::string file, int line)
{
#ifdef Cao_Windows
    // @todo log to window too
    // prob just log to some string vector? hmm
#endif

    
    _logFile << file << ":" << line << "    " << message << std::endl;
}

void
CloseLog()
{
    _logFile.close();
}