#include "k_s_definitions.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define BUFFER_SZ 2048

char *endMsg = ":end";
char policko[24];
int podarie = 0;

void printError(char *str) {
    if (errno != 0) {
		perror(str);
	}
	else {
		fprintf(stderr, "%s\n", str);
	}
    exit(EXIT_FAILURE);
}

void discard(const char *buf) {
    if(strchr(buf, '\n') == NULL) {
        while(getchar() != '\n');
    }
}

char *readLine(char *buf, size_t size) {
    printf("$ ");
    if (fgets(buf, size, stdin) != NULL) {
        discard(buf);
    }
    return buf;
}

void nastavNaX() {
    for (int i = 0; i < 24; ++i) {
        policko[i] = 'x';
    }
}

char getPolicko(int index) {
    return policko[index];
}

void nastavPoradie(int _poradie) {
    podarie = _poradie;
}

int getPoradie() {
    return podarie;
}

//void vykresli(char* vykresli_) {
//    sprintf(vykresli_,"***********|%cx|%cx|%cx|*************\n"
//                      "**|\033[31mxx\033[0m|\033[31mxx\033[0m|****|xx|yy|xx|**|\033[34mxx\033[0m|\033[34mxx\033[0m|****\n"
//                      "**|\033[31mxx\033[0m|\033[31mxx\033[0m|****|xx|yy|xx|**|\033[34mxx\033[0m|\033[34mxx\033[0m|****\n"
//                      "*************|xx|yy|xx|*************\n"
//                      "*************|xx|yy|xx|*************\n"
//                      "*|xx|xx|xx|xx|xx|**|xx|xx|xx|xx|xx|*\n"
//                      "*|xx|yy|yy|yy|yy|**|yy|yy|yy|yy|xx|*\n"
//                      "*|xx|xx|xx|xx|xx|**|xx|xx|xx|xx|xx|*\n"
//                      "*************|xx|yy|xx|*************\n"
//                      "*************|xx|yy|xx|*************\n"
//                      "**|\033[33mxx\033[0m|\033[33mxx\033[0m|****|xx|yy|xx|****|\033[32mxx\033[0m|\033[32mxx\033[0m|**\n"
//                      "**|\033[33mxx\033[0m|\033[33mxx\033[0m|****|xx|yy|xx|****|\033[32mxx\033[0m|\033[32mxx\033[0m|**\n"
//                      "*************|xx|xx|xx|*************\n",getPolicko(0),getPolicko(1),getPolicko(2));
//}