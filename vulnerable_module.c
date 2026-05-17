#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cred.h>

#define DEVICE_NAME "copyfail_dev"

static int device_open(struct inode *inode, struct file *file) { return 0; }
static int device_release(struct inode *inode, struct file *file) { return 0; }

static ssize_t device_write(struct file *filp, const char __user *buffer, size_t len, loff_t *off) {
    char kernel_buffer[100];
    
    // Evitamos desbordamientos de memoria al copiar
    if (len > 99) len = 99;

    // Vulnerabilidad: copia sin validar el retorno
    copy_from_user(kernel_buffer, buffer, len);
    kernel_buffer[len] = '\0'; // Aseguramos el fin de cadena

    // Si la entrada contiene la palabra clave, disparamos la escalada real
    if (strstr(kernel_buffer, "explotar") != NULL) {
        commit_creds(prepare_kernel_cred(NULL));
        printk(KERN_INFO "CopyFail: Escalada ejecutada. Privilegios elevados a ROOT.\n");
    } else {
        printk(KERN_INFO "CopyFail: Datos recibidos\n");
    }

    return len;
}

static struct file_operations fops = {
    .write = device_write,
    .open = device_open,
    .release = device_release,
};

// Declara esta variable global arriba, fuera de cualquier función
int major_num;

int init_module(void) {
    // Al pasarle 0, el kernel busca un número mayor libre automáticamente
    major_num = register_chrdev(0, DEVICE_NAME, &fops);
    
    if (major_num < 0) {
        printk(KERN_ALERT "CopyFail: Error al registrar el dispositivo\n");
        return major_num;
    }
    
    printk(KERN_INFO "Modulo vulnerable cargado con Major: %d\n", major_num);
    return 0;
}

void cleanup_module(void) {
    // Liberamos el dispositivo usando la variable dinámica correspondiente
    unregister_chrdev(major_num, DEVICE_NAME);
}
MODULE_LICENSE("GPL");
