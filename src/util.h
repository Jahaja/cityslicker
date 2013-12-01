#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdarg.h>

void log_error(const char *format, ...);
void log_fatal(const char *format, ...);
void log_info(const char *format, ...);

long long ustime(void);
long long mstime(void);

#endif