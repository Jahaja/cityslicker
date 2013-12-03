/*
* cityslicker by Joakim Hamren
* 
* To the extent possible under law, the person who associated CC0 with
* cityslicker has waived all copyright and related or neighboring rights
* to cityslicker.
*
* You should have received a copy of the CC0 legalcode along with this
* work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdarg.h>

void log_error(const char *format, ...);
void log_fatal(const char *format, ...);
void log_info(const char *format, ...);

long long ustime(void);
long long mstime(void);

#endif