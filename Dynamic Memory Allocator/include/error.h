/*
 * Functions For Error Handling
 */

void fatal(char *fmt, ...);
void error(char *fmt, ...);
void warning(char *fmt, ...);
int geterrors();
int getwarning();