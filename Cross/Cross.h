namespace cao
{
    const char Config_CONTROL = 1 << 0;
    const char Config_ALT     = 1 << 1;
    const char Config_SHIFT   = 1 << 2;

    const char Config_STANDARD = 1 << 0;
    const char Config_TEMPFILE = 1 << 1;
    const char Config_ARGS     = 1 << 2;

    #ifdef Cao_Windows
    const char *test = "Cao on Windows!";
    #endif

    #ifdef Cao_Linux
    const char *test = "Cao on Linux!";
    #endif

    void
    blah();
}

