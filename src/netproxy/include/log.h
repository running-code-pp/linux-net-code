#ifndef LOG_H
#define LOG_H

#include <syslog.h>
#include <cstdarg>

void set_loglevel( int log_level = LOG_DEBUG );
void log( int log_level, const char* file_name, int line_num, const char* format, ... );
#define LOG(log_level, format, ...) log(log_level, __FILE__, __LINE__, format, __VA_ARGS__);
#endif
