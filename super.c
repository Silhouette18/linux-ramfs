#include "ramfs.h"
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/statfs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Silhouette");
MODULE_DESCRIPTION("Custom file system in RAM");

struct kmem_cache *inode_cache;

static const struct super_operations s_o;

void ramfs_evict_inode(struct inode *inode) {
    struct extended_inode *ext_inode = container_of(inode, struct extended_inode, standart_inode);

    struct super_block *sb = inode->i_sb;
    struct fs_superblock *fs_sb = sb->s_fs_info; 
    
    fs_sb->in_use -= ext_inode->size;

    kfree(ext_inode->data);
    ext_inode->size = 0;
    clear_inode(inode);
}

struct inode *alloc_inode(struct super_block *sb) {
    struct extended_inode *mem_extended_inode = kmem_cache_alloc(inode_cache, GFP_KERNEL);
    if (mem_extended_inode == NULL) {
       return NULL;
    }
    INIT_LIST_HEAD(&mem_extended_inode->head);
    return &mem_extended_inode->standart_inode;
}

void destroy_inode(struct inode *inode) {
    struct extended_inode *mem_extended_inode = container_of(inode, struct extended_inode, standart_inode);
    kmem_cache_free(inode_cache, mem_extended_inode);
}

int fill_super_call(struct super_block *superblock, void *data, int silent) {
    struct fs_superblock *fs_superblock;
    struct inode *root_inode;
    struct dentry *root;

    superblock->s_blocksize = PAGE_SIZE;
    superblock->s_magic = 17;
    superblock->s_maxbytes = 100000000;
    superblock->s_op = &s_o;
    
    fs_superblock = kmalloc(sizeof(struct fs_superblock), GFP_KERNEL);
    fs_superblock->max_memory = 1000000000;
    fs_superblock->in_use = 0;
    superblock->s_fs_info = fs_superblock;
    
    root_inode = new_inode(superblock);
    root_inode->i_mode = S_IFDIR | 0755;
    root_inode->i_op = &inode_operations_dir;
    root_inode->i_fop = &file_operations_dir; 
    
    root = d_make_root(root_inode);
    if (!root) {
       return -ENOMEM;
    }
    superblock->s_root = root;
    return 0;
}

struct dentry *ramfs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data) {
     return mount_nodev(fs_type, flags, data, fill_super_call);
}

void ramfs_inode_init_once(void *ptr) {
     struct extended_inode *ext_inode = (struct extended_inode *)ptr;
     
     inode_init_once(&ext_inode->standart_inode);
}

int ramfs_statfs(struct dentry *dentry, struct kstatfs *buf) {
    struct super_block *sb = dentry->d_sb;
    struct fs_superblock *fs_sb = sb->s_fs_info;
    
    buf->f_type = sb->s_magic;
    buf->f_bsize = sb->s_blocksize;
    
    buf->f_blocks = fs_sb->max_memory / sb->s_blocksize;
    buf->f_bfree  = (fs_sb->max_memory - fs_sb->in_use) / sb->s_blocksize;
    
    buf->f_bavail = buf->f_bfree;
    buf->f_namelen = 31;
    
    return 0;
}

static struct file_system_type ramfs_type = {
     .owner = THIS_MODULE,
     .name = "my_ramfs",
     .mount = ramfs_mount,
     .kill_sb = kill_litter_super,
};

static const struct super_operations s_o = {
  .statfs = ramfs_statfs,
  .alloc_inode = alloc_inode,
  .destroy_inode = destroy_inode,
  .evict_inode = ramfs_evict_inode,
};

int __init ramfs_init(void) {
    int ret;

    inode_cache = kmem_cache_create("ramfs_inode", sizeof(struct extended_inode), 0, 0, ramfs_inode_init_once);
    if (!inode_cache) {
        return -ENOMEM;
    }
    
    ret = register_filesystem(&ramfs_type);
    
    if (ret) {
       kmem_cache_destroy(inode_cache);
       return ret;
    }
   
    return 0;
}

void __exit ramfs_exit(void) {
     unregister_filesystem(&ramfs_type);
     kmem_cache_destroy(inode_cache);
}
  
module_init(ramfs_init);
module_exit(ramfs_exit);
