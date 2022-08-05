#include <stdio.h>
#include <string.h>

#define DH_IMPLEMENT_HERE
#include "dh_cuts.h"
#include "c-stache.h"

static char *templateTextSimple = "{{#subjects}}Unbelievable! You, {{ subjectNameHere }}, must be the pride of {{ subjectHometownHere }}!\n{{/subjects}}\n";

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
		return templateTextSimple;
	} else {
		return NULL;
	}
}

int
main()
{
	dh_init(stderr);

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
	if (!template) {
		fprintf(stderr, "c_stache_load_template()\n");
		exit(1);
	}
	c_stache_render(template, &model, &sink);

	c_stache_shutdown_engine(&engine);

	dh_summarize();
	return 0;
}

