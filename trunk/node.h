#ifndef _NODE_H
#define _NODE_H

#include "dict.h"
#include "handler.h"

typedef struct node node_t;

extern NOFAIL node_t *node_create(str_t *fs_path, handler_t *default_handler);
extern void node_reg_child(node_t *node, str_t *name, node_t *child);
extern void node_reg_handler(node_t *node, str_t *ext, handler_t *handler);
extern NOFAIL handler_t *node_get_handler(node_t *node, str_t *ext);

#endif
