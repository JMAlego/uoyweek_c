/**
 * UoYWeek C
 * Originally by Luke Moll
 * Adapted from C++ to C by Jacob Allen
*/

/**
 * Deal with Windows crying about deprecation.
 */
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

/**
 * Useful time constants to avoid magic numbers.
 */
static const long SECS_PER_MIN = 60;
static const long SECS_PER_HOUR = 60 * 60;
static const long SECS_PER_DAY = 60 * 60 * 24;
static const long SECS_PER_WEEK = 60 * 60 * 24 * 7;

/**
 * Days of the week as indexed by tm struct.
 */
const char DAYS[7][10] = {
    "sunday",
    "monday",
    "tuesday",
    "wednesday",
    "thursday",
    "friday",
    "saturday"};

/**
 * Normalises a time to the monday of the containing week.
 */
time_t normalise_week_to_monday(time_t to_normalise)
{
    struct tm tm_to_normalise;
    time_t result;

    memcpy(&tm_to_normalise, gmtime(&to_normalise), sizeof(struct tm));

    result = to_normalise;
    result -= tm_to_normalise.tm_sec;
    result -= tm_to_normalise.tm_min * SECS_PER_MIN;
    result -= tm_to_normalise.tm_hour * SECS_PER_HOUR;
    result -= (tm_to_normalise.tm_wday - 1) * SECS_PER_DAY;

    return result;
}

/**
 * Term struct used to contain information about a term,
 * such as it's start and end, and it's code name.
 */
typedef struct Term
{
    time_t start_time_stamp;
    time_t end_time_stamp;
    struct tm start;
    struct tm end;
    char *code_name;
} Term;

/**
 * Creates a new term on the heap and returns a pointer to it.
 * Takes the terms code name, a three letter identifier, the
 * start time and end time as time_ts. Returns NULL on error.
 */
Term *term_new(char *code_name, time_t start_time, time_t end_time)
{
    Term *new_term;
    struct tm start;
    struct tm end;

    /* Not using strnlen/strlen_s here to get around the fact it's not always available, even in C11 */
    if (code_name[3] == 0 && strlen(code_name) != 3)
        return NULL;

    new_term = (Term *)malloc(sizeof(Term));
    if (new_term == NULL)
        return NULL;

    new_term->code_name = (char *)malloc(sizeof(char) * 4);
    if (new_term->code_name == NULL)
        return NULL;
    new_term->code_name = code_name;

    new_term->start_time_stamp = start_time;
    new_term->end_time_stamp = end_time;

    memcpy(&start, gmtime(&start_time), sizeof(struct tm));

    memcpy(&end, gmtime(&end_time), sizeof(struct tm));

    new_term->start = start;
    new_term->end = end;

    return new_term;
}

/**
 * Checks if the specified time is within the specified
 * terms start and end times. Returns 0 on error.
 */
int term_contains_time(Term *term, time_t ts)
{
    if (term == NULL)
        return 0;

    return term->start_time_stamp <= ts && ts <= term->end_time_stamp;
}

/**
 * Converts a term to a string on the heap and returns
 * a pointer to it. Mainly for debug. Returns NULL on error.
 */
char *term_to_string(Term *term)
{
    char *term_string;
    char start_string[64];
    char end_string[64];

    if (term == NULL)
        return NULL;

    term_string = (char *)malloc(sizeof(char) * 128);

    strncpy(start_string, asctime(&term->start), 64);

    strncpy(end_string, asctime(&term->end), 64);

    snprintf(term_string, 128, "%s (%d-%d) %s %s", term->code_name, ((&term->start)->tm_year + 1900), ((&term->end)->tm_year + 1900), start_string, end_string);

    return term_string;
}

/**
 * Gets the week inside a term, given the term and
 * a time inside that term. Returns 0 on error.
 */
int term_get_week(Term *term, time_t term_time)
{
    time_t start_normalised;
    time_t interval;
    int weeks;

    if (term == NULL)
        return 0;

    start_normalised = normalise_week_to_monday(term->start_time_stamp);
    interval = term_time - start_normalised;
    weeks = interval / SECS_PER_WEEK;
    return weeks + 1;
}

/** 
 * Terms struct used to contain a set of terms.
*/
typedef struct Terms
{
    Term **terms;
    size_t term_count;
} Terms;

/**
 * Creates a new set of terms on the heap and returns a 
 * pointer to it, or returns NULL on error.
 */
Terms *terms_new()
{
    Terms *terms_new;

    terms_new = malloc(sizeof(Terms));
    if (terms_new == NULL)
        return NULL;

    terms_new->term_count = 0;
    terms_new->terms = NULL;

    return terms_new;
}

/**
 * Adds a term to a set of terms, returns NULL on error.
 */
int terms_add(Terms *terms, Term *new_term)
{
    if (new_term == NULL || terms == NULL)
        return 1;

    terms->term_count += 1;

    if (terms->terms == NULL)
        terms->terms = (Term **)malloc(sizeof(Term **) * terms->term_count);
    else
        terms->terms = (Term **)realloc(terms->terms, sizeof(Term **) * terms->term_count);
    if (terms->terms == NULL)
        return 1;

    (terms->terms)[terms->term_count - 1] = new_term;

    return 0;
}

/**
 * Find the term that contains the specified time from a
 * set of terms. Returns NULL if not found, or on error.
 */
Term *terms_get_term_from_time(Terms *terms, time_t term_time)
{
    size_t term_counter;

    if (terms == NULL)
        return NULL;

    term_counter = 0;
    while (term_counter < terms->term_count)
    {
        if (term_contains_time(terms->terms[term_counter], term_time))
            return terms->terms[term_counter];
        term_counter++;
    }
    return NULL;
}

/**
 * Gets the term string for a given set of terms and
 * a time. Has a fancy mode which capitalises nicely.
 * Returns NULL if time is not in a term, or on error.
 */
char *terms_get_term_string(Terms *terms, time_t term_time, int fancy_mode, int short_mode)
{
    Term *term;
    struct tm gm_now;
    char day[10];
    char term_code[4];
    char *term_time_string;
    int week;

    if (terms == NULL)
        return NULL;

    term = terms_get_term_from_time(terms, term_time);
    if (term == NULL)
        return NULL;

    week = term_get_week(term, term_time);

    memcpy(&gm_now, gmtime(&term_time), sizeof(struct tm));

    strncpy(day, DAYS[gm_now.tm_wday], 10);

    strncpy(term_code, term->code_name, 4);

    if (fancy_mode)
    {
        term_code[0] = toupper(term_code[0]);
        day[0] = toupper(day[0]);
    }

    if(short_mode)
        day[3] = '\0';

    term_time_string = (char *)malloc(sizeof(char) * 17);
    snprintf(term_time_string, 17, "%s/%d/%s", term_code, week, day);

    return term_time_string;
}

/**
 * Term start and end times, hardcoded for now.
 */
static const time_t AUT_START = 1506297600l;
static const time_t AUT_END = 1512086400l;
static const time_t SPR_START = 1515369600l;
static const time_t SPR_END = 1521158400l;
static const time_t SUM_START = 1523836800l;
static const time_t SUM_END = 1529625600l;

/**
 * UoYWeek C is a small command line utility which outputs
 * the current term/week number/day as per the
 * University of York timetabling.
 * 
 * Originally by Luke Moll
 * Adapted from C++ to C by Jacob Allen
*/
int main(int argc, char *argv[])
{
    int arg_counter;
    int flag_fancy;
    int flag_short;
    Term *aut;
    Term *spr;
    Term *sum;
    Terms *terms;
    time_t now;
    char *term_string;

    flag_fancy = 0;
    flag_short = 0;
    if (argc > 1)
    {
        for (arg_counter = 1; arg_counter < argc; arg_counter++)
        {
            if (!flag_fancy && (strncmp(argv[arg_counter], "--fancy", 8) == 0))
            {
                flag_fancy = 1;
            }
            else if (!flag_short && (strncmp(argv[arg_counter], "--short", 8) == 0))
            {
                flag_short = 1;
            }
        }
    }

    now = time(0);
    aut = term_new("aut", AUT_START, AUT_END);
    if (aut == NULL)
    {
        fprintf(stderr, "Could not create autumn term.\n");
        return 1;
    }

    spr = term_new("spr", SPR_START, SPR_END);
    if (spr == NULL)
    {
        fprintf(stderr, "Could not create spring term.\n");
        return 2;
    }

    sum = term_new("sum", SUM_START, SUM_END);
    if (sum == NULL)
    {
        fprintf(stderr, "Could not create summer term.\n");
        return 3;
    }

    terms = terms_new();
    if (terms == NULL)
    {
        fprintf(stderr, "Could not create terms.\n");
        return 4;
    }

    if (terms_add(terms, aut))
    {
        fprintf(stderr, "Could not add autumn term to terms.\n");
        return 5;
    }
    if (terms_add(terms, spr))
    {
        fprintf(stderr, "Could not add spring term to terms.\n");
        return 6;
    }
    if (terms_add(terms, sum))
    {
        fprintf(stderr, "Could not add summer term to terms.\n");
        return 7;
    }

    term_string = terms_get_term_string(terms, now, flag_fancy, flag_short);
    if (term_string == NULL)
    {
        fprintf(stderr, "Could not get term string.\n");
        return 8;
    }
    printf("%s", term_string);

    return 0;
}
