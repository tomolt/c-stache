#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "c-stache.h"

static inline int
iskey(int c)
{
	const char lut[256] = {
		['0' ... '9'] = 1,
		['a' ... 'z'] = 1,
		['A' ... 'Z'] = 1,
		['_'] = 1,
		['-'] = 1,
		['.'] = 1,
	};
	return lut[c];
}

int
c_stache_parse(CStacheTemplate *tpl, const char *text, size_t length)
{
	const char *startDelim = "{{";
	const char *endDelim   = "}}";
	const char *ptr = text;
	CStacheTag *tag;

	tpl->text   = text;
	tpl->length = length;

	while ((ptr = strstr(ptr, startDelim))) {
		/* allocate new tag */
		if (tpl->numTags == tpl->capTags) {
			tpl->capTags = tpl->capTags ? 2 * tpl->capTags : 16;
			tpl->tags = realloc(tpl->tags, tpl->capTags * sizeof *tpl->tags);
			if (!tpl->tags)
				return -1;
		}
		tag = &tpl->tags[tpl->numTags++];
		
		/* tag start */
		tag->pointer = ptr;
		ptr += strlen(startDelim);
		if (strchr("&#/^!>", *ptr))
			tag->kind = *(ptr++);
		else
			tag->kind = 0;
		while (isspace(*ptr)) ptr++;

		/* key */
		tag->keyStart = ptr - tag->pointer;
		while (iskey(*ptr)) ptr++;
		tag->keyEnd = ptr - tag->pointer;
		if (tag->keyStart == tag->keyEnd)
			return -1;

		/* tag end */
		while (isspace(*ptr)) ptr++;
		if (strcmp(ptr, endDelim))
			return -1;
		ptr += strlen(endDelim);
	}

	return 0;
}

#if 0
void
c_stache_render(const char *text)
{
	char *cur = text;
	size_t tagidx = 0;

	for (;;) {
		char *end = ;

		cb->write_out(cb->userdata, cur, end - cur);

		switch (tags[tagidx].kind) {
		case '&':
			break;
		case '#':
			break;
		case '/':
			break;
		case '^':
			break;
		case '!':
			break;
		case '>':
			break;
		default:
		}
		
		cur = end;
	}

	cb->write_out(cb->userdata, cur, strlen(cur));

	cb->release(cb->userdata);
}
#endif

