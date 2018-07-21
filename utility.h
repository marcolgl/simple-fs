#pragma once
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define ERROR_HELPER(ret, msg) do {												\
		if (ret < 0){															\
			fprintf(stderr, "Errore: %s\n errno: %s\n", msg, strerror(errno));	\
			exit(EXIT_FAILURE);													\
		}																		\
	}while(0);

