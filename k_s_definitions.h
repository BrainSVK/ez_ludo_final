#ifndef K_DEFINITIONS_H
#define	K_DEFINITIONS_H


#include <stddef.h>

#ifdef	__cplusplus
extern "C" {
#endif

#define BUFFER_LENGTH 300
extern char *endMsg;

void printError(char *str);
void vykresli(char* vykresli_);
void nastavPoradie(int _poradie);
int getPoradie();
void str_overwrite_stdout();
void str_trim_lf(char* arr, int lenght);

#ifdef	__cplusplus
}
#endif

#endif	/* K_DEFINITIONS_H */

