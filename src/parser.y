%{
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parse.h"
#include "parser.h"

static char valid_chars[256];

static void yyerror(const char * const msg)
{
	fprintf(stderr, "invalid charset: %s\n", msg);
}

%}

%token	CHAR;
%token	DASH;
%token	INVALID;

%%

line:	line token
    |	token
    ;

token:	CHAR DASH CHAR {
		if ($1 > $3) {
			yyerror("ranges must be positive");
			YYABORT;
		}
		for (size_t i = $1; i <= $3; i++)
			valid_chars[i] = 1;
	}
     |	CHAR {
		valid_chars[$1] = 1;
	}
     |	INVALID {
		yyerror("invalid character");
		YYABORT;
	}
     ;

%%

static int parse_charset(const char *input)
{
	memset(valid_chars, 0, sizeof(valid_chars));
	return parse(input);
}

static size_t charset_size(void)
{
	size_t size = 0;

	for (size_t i = 0; i < sizeof(valid_chars); i++) {
		if (valid_chars[i])
			size++;
	}

	return size;
}

int set_charset(char **out_charset, size_t *out_size, const char *input)
{
	size_t size;
	char *charset;

	if (!input)
		return 1;

	if (parse_charset(input))
		return 1;

	size = charset_size();
	if (!size)
		return 1;

	charset = malloc(size);
	if (!charset)
		return 1;

	size_t j = 0;
	for (size_t i = 0; i < sizeof(valid_chars); i++) {
		if (valid_chars[i])
			charset[j++] = i;
	}

	*out_size = size;
	*out_charset = charset;

	return 0;
}
