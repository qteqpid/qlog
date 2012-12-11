#include <stdio.h>
#include "dso.h"

int handler(char * msg) {
	printf("I am here in category:%s\n", msg);
	return OK;
}

Channel category = {
	STANDARD_CHANNEL_STUFF,
	handler
};
