#ifndef W_DIAGNOSTICS_H
#define W_DIAGNOSTICS_H

// error reporting shared by every stage

// prints "Error on line N: message" ("Error: message" when line is 0) and counts it.
// takes printf-style arguments. Exits after too many errors.
void report_error(int line, const char* format, ...);

// errors reported so far
int error_count(void);

#endif
