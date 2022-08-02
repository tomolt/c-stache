#include <stdio.h>
#include <string.h>

#define DH_IMPLEMENT_HERE
#include "dh_cuts.h"
#include "c-stache.h"

static int
enter_cb(void *userdata, const char *section)
{
	(void) userdata;
	printf("E\n");
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
isempty_cb(void *userdata, const char *section)
{
	(void) userdata;
	printf("?\n");
	return 0;
}

static void
subst_cb(void *userdata, const char *key, int escape)
{
	(void) userdata;
	printf(".<substituted value>\n");
}

static void
write_cb(void *userdata, const char *data, size_t length)
{
	(void) userdata;
	printf(".%.*s\n", (int) length, data);
}

int
main()
{
	const char *text = "Unbelievable! You, {{ subjectNameHere }}, must be the pride of {{ subjectHometownHere }}\n";

	dh_init(stderr);

	CStacheCallbacks callbacks = {
		.enter   = enter_cb,
		.next    = next_cb,
		.isempty = isempty_cb,
		.subst   = subst_cb,
		.write   = write_cb,
		.userdata = NULL
	};

	CStacheTemplate template;
	if (c_stache_parse(&template, text, strlen(text))) {
		fprintf(stderr, "c_stache_parse()\n");
		exit(1);
	}
	c_stache_render(&template, &callbacks);

	dh_summarize();
	return 0;
}

