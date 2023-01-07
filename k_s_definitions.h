#ifndef K_DEFINITIONS_H
#define	K_DEFINITIONS_H


#include <stddef.h>

#ifdef	__cplusplus
extern "C" {
#endif

#define BUFFER_LENGTH 300
extern char *endMsg;
extern char policko[24];

void printError(char *str);
void discard(const char *buf);
char *readLine(char *buf, size_t size);
void vykresli(char* vykresli_);
void nastavNaX();
char getPolicko(int index);
void nastavPoradie(int _poradie);
int getPoradie();

#ifdef	__cplusplus
}
#endif

#endif	/* K_DEFINITIONS_H */

