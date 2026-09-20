#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <time.h>

void sleep_one_second(void)
{
    struct timespec delay;

    delay.tv_sec = 1;
    delay.tv_nsec = 0;

    nanosleep(&delay, NULL);
}

int main(void)
{
    printf("2026-09-20 10:00:01 INFO system Application started\n");
    fflush(stdout);
    sleep_one_second();

    printf("2026-09-20 10:00:02 INFO auth User login successful\n");
    fflush(stdout);
    sleep_one_second();

    printf("2026-09-20 10:00:03 INFO database Connection established\n");
    fflush(stdout);
    sleep_one_second();

    printf("2026-09-20 10:00:04 WARN database Query response slow\n");
    fflush(stdout);
    sleep_one_second();

    printf("2026-09-20 10:00:05 ERROR database Connection timeout\n");
    fflush(stdout);
    sleep_one_second();

    printf("2026-09-20 10:00:06 INFO model Model loaded\n");
    fflush(stdout);
    sleep_one_second();

    printf("2026-09-20 10:00:07 ERROR model Model loading failed\n");
    fflush(stdout);
    sleep_one_second();

    printf("2026-09-20 10:00:08 INFO system Worker restarted\n");
    fflush(stdout);
    sleep_one_second();

    printf("2026-09-20 10:00:09 ERROR database Connection timeout\n");
    fflush(stdout);
    sleep_one_second();

    printf("2026-09-20 10:00:10 INFO auth Request completed\n");
    fflush(stdout);

    return 0;
}