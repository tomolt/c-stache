#include <stddef.h>

typedef struct c_stache_callbacks CStacheCallbacks;
typedef struct c_stache_tag       CStacheTag;
typedef struct c_stache_template  CStacheTemplate;

struct c_stache_callbacks {
	void        (*enter )(void *userdata, const char *section);
	void        (*leave )(void *userdata, const char *section);
	void        (*next  )(void *userdata);
	const char *(*lookup)(void *userdata, const char *key, int escape);
	const char *(*load  )(void *userdata, const char *filename);
	void        (*write )(void *userdata, const char *data, size_t length);

	void *userdata;
};

struct c_stache_tag {
	const char    *pointer;
	unsigned short keyStart;
	unsigned short keyEnd;
	unsigned short tagEnd;
	char           kind;
};

struct c_stache_template {
	CStacheTag *tags;
	size_t      numTags;
	size_t      capTags;
	const char *text;
	size_t      length;
};

int c_stache_parse(CStacheTemplate *tpl, const char *text, size_t length);

