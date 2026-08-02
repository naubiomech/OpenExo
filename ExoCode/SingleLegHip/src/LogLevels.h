#ifndef LOG_LEVELS_H
#define LOG_LEVELS_H

enum class LogLevel:int
{
    Release = 0,
    Error = 1,
    Warn = 2,
    Debug = 3,
    Disable = 4
};

#endif