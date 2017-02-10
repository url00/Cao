

#include <vector>
#include <bitset>

namespace cao
{
    const char *TITLE = "Cao";

    const char Config_CONTROL = 1 << 0;
    const char Config_ALT     = 1 << 1;
    const char Config_SHIFT   = 1 << 2;

    const char Config_STANDARD = 1 << 0;
    const char Config_TEMPFILE = 1 << 1;
    const char Config_ARGS     = 1 << 2;

    class ConfigLine
    {
    public:
        std::string name;
        std::string command;
        std::string hotkey;
        std::bitset<3> inputModes;
        std::bitset<3> hotkeyMods;

        ConfigLine(std::string rawLine);
        ~ConfigLine();
    };

    class Config
    {
    private:
        std::vector<ConfigLine> lines;

    public:
        Config(std::string configFile);
        ~Config();
    };

    class Cao
    {
    private:
        bool isChildRunning;
        Config currentConfig;        
    public:
        Cao();
        ~Cao();

        void LoadConfigFile();
        void Cancel();
        void Run(Config *config);
    };

    #ifdef Cao_Windows
    const char *test = "Cao on Windows!";
    #endif

    #ifdef Cao_Linux
    const char *test = "Cao on Linux!";
    #endif
}

