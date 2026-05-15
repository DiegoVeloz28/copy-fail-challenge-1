#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "copyfail_dev"

static int device_open(struct inode *inode, struct file *file) { return 0; }
static int device_release(struct inode *inode, struct file *file) { return 0; }

static ssize_t device_write(struct file *filp, const char __user *buffer, size_t len, loff_t *off) {
    char kernel_buffer[100];
    // Vulnerabilidad: no se valida el retorno
    copy_from_user(kernel_buffer, buffer, len); 
    printk(KERN_INFO "CopyFail: Datos recibidos\n");
    return len;
}

static struct file_operations fops = {
    .write = device_write,
    .open = device_open,
    .release = device_release,
};

int init_module(void) {
    register_chrdev(240, DEVICE_NAME, &fops);
    printk(KERN_INFO "Modulo vulnerable cargado en /dev/%s\n", DEVICE_NAME);
    return 0;
}

void cleanup_module(void) {
    unregister_chrdev(240, DEVICE_NAME);
}

MODULE_LICENSE("GPL");
