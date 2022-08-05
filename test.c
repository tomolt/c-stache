#include <stdio.h>
#include <string.h>

#define DH_IMPLEMENT_HERE
#include "dh_cuts.h"
#include "c-stache.h"

static const char *templateTextSimple = "{{! this is a comment! }}{{#subjects}}Unbelievable! You, {{ subjectNameHere }}, must be the pride of {{ subjectHometownHere }}!\n{{/subjects}}\n";

struct string_sink {
	char  *text;
	size_t length;
};

static int
enter_cb(void *userptr, const char *section)
{
	(void) userptr;
	printf("E %s\n", section);
	return 1;
}

static int
next_cb(void *userptr)
{
	(void) userptr;
	printf("N\n");
	return 0;
}

static int
empty_cb(void *userptr, const char *section)
{
	(void) userptr;
	printf("? %s\n", section);
	return 0;
}

static const char *
subst_cb(void *userptr, const char *key)
{
	(void) userptr;
	(void) key;
	return "<substituted value>";
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
	if (!strcmp(name, "simple")) {
		*length = strlen(templateTextSimple);
		return strdup(templateTextSimple);
	} else {
		return NULL;
	}
}

static void
test_complete_runthrough(void)
{
	int s;

	dh_push("complete run-through");

	CStacheEngine engine;
	c_stache_start_engine(&engine, read_cb);

	CStacheModel model = {
		.enter   = enter_cb,
		.next    = next_cb,
		.empty   = empty_cb,
		.subst   = subst_cb,
		.userptr = NULL
	};

	struct string_sink str = { 0 };
	CStacheSink sink = {
		.escape  = c_stache_escape_xml,
		.write   = write_cb,
		.userptr = &str
	};

	CStacheTemplate *template;
	s = c_stache_load_template(&engine, "simple", &template);
	dh_assert(s == C_STACHE_OK);
	dh_assert(template != NULL);

	s = c_stache_render(template, &model, &sink);
	dh_assert(s == C_STACHE_OK);

	printf("OUTPUT:\n---\n%.*s\n---\n", (int) str.length, str.text);
	free(str.text);

	c_stache_shutdown_engine(&engine);

	dh_pop();
}

int
main()
{
	dh_init(stderr);
	dh_branch( test_complete_runthrough(); )
	dh_summarize();
	return 0;
}

