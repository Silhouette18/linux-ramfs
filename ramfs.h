#ifndef RAMFS_H
#define RAMFS_H

#include <linux/fs.h>
#include <linux/list.h>

struct extended_inode {
   struct inode standart_inode;
   struct list_head head;
   char *data;
   size_t size;
};

struct write_directory {
    char name[32];
    int len;
    u64 inode_number;
    struct list_head list;
};

struct fs_superblock {
    unsigned long max_memory;
    unsigned long in_use;
    unsigned long magic_number;
};

extern const struct inode_operations inode_operations_dir;
extern const struct file_operations file_operations_dir;

extern const struct inode_operations inode_operations_file;
extern const struct file_operations file_operations_file;

#endif
