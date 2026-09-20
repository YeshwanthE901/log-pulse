#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "parser.h"

void test_valid_log(void)
{
    const char *line =
        "2026-09-19 10:00:09 ERROR database Connection timeout\n";

    LogRecord record;

    int result = parse_log_line(line, &record);

    assert(result == 1);
    assert(strcmp(record.date, "2026-09-19") == 0);
    assert(strcmp(record.time, "10:00:09") == 0);
    assert(strcmp(record.level, "ERROR") == 0);
    assert(strcmp(record.component, "database") == 0);
    assert(strcmp(record.message, "Connection timeout") == 0);

    printf("PASS: valid log parsing\n");
}

void test_valid_info_log(void)
{
    const char *line =
        "2026-09-19 11:30:05 INFO system Application started\n";

    LogRecord record;

    int result = parse_log_line(line, &record);

    assert(result == 1);
    assert(strcmp(record.level, "INFO") == 0);
    assert(strcmp(record.component, "system") == 0);
    assert(strcmp(record.message, "Application started") == 0);

    printf("PASS: INFO log parsing\n");
}

void test_invalid_log(void)
{
    const char *line =
        "THIS IS NOT A VALID LOG RECORD\n";

    LogRecord record;

    int result = parse_log_line(line, &record);

    assert(result == 0);

    printf("PASS: invalid log rejected\n");
}

void test_missing_message(void)
{
    const char *line =
        "2026-09-19 10:00:09 ERROR database\n";

    LogRecord record;

    int result = parse_log_line(line, &record);

    assert(result == 0);

    printf("PASS: missing message rejected\n");
}

void test_long_message(void)
{
    const char *line =
        "2026-09-19 12:00:00 ERROR server "
        "Database connection timeout after retry attempt\n";

    LogRecord record;

    int result = parse_log_line(line, &record);

    assert(result == 1);
    assert(strcmp(record.level, "ERROR") == 0);
    assert(strcmp(record.component, "server") == 0);
    assert(
        strcmp(
            record.message,
            "Database connection timeout after retry attempt"
        ) == 0
    );

    printf("PASS: long message parsing\n");
}

int main(void)
{
    test_valid_log();
    test_valid_info_log();
    test_invalid_log();
    test_missing_message();
    test_long_message();

    printf("\nAll parser tests passed.\n");

    return 0;
}