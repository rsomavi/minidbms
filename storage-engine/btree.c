#include <fcntl.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "btree.h"

#define BTREE_KEY_TYPE_INT 0

static int btree_page_offset(int page_id, off_t *offset) {
    if (page_id < 0 || offset == NULL) {
        return -1;
    }

    *offset = (off_t)page_id * PAGE_SIZE;
    return 0;
}

static int btree_write_page(int fd, int page_id, const char *buffer) {
    off_t offset;
    ssize_t bytes_written;

    if (fd < 0 || buffer == NULL || btree_page_offset(page_id, &offset) != 0) {
        return -1;
    }

    bytes_written = pwrite(fd, buffer, PAGE_SIZE, offset);
    if (bytes_written != PAGE_SIZE) {
        return -1;
    }

    return 0;
}

static int btree_read_page(int fd, int page_id, char *buffer) {
    off_t offset;
    ssize_t bytes_read;

    if (fd < 0 || buffer == NULL || btree_page_offset(page_id, &offset) != 0) {
        return -1;
    }

    bytes_read = pread(fd, buffer, PAGE_SIZE, offset);
    if (bytes_read != PAGE_SIZE) {
        return -1;
    }

    return 0;
}

static int btree_build_path(char *path, size_t path_size, const char *data_dir, const char *table_name) {
    int written;

    if (path == NULL || data_dir == NULL || table_name == NULL) {
        return -1;
    }

    written = snprintf(path, path_size, "%s/%s.idx", data_dir, table_name);
    if (written < 0 || (size_t)written >= path_size) {
        return -1;
    }

    return 0;
}

int btree_open(const char *data_dir, const char *table_name) {
    char path[PATH_MAX];

    if (btree_build_path(path, sizeof(path), data_dir, table_name) != 0) {
        return -1;
    }

    return open(path, O_RDWR | O_CREAT, 0644);
}

int btree_read_meta(int fd, BTreeMeta *meta) {
    char buffer[PAGE_SIZE];
    size_t offset = 0;

    if (meta == NULL || btree_read_page(fd, 0, buffer) != 0) {
        return -1;
    }

    memcpy(&meta->root_page_id, buffer + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&meta->n_entries, buffer + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&meta->height, buffer + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&meta->key_type, buffer + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&meta->next_free_page, buffer + offset, sizeof(int));

    return 0;
}

int btree_write_meta(int fd, const BTreeMeta *meta) {
    char buffer[PAGE_SIZE];
    size_t offset = 0;

    if (fd < 0 || meta == NULL) {
        return -1;
    }

    memset(buffer, 0, sizeof(buffer));

    memcpy(buffer + offset, &meta->root_page_id, sizeof(int));
    offset += sizeof(int);
    memcpy(buffer + offset, &meta->n_entries, sizeof(int));
    offset += sizeof(int);
    memcpy(buffer + offset, &meta->height, sizeof(int));
    offset += sizeof(int);
    memcpy(buffer + offset, &meta->key_type, sizeof(int));
    offset += sizeof(int);
    memcpy(buffer + offset, &meta->next_free_page, sizeof(int));

    return btree_write_page(fd, 0, buffer);
}

int btree_read_node(int fd, int page_id, BTreeNode *node) {
    char buffer[PAGE_SIZE];
    size_t offset = 0;

    if (node == NULL || btree_read_page(fd, page_id, buffer) != 0) {
        return -1;
    }

    memcpy(&node->type, buffer + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&node->n_keys, buffer + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(node->keys, buffer + offset, sizeof(node->keys));
    offset += sizeof(node->keys);
    memcpy(node->children, buffer + offset, sizeof(node->children));
    offset += sizeof(node->children);
    memcpy(node->row_ids, buffer + offset, sizeof(node->row_ids));
    offset += sizeof(node->row_ids);
    memcpy(&node->next_leaf, buffer + offset, sizeof(int));

    return 0;
}

int btree_write_node(int fd, int page_id, const BTreeNode *node) {
    char buffer[PAGE_SIZE];
    size_t offset = 0;

    if (fd < 0 || page_id <= 0 || node == NULL) {
        return -1;
    }

    memset(buffer, 0, sizeof(buffer));

    memcpy(buffer + offset, &node->type, sizeof(int));
    offset += sizeof(int);
    memcpy(buffer + offset, &node->n_keys, sizeof(int));
    offset += sizeof(int);
    memcpy(buffer + offset, node->keys, sizeof(node->keys));
    offset += sizeof(node->keys);
    memcpy(buffer + offset, node->children, sizeof(node->children));
    offset += sizeof(node->children);
    memcpy(buffer + offset, node->row_ids, sizeof(node->row_ids));
    offset += sizeof(node->row_ids);
    memcpy(buffer + offset, &node->next_leaf, sizeof(int));

    return btree_write_page(fd, page_id, buffer);
}

int btree_alloc_page(int fd, BTreeMeta *meta) {
    char buffer[PAGE_SIZE];
    int page_id;

    if (fd < 0 || meta == NULL || meta->next_free_page <= 0) {
        return -1;
    }

    page_id = meta->next_free_page;
    meta->next_free_page++;

    memset(buffer, 0, sizeof(buffer));
    if (btree_write_page(fd, page_id, buffer) != 0) {
        meta->next_free_page--;
        return -1;
    }

    if (btree_write_meta(fd, meta) != 0) {
        meta->next_free_page--;
        return -1;
    }

    return page_id;
}

int btree_init(const char *data_dir, const char *table_name) {
    char path[PATH_MAX];
    BTreeMeta meta;
    BTreeNode root;
    int fd;

    if (btree_build_path(path, sizeof(path), data_dir, table_name) != 0) {
        return -1;
    }

    fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        return -1;
    }

    meta.root_page_id = 1;
    meta.n_entries = 0;
    meta.height = 1;
    meta.key_type = BTREE_KEY_TYPE_INT;
    meta.next_free_page = 2;

    memset(&root, 0, sizeof(root));
    root.type = BTREE_LEAF;
    root.n_keys = 0;
    root.next_leaf = -1;

    if (btree_write_meta(fd, &meta) != 0 || btree_write_node(fd, meta.root_page_id, &root) != 0) {
        close(fd);
        return -1;
    }

    if (close(fd) != 0) {
        return -1;
    }

    return 0;
}

int btree_search(int fd, const BTreeMeta *meta, int key) {
    BTreeNode node;
    int current_page_id;

    if (fd < 0 || meta == NULL || meta->root_page_id <= 0 || meta->key_type != BTREE_KEY_TYPE_INT) {
        return -1;
    }

    current_page_id = meta->root_page_id;

    while (1) {
        int index;

        if (btree_read_node(fd, current_page_id, &node) != 0) {
            return -1;
        }

        if (node.type == BTREE_LEAF) {
            for (index = 0; index < node.n_keys; index++) {
                if (node.keys[index] == key) {
                    return node.row_ids[index];
                }
            }
            return -1;
        }

        if (node.type != BTREE_INTERNAL) {
            return -1;
        }

        index = 0;
        while (index < node.n_keys && key >= node.keys[index]) {
            index++;
        }

        current_page_id = node.children[index];
        if (current_page_id <= 0) {
            return -1;
        }
    }
}
