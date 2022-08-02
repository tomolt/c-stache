#include <stddef.h>

typedef struct c_stache_callbacks CStacheCallbacks;
typedef struct c_stache_tag       CStacheTag;
typedef struct c_stache_template  CStacheTemplate;

struct c_stache_callbacks {
	int  (*enter  )(void *userdata, const char *section);
	int  (*next   )(void *userdata);
	int  (*isempty)(void *userdata, const char *section);
	void (*subst  )(void *userdata, const char *key, int escape);
	void (*write  )(void *userdata, const char *data, size_t length);

	void *userdata;
};

struct c_stache_tag {
	CStacheTag    *buddy;
	const char    *pointer;
	unsigned short keyStart;
	unsigned short keyLength;
	unsigned short tagLength;
	char           kind;
};

struct c_stache_template {
	CStacheTag *tags;
	size_t      numTags;
	size_t      capTags;
	const char *text;
	size_t      length;
};

int  c_stache_parse (CStacheTemplate *tpl, const char *text, size_t length);
void c_stache_render(const CStacheTemplate *tpl, CStacheCallbacks *cbs);

