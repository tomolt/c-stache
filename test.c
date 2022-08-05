#include <stdio.h>
#include <string.h>

#define DH_IMPLEMENT_HERE
#include "dh_cuts.h"
#include "c-stache.h"

static const char *templateTextSimple = "{{#subjects}}Unbelievable! You, {{ subjectNameHere }}, must be the pride of {{ subjectHometownHere }}!\n{{/subjects}}\n";

static int
enter_cb(void *userdata, const char *section)
{
	(void) userdata;
	printf("E %s\n", section);
	return 1;
}

static int
next_cb(void *userdata)
{
	(void) userdata;
	printf("N\n");
	return 0;
}

static int
empty_cb(void *userdata, const char *section)
{
	(void) userdata;
	printf("? %s\n", section);
	return 0;
}

static const char *
subst_cb(void *userdata, const char *key)
{
	(void) userdata;
	(void) key;
	return "<substituted value>";
}

static int
write_cb(void *userdata, const char *text, size_t length)
{
	(void) userdata;
	printf(".%.*s\n", (int) length, text);
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
	dh_push("complete run-through");

	CStacheEngine engine;
	c_stache_start_engine(&engine, read_cb);

	CStacheModel model = {
		.enter = enter_cb,
		.next  = next_cb,
		.empty = empty_cb,
		.subst = subst_cb,
		.userdata = NULL
	};

	CStacheSink sink = {
		.escape = c_stache_escape_xml,
		.write  = write_cb,
		.userdata = NULL
	};

	const CStacheTemplate *template;
	template = c_stache_load_template(&engine, "simple");
	dh_assert(template != NULL);

	c_stache_render(template, &model, &sink);

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

