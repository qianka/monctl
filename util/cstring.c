#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>


char *ltrim(char *s)
{
	size_t len = strlen(s);
	while (len && isspace(*s)) {
		s++;
		len--;
	}

	return s;
}


char *rtrim(char *s)
{
	size_t size = strlen(s);
	char *end;

	if (!size)
		return s;

	end = s + size - 1;
	while (end >= s && isspace(*end))
		end--;
	*(end + 1) = '\0';

	return s;
}

char *trim(char *s)
{
	s = ltrim(s);
	s = rtrim(s);
	return s;
}
