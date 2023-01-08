#include "k_s_definitions.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define BUFFER_SZ 2048

char *endMsg = ":end";
int podarie = -1;

void printError(char *str) {
    if (errno != 0) {
		perror(str);
	}
	else {
		fprintf(stderr, "%s\n", str);
	}
    exit(EXIT_FAILURE);
}

void nastavPoradie(int _poradie) {
    podarie = _poradie;
}

int getPoradie() {
    return podarie;
}

void str_overwrite_stdout() {
    printf("\r%s", "> ");
    fflush(stdout);
}

void str_trim_lf(char* arr, int lenght) {
    for (int i = 0; i < lenght; ++i) {
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}
