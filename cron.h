#ifndef CONDUIT_CRON_H
#define CONDUIT_CRON_H

struct CronTime {
    int minute;
    int hour;
    int day_of_month;
    int month;
    int day_of_week;
};

int match_cron_field(const char *field, int value, int min, int max);

#endif
