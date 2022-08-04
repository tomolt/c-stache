#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "c-stache.h"

size_t
c_stache_escape_xml(const char **text, char *buf, size_t max)
{
	const char *subst;
	size_t len, written = 0;
	while (written < max) {
		switch (**text) {
		case '<':  subst = "&lt;";   break;
		case '>':  subst = "&gt;";   break;
		case '\'': subst = "&#39;";  break;
		case '&':  subst = "&amp;";  break;
		case '"':  subst = "&quot;"; break;
		case 0:    return written;
		default:
			buf[written++] = **text;
			(*text)++;
			continue;
		}

		len = strlen(subst);
		if (written + len > max) break;
		memcpy(buf + written, subst, len);
		written += len;
		(*text)++;
	}
	return written;
}

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
c_stache_render(const CStacheTemplate *tpl, CStacheModel *model, CStacheSink *sink)
{
	char key[256];
	CStacheTag *tag = &tpl->tags[0];
	const char *cur = tpl->text;
	const char *tmp;
	size_t written;

	while (tag < tpl->tags + tpl->numTags) {
		if (cur < tag->pointer)
			sink->write(sink->userdata, cur, tag->pointer - cur);

		if (tag->keyLength + 1 > sizeof key)
			return;
		memcpy(key, tag->pointer + tag->keyStart, tag->keyLength);
		key[tag->keyLength] = 0;

		switch (tag->kind) {
		case '&':
			tmp = model->subst(model->userdata, key);
			sink->write(sink->userdata, tmp, strlen(tmp));
			break;
		case '#':
			if (!model->enter(model->userdata, key))
				tag = tag->buddy;
			break;
		case '/':
			if (model->next(model->userdata))
				tag = tag->buddy;
			break;
		case '^':
			if (!model->empty(model->userdata, key))
				tag = tag->buddy;
			break;
		default:
			tmp = model->subst(model->userdata, key);
			if (!tmp)
				return;
			while (*tmp) {
				written = sink->escape(&tmp, key, sizeof key);
				sink->write(sink->userdata, key, written);
			}
		}
		
		cur = tag->pointer + tag->tagLength;
		tag++;
	}

	if (cur - tpl->text < tpl->length)
		sink->write(sink->userdata, cur, tpl->length - (cur - tpl->text));
}

