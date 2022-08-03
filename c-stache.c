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
	CStacheTag *tag, *top = NULL;

	memset(tpl, 0, sizeof *tpl);
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
		if (strchr("&#/^", *ptr))
			tag->kind = *(ptr++);
		else
			tag->kind = 0;
		while (isspace(*ptr)) ptr++;

		/* key */
		tag->keyStart = ptr - tag->pointer;
		while (iskey(*ptr)) ptr++;
		tag->keyLength = ptr - (tag->pointer + tag->keyStart);
		if (!tag->keyLength)
			return -1;

		/* tag end */
		while (isspace(*ptr)) ptr++;
		if (strncmp(ptr, endDelim, strlen(endDelim)))
			return -1;
		ptr += strlen(endDelim);
		tag->tagLength = ptr - tag->pointer;
	}

	for (tag = tpl->tags; tag < tpl->tags + tpl->numTags; tag++) {
		if (tag->kind == '#' || tag->kind == '^') {
			tag->buddy = top;
			top = tag;
		} else if (tag->kind == '/') {
			if (!top || tag->keyLength != top->keyLength
			  || memcmp(tag->pointer + tag->keyStart, top->pointer + top->keyStart, tag->keyLength))
				return -1;
			tag->buddy = top;
			top = top->buddy;
			tag->buddy->buddy = tag;
		}
	}
	if (top)
		return -1;

	return 0;
}

void
c_stache_render(const CStacheTemplate *tpl, CStacheCallbacks *cbs)
{
	char key[256];
	CStacheTag *tag = &tpl->tags[0];
	const char *cur = tpl->text;

	while (tag < tpl->tags + tpl->numTags) {
		if (cur < tag->pointer)
			cbs->write(cbs->userdata, cur, tag->pointer - cur);

		if (tag->keyLength + 1 > sizeof key)
			return;
		memcpy(key, tag->pointer + tag->keyStart, tag->keyLength);
		key[tag->keyLength] = 0;

		switch (tag->kind) {
		case '&':
			cbs->subst(cbs->userdata, key, 0);
			break;
		case '#':
			if (!cbs->enter(cbs->userdata, key))
				tag = tag->buddy;
			break;
		case '/':
			if (cbs->next(cbs->userdata))
				tag = tag->buddy;
			break;
		case '^':
			if (!cbs->isempty(cbs->userdata, key))
				tag = tag->buddy;
			break;
		default:
			cbs->subst(cbs->userdata, key, 1);
		}
		
		cur = tag->pointer + tag->tagLength;
		tag++;
	}

	if (cur - tpl->text < tpl->length)
		cbs->write(cbs->userdata, cur, tpl->length - (cur - tpl->text));
}

