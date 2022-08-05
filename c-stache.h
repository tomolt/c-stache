#include <stddef.h>

typedef struct c_stache_model    CStacheModel;
typedef struct c_stache_sink     CStacheSink;
typedef struct c_stache_tag      CStacheTag;
typedef struct c_stache_template CStacheTemplate;
typedef struct c_stache_engine   CStacheEngine;

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
	const char *name;
	CStacheTag *tags;
	size_t      numTags;
	size_t      capTags;
	const char *text;
	size_t      length;
};

struct c_stache_engine {
	char *(*read)(const char *name, size_t *length);

	CStacheTemplate *templates;
	size_t numTemplates;
	size_t capTemplates;
};

size_t c_stache_escape_xml(const char **text, char *buf, size_t max);
int    c_stache_parse (CStacheTemplate *tpl, const char *text, size_t length);
void   c_stache_render(const CStacheTemplate *tpl, CStacheModel *model, CStacheSink *sink);

char *c_stache_read_file(const char *name, size_t *length);

void c_stache_start_engine(CStacheEngine *engine, char *(*read)(const char *name, size_t *length));
void c_stache_shutdown_engine(CStacheEngine *engine);
const CStacheTemplate *c_stache_load_template(CStacheEngine *engine, const char *name);

