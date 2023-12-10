/*
 * Error handling routines
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "error.h"


int errors = 0;
int warnings = 0;
int dbflag = 1;

void fatal(char * fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);
        fprintf(stderr, "\nFatal Error: ");
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        fprintf(stderr, "\n");
        exit(EXIT_FAILURE);
}

void error(char * fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);
        fprintf(stderr, "\nErrors: ");
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        fprintf(stderr, "\n");
        errors++; 
}


void warning(char * fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);
        fprintf(stderr, "\nWarning: ");
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        fprintf(stderr, "\n");
        warnings++;
}


void debug(char * fmt, ...)
{
        if(!dbflag) return;

        va_list ap;
        va_start(ap, fmt);
        fprintf(stderr, "\nDebug: ");
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        fprintf(stderr, "\n");
}


int geterrors()
{
        return errors;
}

int getwarning()
{
        return warnings;
}
