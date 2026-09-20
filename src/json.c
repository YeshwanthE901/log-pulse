#include <stdio.h>

#include "json.h"

static void print_json_string(const char *text)
{
    putchar('"');

    for (const unsigned char *p =
             (const unsigned char *)text;
         *p != '\0';
         p++)
    {
        switch (*p)
        {
            case '"':
                printf("\\\"");
                break;

            case '\\':
                printf("\\\\");
                break;

            case '\b':
                printf("\\b");
                break;

            case '\f':
                printf("\\f");
                break;

            case '\n':
                printf("\\n");
                break;

            case '\r':
                printf("\\r");
                break;

            case '\t':
                printf("\\t");
                break;

            default:
                if (*p < 0x20)
                {
                    printf("\\u%04x", *p);
                }
                else
                {
                    putchar(*p);
                }
                break;
        }
    }

    putchar('"');
}

void print_json_report(const AnalysisStats *stats,
                       int workers,
                       int anomaly_count)
{
    printf("{\n");

    printf("  \"workers\": %d,\n",
           workers);

    printf("  \"total_records\": %d,\n",
           stats->total_records);

    printf("  \"info\": %d,\n",
           stats->info_count);

    printf("  \"warn\": %d,\n",
           stats->warn_count);

    printf("  \"error\": %d,\n",
           stats->error_count);

    printf("  \"invalid\": %d,\n",
           stats->invalid_count);

    printf("  \"anomalies\": %d,\n",
           anomaly_count);

    printf("  \"error_patterns\": [\n");

    int first = 1;

    for (unsigned long i = 0;
         i < stats->error_table->size;
         i++)
    {
        ErrorNode *node =
            stats->error_table->buckets[i];

        while (node != NULL)
        {
            if (!first)
                printf(",\n");

            printf("    {\n");
            printf("      \"message\": ");

            print_json_string(node->message);

            printf(",\n");
            printf("      \"count\": %d\n",
                   node->count);
            printf("    }");

            first = 0;
            node = node->next;
        }
    }

    printf("\n  ]\n");
    printf("}\n");
}
