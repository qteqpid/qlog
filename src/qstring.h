#ifndef QSTRING_H
#define QSTRING_H

char * qstrdup(const char *str);

char * strcat2(int argc, const char *str1, const char * str2, ...);

void strreplace(char **old, const char *new);

char * strim(char *str);

char ** strsplit(char *line, char delimeter, int *count, int limit);

int yesnotoi(char *str);

#endif
