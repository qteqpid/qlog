#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include "qstring.h"

char * strdup(const char *str)
{
	size_t len = strlen(str)+1;
	char *dup = malloc(len);
	memcpy(dup, str, len);
	return dup;
}

char * strcat2(char *str1, char * str2, ...) 
{
	char *dest = NULL;
	va_list va_ptr;
	char *cur = NULL;
	size_t len = strlen(str1) + strlen(str2);

	va_start(va_ptr, str2);
	while((cur = va_arg(va_ptr,char *))) {
		len += strlen(cur);
	}
	va_end(va_ptr);

	dest = malloc(len+1);
	dest[0] = '\0';
	strcat(dest, str1);
	strcat(dest, str2);
	va_start(va_ptr, str2);
	while((cur = va_arg(va_ptr,char *))) {
		strcat(dest, cur);
	}
	va_end(va_ptr);

	return dest;
}

void strreplace(char **old, const char *new)
{
	free(*old);
	*old = strdup(new);
}

char * strim(char *str)
{
	char *end, *sp, *ep;
	size_t len;

	sp = str;
	end = ep = str+strlen(str)-1;
	while(sp <= end && isspace(*sp)) sp++;
	while(ep >= sp && isspace(*ep)) ep--;
	len = (ep < sp) ? 0 : (ep-sp)+1;
	sp[len] = '\0';
	return sp;
}

char ** strsplit(char *line, char delimeter, int *count, int limit)
{
	char *ptr = NULL, *str = line;
	char **vector = NULL;

	*count = 0;
	while((ptr = strchr(str, delimeter))) {
		*ptr = '\0';
		vector = realloc(vector,((*count)+1)*sizeof(char *));
		vector[*count] = strim(str);
		str = ptr+1;
		(*count)++;	
		if (--limit == 0) break;
	}
	if (*str != '\0') {
		vector = realloc(vector,((*count)+1)*sizeof(char *));
		vector[*count] = strim(str);
		(*count)++;
	}
	return vector;
}

int yesnotoi(char *str)
{
	if (strcasecmp(str,"yes") == 0) return 1;
	if (strcasecmp(str,"no") == 0) return 0;
	return -1;
}