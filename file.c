#include "ramfs.h"
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>



ssize_t ramfs_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    size_t bytes_to_write;
  
    struct inode *inode = file_inode(file);
    
    struct super_block *sb = inode->i_sb;
    struct fs_superblock *fs_sb = sb->s_fs_info; 
    
    struct extended_inode *ext_inode = container_of(inode, struct extended_inode, standart_inode);
    
     size_t old_size = ext_inode->size;
    
    bytes_to_write = *ppos + count;
    
    if (bytes_to_write > ext_inode->size) {
       void *temp_data;
       temp_data = krealloc(ext_inode->data, bytes_to_write, GFP_KERNEL);
       if (temp_data == NULL)
          return -ENOMEM;
        
       ext_inode->data = temp_data;
       ext_inode->size = bytes_to_write;
    }
    
    if (copy_from_user(ext_inode->data + *ppos, buf, count)) {
      pr_err("Failed to copy from user space\n");
      return -EFAULT;
    }
    *ppos = *ppos + count;
    
    fs_sb->in_use += (ext_inode->size - old_size);
    return count;
}


ssize_t ramfs_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
     size_t bytes_to_read;
   
     struct inode *inode = file_inode(file);
     
     struct extended_inode *ext_inode = container_of(inode, struct extended_inode, standart_inode);
     
     bytes_to_read = count;
     
     if (*ppos >= ext_inode->size) {
         return 0;
     }
     
     if (*ppos + count > ext_inode->size) {
       bytes_to_read = ext_inode->size - *ppos;
     }
     
     if (copy_to_user(buf, ext_inode->data + *ppos, bytes_to_read)) {
        return -EFAULT;
     }
     
     *ppos = *ppos + bytes_to_read;
     
     return bytes_to_read;
}

const struct inode_operations inode_operations_file = {
    // Inode doesn't need this
};

const struct file_operations file_operations_file = {
    .write = ramfs_write,
    .read = ramfs_read,
    .llseek = default_llseek,
};

