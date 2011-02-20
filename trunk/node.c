#include "node.h"
#include "mem.h"
#include "dict.h"
#include "str.h"

struct node {
	dict_t children;
	str_t *fs_path;
	dict_t ext_handlers;
	handler_t *default_handler;
};

NOFAIL node_t *node_create(str_t *fs_path, handler_t *default_handler)
{
	node_t *node = mem_alloc(sizeof(node_t));

	dict_init(&node->children);
	node->fs_path = str_dup(fs_path);
	dict_init(&node->ext_handlers);
	node->default_handler = default_handler;

	return node;
}

void node_reg_child(node_t *node, str_t *name, node_t *child)
{
	dict_add_ptr(&node->children, str_dup(name), child);
}

void node_reg_handler(node_t *node, str_t *ext, handler_t *handler)
{
	dict_add_ptr(&node->ext_handlers, str_dup(ext), handler);
}

NOFAIL handler_t *node_get_handler(node_t *node, str_t *ext)
{
	handler_t *handler = node->default_handler;
	void **value = dict_query_ptr(&node->ext_handlers, ext, 0);

	if (value != NULL)
		handler = *value;

	return handler;
}
