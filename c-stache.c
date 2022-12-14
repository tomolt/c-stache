#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "c-stache.h"

const char *
c_stache_strerror(int status)
{
	switch (status) {
	case C_STACHE_OK:            return "ok";
	case C_STACHE_ERROR_OOM:     return "out of memory";
	case C_STACHE_ERROR_IO:      return "I/O error";
	case C_STACHE_ERROR_NO_END:  return "tag is not closed";
	case C_STACHE_ERROR_NO_KEY:  return "tag has no key";
	case C_STACHE_ERROR_PAIRING: return "section start and end are mismatched";
	case C_STACHE_ERROR_BAD_TPL: return "attempting to use template that was not loaded successfully";
	default:                     return "an unknown error has occurred";
	}
}

static void
c_stache_free_template(CStacheTemplate *tpl)
{
	if (!tpl) return;
	free(tpl->name);
	free(tpl->tags);
	free(tpl->text);
	free(tpl);
}

void
c_stache_start_engine(CStacheEngine *engine, char *(*read)(const char *name, size_t *length))
{
	memset(engine, 0, sizeof *engine);
	engine->read = read;
	engine->startDelim = "{{";
	engine->endDelim   = "}}";
}

void
c_stache_shutdown_engine(CStacheEngine *engine)
{
	size_t i;

	for (i = 0; i < engine->numTemplates; i++) {
		c_stache_free_template(engine->templates[i]);
	}
	free(engine->templates);
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

static int
c_stache_parse(CStacheEngine *engine, CStacheTemplate *tpl)
{
	char *ptr = tpl->text;
	char *keyEnd;
	CStacheTag *tag;
	void *mem;

	while ((ptr = strstr(ptr, engine->startDelim))) {
		/* allocate new tag */
		if (tpl->numTags == tpl->capTags) {
			tpl->capTags = tpl->capTags ? 2 * tpl->capTags : 16;
			mem = realloc(tpl->tags, tpl->capTags * sizeof *tpl->tags);
			if (!mem)
				return C_STACHE_ERROR_OOM;
			tpl->tags = mem;
		}
		tag = &tpl->tags[tpl->numTags++];
		
		tag->pointer = ptr;
		ptr += strlen(engine->startDelim);

		tag->kind = 0;
		keyEnd = NULL;
		switch (*ptr) {
		case '!':
			tag->kind = *(ptr++);
			if (!(ptr = strstr(ptr, engine->endDelim)))
				return C_STACHE_ERROR_NO_END;
			break;

		case '&': case '#': case '/': case '^': case '>':
			tag->kind = *(ptr++);
			/* fallthrough */

		default:
			while (isspace(*ptr)) ptr++;
			tag->keyStart = ptr - tag->pointer;
			while (iskey(*ptr)) ptr++;
			if (ptr == tag->pointer + tag->keyStart)
				return C_STACHE_ERROR_NO_KEY;
			keyEnd = ptr;
			while (isspace(*ptr)) ptr++;
		}

		if (strncmp(ptr, engine->endDelim, strlen(engine->endDelim)))
			return C_STACHE_ERROR_NO_END;
		ptr += strlen(engine->endDelim);
		tag->tagLength = ptr - tag->pointer;
		if (keyEnd) *keyEnd = 0;
	}

	return C_STACHE_OK;
}

static int
c_stache_weave(CStacheEngine *engine, CStacheTemplate *tpl)
{
	CStacheTag *tag, *top = NULL;

	for (tag = tpl->tags; tag < tpl->tags + tpl->numTags; tag++) {
		if (tag->kind == '#' || tag->kind == '^') {
			tag->buddy = top;
			top = tag;
		} else if (tag->kind == '/') {
			if (!top || strcmp(tag->pointer + tag->keyStart, top->pointer + top->keyStart))
				return C_STACHE_ERROR_PAIRING;
			tag->buddy = top;
			top = top->buddy;
			tag->buddy->buddy = tag;
		} else if (tag->kind == '>') {
			/* ignore return code - template should not fail to load just because
			 * a referenced template is unusable. */
			c_stache_load_template(engine, tag->pointer + tag->keyStart, &tag->otherTpl);
		}
	}

	return top ? C_STACHE_ERROR_PAIRING : C_STACHE_OK;
}

int
c_stache_load_template(CStacheEngine *engine, const char *name, CStacheTemplate **template)
{
	size_t i;
	CStacheTemplate *tpl;
	void *mem;

	for (i = 0; i < engine->numTemplates; i++) {
		tpl = engine->templates[i];
		if (!strcmp(tpl->name, name)) {
			tpl->refcount++;
			*template = tpl;
			return tpl->status;
		}
	}

	*template = NULL;

	if (engine->numTemplates == engine->capTemplates) {
		engine->capTemplates = engine->capTemplates ? 2 * engine->capTemplates : 16;
		mem = realloc(engine->templates, engine->capTemplates * sizeof *engine->templates);
		if (!mem)
			return C_STACHE_ERROR_OOM;
		engine->templates = mem;
	}

	tpl = calloc(1, sizeof *tpl);
	*template = tpl;
	if (!tpl)
		return C_STACHE_ERROR_OOM;
	tpl->name = strdup(name);
	if (!tpl->name) {
		free(tpl);
		return C_STACHE_ERROR_OOM;
	}
	tpl->status = C_STACHE_OK;
	tpl->refcount = 1;
	engine->templates[engine->numTemplates++] = tpl;
	
	tpl->text = engine->read(name, &tpl->length);
	if (!tpl->text) {
		tpl->status = C_STACHE_ERROR_IO;
		return tpl->status;
	}
	if ((tpl->status = c_stache_parse(engine, tpl)) < 0)
		return tpl->status;
	if ((tpl->status = c_stache_weave(engine, tpl)) < 0)
		return tpl->status;

	return C_STACHE_OK;
}

void
c_stache_drop_template(CStacheEngine *engine, CStacheTemplate *tpl)
{
	CStacheTag *tag;
	size_t i;

	if (!tpl || --tpl->refcount) return;

	for (i = 0; i < engine->numTemplates; i++) {
		if (engine->templates[i] == tpl)
			break;
	}
	engine->templates[i] = engine->templates[--engine->numTemplates];

	for (i = 0; i < tpl->numTags; i++) {
		tag = &tpl->tags[i];
		if (tag->kind == '>')
			c_stache_drop_template(engine, tag->otherTpl);
	}

	c_stache_free_template(tpl);
}

static int
c_stache_render_recursive(const CStacheTemplate *tpl, CStacheModel *model, CStacheSink *sink, int maxDepth)
{
	char buf[512];
	CStacheTag *tag = &tpl->tags[0];
	const char *cur = tpl->text;
	const char *tmp;
	size_t written;
	int s;

	if (tpl->status != C_STACHE_OK)
		return C_STACHE_ERROR_BAD_TPL;

	while (tag < tpl->tags + tpl->numTags) {
		if (cur < tag->pointer)
			sink->write(sink->userptr, cur, tag->pointer - cur);

		switch (tag->kind) {
		case '!':
			break;

		case '&':
			tmp = model->subst(model->userptr, tag->pointer + tag->keyStart);
			sink->write(sink->userptr, tmp, strlen(tmp));
			break;

		case '#':
			if (model->enter(model->userptr, tag->pointer + tag->keyStart)) {
				if (!model->next(model->userptr))
					model->leave(model->userptr);
			} else {
				tag = tag->buddy;
			}
			break;

		case '/':
			if (model->next(model->userptr))
				tag = tag->buddy;
			else
				model->leave(model->userptr);
			break;

		case '^':
			if (model->enter(model->userptr, tag->pointer + tag->keyStart)) {
				if (model->next(model->userptr))
					tag = tag->buddy;
				model->leave(model->userptr);
			}
			break;

		case '>':
			s = c_stache_render_recursive(tag->otherTpl, model, sink, maxDepth - 1);
			if (s < 0)
				return s;
			break;

		default:
			tmp = model->subst(model->userptr, tag->pointer + tag->keyStart);
			if (!tmp)
				return -1; /* FIXME propagate proper error code! */
			while (*tmp) {
				written = sink->escape(&tmp, buf, sizeof buf);
				sink->write(sink->userptr, buf, written);
			}
		}
		
		cur = tag->pointer + tag->tagLength;
		tag++;
	}

	if (cur - tpl->text < tpl->length)
		sink->write(sink->userptr, cur, tpl->length - (cur - tpl->text));

	return C_STACHE_OK;
}

int
c_stache_render(const CStacheTemplate *tpl, CStacheModel *model, CStacheSink *sink)
{
	return c_stache_render_recursive(tpl, model, sink, C_STACHE_MAX_PARTIAL_DEPTH);
}

char *
c_stache_read_file(const char *name, size_t *length)
{
	FILE *file;
	char *data;

	*length = 0;

	file = fopen(name, "rb");
	if (!file) return NULL;

	fseek(file, 0, SEEK_END);
	*length = ftell(file);
	fseek(file, 0, SEEK_SET);

	data = malloc(*length + 1);
	if (!data) {
		fclose(file);
		return NULL;
	}

	if (fread(data, 1, *length, file) != *length) {
		free(data);
		fclose(file);
		return NULL;
	}
	data[*length] = 0;

	fclose(file);

	return data;
}

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

int
c_stache_write_file(void *fileptr, const char *text, size_t length)
{
	return fwrite(text, 1, length, fileptr) == length ? C_STACHE_OK : C_STACHE_ERROR_IO;
}

