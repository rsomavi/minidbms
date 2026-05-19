#ifndef BTREE_H
#define BTREE_H

#include "page.h"

#define BTREE_ORDER 100

#define BTREE_INTERNAL 0
#define BTREE_LEAF     1

typedef struct {
    int type;
    int n_keys;
    int keys[BTREE_ORDER];
    int children[BTREE_ORDER + 1];
    int row_ids[BTREE_ORDER];
    int next_leaf;
} BTreeNode;

typedef struct {
    int root_page_id;
    int n_entries;
    int height;
    int key_type;
    int next_free_page;
} BTreeMeta;

int btree_open(const char *data_dir, const char *table_name);
int btree_read_meta(int fd, BTreeMeta *meta);
int btree_write_meta(int fd, const BTreeMeta *meta);
int btree_read_node(int fd, int page_id, BTreeNode *node);
int btree_write_node(int fd, int page_id, const BTreeNode *node);
int btree_alloc_page(int fd, BTreeMeta *meta);
int btree_init(const char *data_dir, const char *table_name);
int btree_search(int fd, const BTreeMeta *meta, int key);

#endif /* BTREE_H */
