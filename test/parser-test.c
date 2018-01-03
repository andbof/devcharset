#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"

#define NUM_TESTS 34

struct test {
	char *input;
	char *answer;
};

static struct test characters[] = {
	{ .input = "a", .answer = "a" },
	{ .input = "0", .answer = "0" },
	{ .input = "\\0141", .answer = "a" },
	{ .input = "\\98", .answer = "b" },
	{ .input = "\\x63", .answer  = "c" },
	{ NULL, NULL }
};

static struct test peculiarities[] = {
	{ .input = "c-c", .answer = "c" },
	{ .input = "\\01411-:", .answer = "123456789:a" },
	{ .input = "\\1001-:", .answer = "123456789:d" },
	{ .input = "\\x611-:", .answer = "123456789:a" },
	{ .input = "\\97 1-:", .answer = "123456789:a" },
	{ NULL, NULL }
};

static struct test ranges[] = {
	{ .input = "a-c", .answer = "abc" },
	{ .input = "a-cd", .answer = "abcd" },
	{ .input = "da-c", .answer = "abcd" },
	{ .input = "ac-eg-ik", .answer = "acdeghik" },
	{ .input = "a-\\99e-g", .answer = "abcefg" },
	{ .input = "a-\\0143e-g", .answer = "abcefg" },
	{ .input = "a-\\x63e-g", .answer = "abcefg" },
	{ NULL, NULL }
};

static const char *invalids[] = {
	"",
	"c-a",
	"a-c-g",
	"\\0-\\256",
	"\\0-\\999",
	"\\0-\\0378",
	"\\0-\\0777",
	"\\254-\\256",
	"\\255-\\256",
	"\\0376-\\0378",
	"\\0377-\\0378",
	"\\0--\\255",
	"\\00--\\0123",
	NULL
};

static const char *extremes[] = {
	"\\x00-\\xff",
	"\\0000-\\0377",
	"\\000-\\255",
	"\\0-\\255",
	NULL
};

#define MAGIC_PTR ((void*)0x42)
#define MAGIC_INT 0x42
static unsigned int test_invalid_input(void)
{
	unsigned int num_tests = 0;
	char *out = MAGIC_PTR;
	size_t size = MAGIC_INT;

	for (const char **test = invalids; *test; test++) {
		assert(set_charset(&out, &size, *test));
		/*
		 * Make sure set_charset() hasn't touched out and size,
		 * as the charset was invalid.
		 */
		assert(out == MAGIC_PTR);
		assert(size == MAGIC_INT);

		num_tests++;
	}

	return num_tests;
}

static unsigned int test_extremes(void)
{
	unsigned int num_tests = 0;
	char *out;
	size_t size;
	char answer[256];

	for (size_t i = 0; i < sizeof(answer); i++)
		answer[i] = i;

	for (const char **test = extremes; *test; test++) {
		assert(!set_charset(&out, &size, *test));
		assert(out);
		assert(size == 256);
		assert(!memcmp(out, answer, size));

		free(out);
		num_tests++;
	}

	return num_tests;
}

static unsigned int test(const struct test tests[])
{
	unsigned int num_tests = 0;
	char *out;
	size_t size;

	for (const struct test *test = tests; test->input; test++) {
		assert(!set_charset(&out, &size, test->input));
		assert(out);
		assert(!memcmp(out, test->answer, size));

		free(out);
		num_tests++;
	}

	return num_tests;
}

int main()
{
	unsigned int num_tests = 0;

	fprintf(stderr, "This test binary will write error messages to stderr.\n"
			"This is nothing unusual. Test status should only be "
			"checked by means of the exit code.\n");

	num_tests += test(characters);
	num_tests += test(ranges);
	num_tests += test(peculiarities);
	num_tests += test_invalid_input();
	num_tests += test_extremes();

	assert(num_tests == NUM_TESTS);
}
