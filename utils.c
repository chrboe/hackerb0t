#include "utils.h"

int numberwidth(int num)
{
    if(num >= 1000000000) return 10;
    if(num >= 100000000) return 9;
    if(num >= 10000000) return 8;
    if(num >= 1000000) return 7;
    if(num >= 100000) return 6;
    if(num >= 10000) return 5;
    if(num >= 1000) return 4;
    if(num >= 100) return 3;
    if(num >= 10) return 2;
    return 1;
}

char* datetime_difference(struct tm* first, struct tm* second)
{
    int diff_years = abs(first->tm_year - second->tm_year);
    int diff_months = abs(first->tm_mon - second->tm_mon);
    int diff_days = abs(first->tm_mday - second->tm_mday);
    int diff_hours = abs(first->tm_hour - second->tm_hour);
    int diff_minutes = abs(first->tm_min - second->tm_min);
    int diff_seconds = abs(first->tm_sec - second->tm_sec);

    int len = 0;
    int num_diffs = 0;

    int len_years = 0, len_months = 0, len_days = 0, len_hours = 0, len_minutes = 0, len_seconds = 0;

    if(diff_years)
    {
        len_years = numberwidth(diff_years) + 5; // "x year(s)"
        if(diff_years > 1)
            len_years++;
        num_diffs++;
    }
    if(diff_months)
    {
        len_months = numberwidth(diff_months) + 6; // "x month(s)"
        if(diff_months > 1)
            len_months++;
        num_diffs++;
    }
    if(diff_days)
    {
        len_days = numberwidth(diff_days) + 4; // "x day(s)"
        if(diff_days > 1)
            len_days++;
        num_diffs++;
    }
    if(diff_hours)
    {
        len_hours = numberwidth(diff_hours) + 5; // "x hour(s)"
        if(diff_hours > 1)
            len_hours++;
        num_diffs++;
    }
    if(diff_minutes)
    {
        len_minutes = numberwidth(diff_minutes) + 7; // "x minute(s)"
        if(diff_minutes > 1)
            len_minutes++;
        num_diffs++;
    }
    if(diff_seconds)
    {
        len_seconds = numberwidth(diff_seconds) + 7; // "x second(s)"
        if(diff_seconds > 1)
            len_seconds++;
        num_diffs++;
    }

    /* has been up for x years, x months, x days, x hours, x minutes and x seconds */

    len = len_years + len_months + len_days + len_hours + len_minutes + len_seconds;

    if(num_diffs == 0) /* just say 0 seconds */
    {
        len += 1 + 8; // "0 seconds"
        num_diffs++;
    }
    else if(num_diffs == 1)
    {
        /* don't do anything, buf still include it in if
        so it doesn't fall through */
    }
    else if(num_diffs == 2)
    {
        len += 5;
    }
    else
    {
        /* add length for the commas between diffs */
        len += ((num_diffs-2) * 2) + 5;
        /* -2 because the last 2 don't have a comma */
        /* *2 because we need a comma and a space */
        /* +5 because " and " */
    }

    int diffs_inserted = 0;
    char* result = calloc(len+1, sizeof(char));
    if(diff_years)
    {
        char temp[len_years];
        sprintf(temp, "%d year%s", diff_years, diff_years > 1 ? "s" : "");
        strcat(result, temp);

        if(num_diffs == 2)
            strcat(result, " and ");
        else if(num_diffs > 2)
            strcat(result, ", ");

        diffs_inserted++;
    }
    if(diff_months)
    {
        char temp[len_months];
        sprintf(temp, "%d month%s", diff_months, diff_months > 1 ? "s" : "");
        strcat(result, temp);

        if(num_diffs - diffs_inserted == 2)
            strcat(result, " and ");
        else if(num_diffs - diffs_inserted != 1)
            strcat(result, ", ");

        diffs_inserted++;
    }
    if(diff_days)
    {
        char temp[len_days];
        sprintf(temp, "%d day%s", diff_days, diff_days > 1 ? "s" : "");
        strcat(result, temp);

        if(num_diffs - diffs_inserted == 2)
            strcat(result, " and ");
        else if(num_diffs - diffs_inserted != 1)
            strcat(result, ", ");

        diffs_inserted++;
    }
    if(diff_hours)
    {
        char temp[len_hours];
        sprintf(temp, "%d hour%s", diff_hours, diff_hours > 1 ? "s" : "");
        strcat(result, temp);

        if(num_diffs - diffs_inserted == 2)
            strcat(result, " and ");
        else if(num_diffs - diffs_inserted != 1)
            strcat(result, ", ");

        diffs_inserted++;
    }
    if(diff_minutes)
    {
        char temp[len_minutes];
        sprintf(temp, "%d minute%s", diff_minutes, diff_minutes > 1 ? "s" : "");
        strcat(result, temp);

        if(num_diffs - diffs_inserted == 2)
            strcat(result, " and ");
        else if(num_diffs - diffs_inserted != 1)
            strcat(result, ", ");

        diffs_inserted++;
    }
    if(diff_seconds)
    {
        char temp[len_seconds];
        sprintf(temp, "%d second%s", diff_seconds, diff_seconds > 1 ? "s" : "");
        strcat(result, temp);

        if(num_diffs - diffs_inserted == 2)
            strcat(result, " and ");
        else if(num_diffs - diffs_inserted != 1)
            strcat(result, ", ");

        diffs_inserted++;
    }


    return result;

}