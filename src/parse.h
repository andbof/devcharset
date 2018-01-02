#ifndef _PARSE_H
#define _PARSE_H

#define YYSTYPE unsigned char

int yylex();
int yyparse();
int parse(const char *input);
int set_charset(char **out_charset, size_t *out_size, const char *input);

#endif
