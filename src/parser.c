#include <stdio.h>
#include <string.h>

#include "parser.h"

int time_to_minute(const char *time)
{
    int hour;
    int minute;
    int second;

    if (sscanf(time, "%d:%d:%d", &hour, &minute, &second) != 3)
    {
        return -1;
    }

    return hour * 60 + minute;
}

int parse_log_line(const char *line, LogRecord *record)
{
    const char *cursor = line;
    const char *end;
    size_t length;

    /* Date */
    end = strchr(cursor, ' ');

    if (end == NULL)
    {
        return 0;
    }

    length = (size_t)(end - cursor);

    if (length >= sizeof(record->date))
    {
        return 0;
    }

    memcpy(record->date, cursor, length);
    record->date[length] = '\0';

    cursor = end + 1;

    /* Time */
    end = strchr(cursor, ' ');

    if (end == NULL)
    {
        return 0;
    }

    length = (size_t)(end - cursor);

    if (length >= sizeof(record->time))
    {
        return 0;
    }

    memcpy(record->time, cursor, length);
    record->time[length] = '\0';

    cursor = end + 1;

    /* Level */
    end = strchr(cursor, ' ');

    if (end == NULL)
    {
        return 0;
    }

    length = (size_t)(end - cursor);

    if (length >= sizeof(record->level))
    {
        return 0;
    }

    memcpy(record->level, cursor, length);
    record->level[length] = '\0';

    cursor = end + 1;

    /* Component */
    end = strchr(cursor, ' ');

    if (end == NULL)
    {
        return 0;
    }

    length = (size_t)(end - cursor);

    if (length >= sizeof(record->component))
    {
        return 0;
    }

    memcpy(record->component, cursor, length);
    record->component[length] = '\0';

    cursor = end + 1;

    /* Message */
    length = strcspn(cursor, "\r\n");

    if (length >= sizeof(record->message))
    {
        return 0;
    }

    memcpy(record->message, cursor, length);
    record->message[length] = '\0';

    return 1;
}