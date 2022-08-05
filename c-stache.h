#include <stddef.h>

typedef struct c_stache_model    CStacheModel;
typedef struct c_stache_sink     CStacheSink;
typedef struct c_stache_tag      CStacheTag;
typedef struct c_stache_template CStacheTemplate;
typedef struct c_stache_engine   CStacheEngine;

struct c_stache_model {
	int         (*enter)(void *userptr, const char *section);
	int         (*next )(void *userptr);
	int         (*empty)(void *userptr, const char *section);
	const char *(*subst)(void *userptr, const char *key);

	void *userptr;
};

struct c_stache_sink {
	size_t (*escape)(const char **text, char *buf, size_t max);
	int    (*write )(void *userptr, const char *text, size_t length);

	void *userptr;
};

struct c_stache_tag {
	CStacheTag    *buddy;
	const CStacheTemplate *otherTpl;
	const char    *pointer;
	unsigned short keyStart;
	unsigned short keyLength;
	unsigned short tagLength;
	char           kind;
};

struct c_stache_template {
	const char  *name;
	CStacheTag  *tags;
	size_t       numTags;
	size_t       capTags;
	const char  *text;
	size_t       length;
	unsigned int refcount;
};

struct c_stache_engine {
	char *(*read)(const char *name, size_t *length);

	CStacheTemplate **templates;
	size_t numTemplates;
	size_t capTemplates;
};

void c_stache_start_engine   (CStacheEngine *engine, char *(*read)(const char *name, size_t *length));
void c_stache_shutdown_engine(CStacheEngine *engine);

const CStacheTemplate *c_stache_load_template(CStacheEngine *engine, const char *name);
void                   c_stache_drop_template(CStacheEngine *engine, CStacheTemplate *tpl);

void c_stache_render(const CStacheTemplate *tpl, CStacheModel *model, CStacheSink *sink);

char *c_stache_read_file(const char *name, size_t *length);

size_t c_stache_escape_xml(const char **text, char *buf, size_t max);

