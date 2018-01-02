%option noyywrap
%option 8bit
%option warn nodefault
%option never-interactive

%option noinput
%option nounput

%{

#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include "parse.h"
#include "parser.h"

static int str_to_char(const char *str, char *out, int base)
{
	char *endp;
	int res;

	errno = 0;
	res = strtol(str, &endp, base);
	if (errno)
		return errno;
	if (endp == str || *endp)
		return EINVAL;
	if (res < 0 || res > 255)
		return ERANGE;

	*out = res;

	return 0;
}

static int parse_char(char *str, int base)
{
	char c;
	if (str_to_char(str, &c, base))
		return INVALID;

	yylval = c;
	return CHAR;
}

%}

%%

[ \t]			;
\\x[0-9a-fA-F]{1,2}	{
				return parse_char(yytext + 2, 16);
			}
\\0[0-9]{1,3}		{
				return parse_char(yytext + 2, 8);
			}
\\[0-9]{1,3}		{
				return parse_char(yytext + 1, 10);
			}
-			{
				return DASH;
			}
.			{
				yylval = yytext[0];
				return CHAR;
			}

%%

int parse(const char *input)
{
	YY_BUFFER_STATE yybs = yy_scan_string(input);
	if (!yybs)
		return 1;

	yy_switch_to_buffer(yybs);

	if (yyparse())
		goto err_delete;

	yy_delete_buffer(yybs);

	yylex_destroy();
	return 0;

err_delete:
	yy_delete_buffer(yybs);

	return 1;
}
