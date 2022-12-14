#include <stdio.h>
#include <string.h>

#define DH_IMPLEMENT_HERE
#include "dh_cuts.h"
#include "c-stache.h"

static const char *templateTextInner = "^{{item}}$\n";
static const char *templateTextOuter = "{{! this is a comment! }}header\n{{# the-section}}{{> inner}}{{/ the-section}}";
static const char *templateTextNoEnd = "{{ key ";
static const char *templateTextBadPartial = "{{> noend}}";

struct string_sink {
	char  *text;
	size_t length;
};

static int
enter_cb(void *userptr, const char *section)
{
	int *n = userptr;
	if (*n < 0 && !strcmp(section, "the-section")) {
		*n = 0;
		return 1;
	} else {
		return 0;
	}
}

static int
next_cb(void *userptr)
{
	int *n = userptr;
	if (*n >= 0 && *n < 3) {
		++*n;
		return 1;
	} else {
		return 0;
	}
}

static void
leave_cb(void *userptr)
{
	*(int *) userptr = -1;
}

static const char *
subst_cb(void *userptr, const char *key)
{
	static char buf[50];
	int *n = userptr;
	if (*n >= 0 && !strcmp(key, "item")) {
		sprintf(buf, "Item #%d", *n);
		return buf;
	} else {
		return NULL;
	}
}

static int
write_cb(void *userptr, const char *text, size_t length)
{
	struct string_sink *str = userptr;
	str->text = realloc(str->text, str->length + length);
	memcpy(str->text + str->length, text, length);
	str->length += length;
	return 0;
}

static char *
read_cb(const char *name, size_t *length)
{
	const char *pointer;
	if      (!strcmp(name, "inner"))      pointer = templateTextInner;
	else if (!strcmp(name, "outer"))      pointer = templateTextOuter;
	else if (!strcmp(name, "noend"))      pointer = templateTextNoEnd;
	else if (!strcmp(name, "badpartial")) pointer = templateTextBadPartial;
	else return NULL;
	*length = strlen(pointer);
	return strdup(pointer);
}

static void
test_complete_runthrough(void)
{
	const char *expectedResult = "header\n^Item #1$\n^Item #2$\n^Item #3$\n";

	dh_push("complete run-through");

	CStacheEngine engine;
	c_stache_start_engine(&engine, read_cb);

	int num = -1;
	CStacheModel model = {
		.enter   = enter_cb,
		.next    = next_cb,
		.leave   = leave_cb,
		.subst   = subst_cb,
		.userptr = &num
	};

	struct string_sink str = { 0 };
	CStacheSink sink = {
		.escape  = c_stache_escape_xml,
		.write   = write_cb,
		.userptr = &str
	};

	CStacheTemplate *template;
	dh_assertiq(c_stache_load_template(&engine, "outer", &template), C_STACHE_OK);
	dh_assert(template != NULL);

	dh_assertiq(c_stache_render(template, &model, &sink), C_STACHE_OK);
	dh_assert(str.length == strlen(expectedResult) && !memcmp(expectedResult, str.text, str.length));

	free(str.text);

	c_stache_shutdown_engine(&engine);

	dh_pop();
}

static void
test_failure_handling(void)
{
	dh_push("failure handling");

	CStacheEngine engine;
	c_stache_start_engine(&engine, read_cb);

	int num = -1;
	CStacheModel model = {
		.enter   = enter_cb,
		.next    = next_cb,
		.leave   = leave_cb,
		.subst   = subst_cb,
		.userptr = &num
	};

	struct string_sink str = { 0 };
	CStacheSink sink = {
		.escape  = c_stache_escape_xml,
		.write   = write_cb,
		.userptr = &str
	};

	CStacheTemplate *template;
	dh_assertiq(c_stache_load_template(&engine, "badpartial", &template), C_STACHE_OK);
	dh_assert(template != NULL);

	CStacheTemplate *partial;
	dh_assertiq(c_stache_load_template(&engine, "noend", &partial), C_STACHE_ERROR_NO_END);
	dh_assert(partial != NULL);
	dh_assertiq(partial->refcount, 2);
	c_stache_drop_template(&engine, partial);

	dh_assertiq(c_stache_render(template, &model, &sink), C_STACHE_ERROR_BAD_TPL);

	free(str.text);

	c_stache_shutdown_engine(&engine);

	dh_pop();
}

static void
test_escape_xml(void)
{
	char buf[7];
	const char *text = "<>x";
	const char *ptr = text;
	size_t num;

	dh_push("escape xml");

	dh_push("reentry");

	num = c_stache_escape_xml(&ptr, buf, sizeof buf);
	dh_assertiq(num, 4);
	dh_assert(!strncmp(buf, "&lt;", 4));

	num = c_stache_escape_xml(&ptr, buf, sizeof buf);
	dh_assertiq(num, 5);
	dh_assert(!strncmp(buf, "&gt;x", 5));
	dh_assert(!*ptr);

	dh_pop();

	dh_pop();
}

int
main()
{
	dh_init(stderr);
	dh_branch( test_complete_runthrough(); )
	dh_branch( test_failure_handling(); )
	dh_branch( test_escape_xml(); );
	dh_summarize();
	return 0;
}

