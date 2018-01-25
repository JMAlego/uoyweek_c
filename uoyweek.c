#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

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
    memcpy(&tm_to_normalise, gmtime(&to_normalise), sizeof(struct tm));

    time_t result = to_normalise;
    result -= tm_to_normalise.tm_sec;
    result -= tm_to_normalise.tm_min * 60;
    result -= tm_to_normalise.tm_hour * 3600;
    result -= (tm_to_normalise.tm_wday - 1) * 24 * 3600;

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
    // Not using strnlen/strlen_s here to get around the fact it's not always available, even in C11
    if (code_name[3] == 0 && strlen(code_name) != 3)
        return NULL;

    Term *new_term = (Term *)malloc(sizeof(Term));
    if (new_term == NULL)
        return NULL; 

    new_term->code_name = (char *)malloc(sizeof(char) * 4);
    if (new_term->code_name == NULL)
        return NULL;
    new_term->code_name = code_name;

    new_term->start_time_stamp = start_time;
    new_term->end_time_stamp = end_time;

    struct tm start;
    memcpy(&start, gmtime(&start_time), sizeof(struct tm));

    struct tm end;
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
    if(term == NULL)
        return 0;

    return term->start_time_stamp <= ts && ts <= term->end_time_stamp;
}

/**
 * Converts a term to a string on the heap and returns
 * a pointer to it. Mainly for debug. Returns NULL on error.
 */
char *term_to_string(Term *term)
{
    if(term == NULL)
        return NULL;

    char *term_string = (char *)malloc(sizeof(char) * 128);

    char start_string[64];
    strncpy(start_string, asctime(&term->start), 64);

    char end_string[64];
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
    if(term == NULL)
        return 0;

    time_t start_normalised = normalise_week_to_monday(term->start_time_stamp);
    time_t interval = term_time - start_normalised;
    int weeks = interval / 604800ll;
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
    Terms *terms_new = malloc(sizeof(Terms));
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
    if(new_term == NULL || terms == NULL)
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
    if(terms == NULL)
        return NULL;

    int term_counter = 0;
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
char *terms_get_term_string(Terms *terms, time_t term_time, int fancy_mode)
{
    if(terms == NULL)
        return NULL;

    Term *term = terms_get_term_from_time(terms, term_time);
    if (term == NULL)
        return NULL;

    int week = term_get_week(term, term_time);

    struct tm gm_now;
    memcpy(&gm_now, gmtime(&term_time), sizeof(struct tm));

    char day[10];
    strncpy(day, DAYS[gm_now.tm_wday], 10);

    char term_code[4];
    strncpy(term_code, term->code_name, 4);

    if(fancy_mode){
        term_code[0] = toupper(term_code[0]);
        day[0] = toupper(day[0]);
    }

    char *term_time_string = (char *)malloc(sizeof(char) * 17);
    snprintf(term_time_string, 17, "%s/%d/%s", term_code, week, day);

    return term_time_string;
}

/**
 * UoYWeek is a small command line utility which outputs
 * the current term/week number/day as per the
 * University of York timetabling.
 * 
 * Originally by Luke Moll
 * Adapted to from C++ to C by Jacob Allen
*/
int main(int argc, char *argv[])
{
    int fancy = argc == 2 && strncmp(argv[1], "--fancy", 8) == 0;

    time_t now = time(0);
    Term *aut = term_new("aut", (time_t)1506297600l, (time_t)1512086400l);
    if (aut == NULL)
    {
        fprintf(stderr, "Could not create autumn term.\n");
        return 1;
    }

    Term *spr = term_new("spr", (time_t)1515369600l, (time_t)1521158400l);
    if (spr == NULL)
    {
        fprintf(stderr, "Could not create spring term.\n");
        return 2;
    }

    Terms *terms = terms_new();
    if (terms == NULL)
    {
        fprintf(stderr, "Could not create terms.\n");
        return 3;
    }

    if (terms_add(terms, aut))
    {
        fprintf(stderr, "Could not add autumn term to terms.\n");
        return 4;
    }
    if (terms_add(terms, spr))
    {
        fprintf(stderr, "Could not add spring term to terms.\n");
        return 5;
    }

    char *term_string = terms_get_term_string(terms, now, fancy);
    if (term_string == NULL)
    {
        fprintf(stderr, "Could get term string.\n");
        return 6;
    }
    printf("%s", term_string);

    return 0;
}
