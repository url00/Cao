#include <string>

#define Log(message) {                  \
    __Log(message, __FILE__, __LINE__); \
}                                       \

const char Config_CONTROL = 1 << 0;
const char Config_ALT     = 1 << 1;
const char Config_SHIFT   = 1 << 2;

const char Config_STANDARD = 1 << 0;
const char Config_TEMPFILE = 1 << 1;
const char Config_ARGS     = 1 << 2;

void
__Log(std::string message, std::string file, int line);

void
CloseLog();

