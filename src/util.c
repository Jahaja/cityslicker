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

#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define LOG_BUF_SIZE 1024
#define DATETIME_BUF_SIZE 50

static const char *get_current_datetime(void) {
    static char buf[DATETIME_BUF_SIZE];
    time_t t = time(NULL);
    const struct tm *loctime = (const struct tm *) localtime(&t);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", loctime);
    return buf;
}

static void output_log_msg(const char *prefix, FILE *fp, const char *format, va_list ap) {
    char buf[LOG_BUF_SIZE], logmsg[LOG_BUF_SIZE], dt[LOG_BUF_SIZE];

    snprintf(dt, LOG_BUF_SIZE, "[%s]", get_current_datetime());
    vsnprintf(logmsg, LOG_BUF_SIZE, format, ap);
    snprintf(buf, LOG_BUF_SIZE, "%s %s %s", dt, prefix, logmsg);
    
    fflush(stdout);
    fflush(stderr);
    fputs(buf, fp);
    fflush(fp);
}

void log_info(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    output_log_msg("INFO", stdout, format, ap);
    va_end(ap);
}

void log_error(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    output_log_msg("ERROR", stderr, format, ap);
    va_end(ap);
}

void log_fatal(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    output_log_msg("FATAL", stderr, format, ap);
    va_end(ap);

    exit(EXIT_FAILURE);
}

long long ustime(void) {
    struct timeval tv;
    long long ust;

    gettimeofday(&tv, NULL);
    ust = ((long long) tv.tv_sec) * 1000000;
    ust += tv.tv_usec;
    return ust;
}

long long mstime(void) {
    return ustime() / 1000;
}