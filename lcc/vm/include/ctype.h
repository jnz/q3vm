/* ctype.h -- character classes, implemented in vmlib.c */

#ifndef VM_CTYPE_H
#define VM_CTYPE_H

int isdigit(int c);
int isupper(int c);
int islower(int c);
int isalpha(int c);
int isalnum(int c);
int isspace(int c);
int isprint(int c);
int toupper(int c);
int tolower(int c);

#endif
