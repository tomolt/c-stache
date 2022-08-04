#include <stddef.h>

typedef struct c_stache_model     CStacheModel;
typedef struct c_stache_sink      CStacheSink;
typedef struct c_stache_tag       CStacheTag;
typedef struct c_stache_template  CStacheTemplate;

struct c_stache_model {
	int (*enter)(void *userdata, const char *section);
	int (*next )(void *userdata);
	int (*empty)(void *userdata, const char *section);
	const char *(*subst)(void *userdata, const char *key);

	void *userdata;
};

struct c_stache_sink {
	size_t (*escape)(const char **text, char *buf, size_t max);
	int (*write)(void *userdata, const char *text, size_t length);

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

size_t c_stache_escape_xml(const char **text, char *buf, size_t max);
int    c_stache_parse (CStacheTemplate *tpl, const char *text, size_t length);
void   c_stache_render(const CStacheTemplate *tpl, CStacheModel *model, CStacheSink *sink);

