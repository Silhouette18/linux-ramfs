#include "ramfs.h"
#include <linux/list.h>
#include <linux/fs.h>

int make_directory(struct user_namespace *mnt_userns, struct inode *parent_inode, struct dentry *dentry, umode_t mode) {
    struct inode *inode = new_inode(parent_inode->i_sb);
    unsigned int next_ino = get_next_ino();
    inode->i_ino = next_ino;
    insert_inode_hash(inode);
    
    inode->i_mode = S_IFDIR | mode;
    inode->i_op = &inode_operations_dir;
    inode->i_fop = &file_operations_dir; 
    
    inc_nlink(inode);
    
    struct write_directory *write_directory = kmalloc(sizeof(struct write_directory), GFP_KERNEL);
    
    write_directory->len = dentry->d_name.len;
    strncpy(write_directory->name, dentry->d_name.name, 31);
    write_directory->inode_number = inode->i_ino;
    
    struct extended_inode *parent_ext = container_of(parent_inode, struct extended_inode, standart_inode);
    list_add(&write_directory->list, &parent_ext->head);
    
    d_instantiate(dentry, inode);
    
    dget(dentry);
    
    inc_nlink(parent_inode);
    return 0;
}

int make_file(struct user_namespace *mnt_userns, struct inode *parent_inode, struct dentry *dentry, umode_t mode, bool is_exist) {
    struct inode *inode = new_inode(parent_inode->i_sb);
    unsigned int next_ino = get_next_ino();
    inode->i_ino = next_ino;
    insert_inode_hash(inode);
    
    inode->i_mode = S_IFREG | mode;
    inode->i_op = &inode_operations_file;
    inode->i_fop = &file_operations_file; 
    
    struct write_directory *write_directory = kmalloc(sizeof(struct write_directory), GFP_KERNEL);
    
    write_directory->len = dentry->d_name.len;
    strncpy(write_directory->name, dentry->d_name.name, 31);
    write_directory->inode_number = inode->i_ino;
    
    struct extended_inode *parent_ext = container_of(parent_inode, struct extended_inode, standart_inode);
    list_add(&write_directory->list, &parent_ext->head);
    
    d_instantiate(dentry, inode);
    
    dget(dentry);
    
    return 0;
}

struct dentry *lookup(struct inode *parent_inode, struct dentry *dentry, unsigned int flags) {
     struct extended_inode *parent_ext = container_of(parent_inode, struct extended_inode, standart_inode);
     
     struct inode *found_inode = NULL;
     struct write_directory *cursor;
     
     list_for_each_entry(cursor, &parent_ext->head, list) {
         if (cursor->len == dentry->d_name.len && strncmp(cursor->name, dentry->d_name.name, cursor->len) == 0) {
             found_inode = ilookup(parent_inode->i_sb, cursor->inode_number);     
             break;
         } 
     }
     d_splice_alias(found_inode, dentry);
     return NULL;
}

int ramfs_iterate_shared(struct file *file, struct dir_context *ctx) {
    int i = 2;

       
    struct inode *inode = file_inode(file);
    
    struct extended_inode *ext_inode = container_of(inode, struct extended_inode, standart_inode);
    struct write_directory *cursor;
    
     if (!dir_emit_dots(file, ctx))
       return 0;
    
    list_for_each_entry(cursor, &ext_inode->head, list) {
        if (i < ctx->pos) {
            i++;
            continue;
        }
        if (!dir_emit(ctx, cursor->name, cursor->len, cursor->inode_number, DT_UNKNOWN))
            break;
       ctx->pos++;
       i++;
    }
    return 0;
}

const struct inode_operations inode_operations_dir = {
   .create = make_file,
   .lookup = lookup,
   .mkdir = make_directory,
};

const struct file_operations file_operations_dir = {
    .iterate_shared = ramfs_iterate_shared,
};


