/*
 * =====================================================================================
 *
 *       Filename:  helloworld.c
 *
 *    Description:  helloworld_mod
 *
 *        Version:  1.0
 *        Created:  11/10/2017 10:46:27 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Duan Juntao (atomduan@gmail.com), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/semaphore.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/string.h>
#include "helloworld_mod_ioctl.h"

#define MAJOR_NUM 250
#define MINOR_NUM 0
#define IN_BUF_LEN 256
#define OUT_BUF_LEN 512

MODULE_AUTHOR("Tishion");
MODULE_DESCRIPTION("Hello_mod driver by tishion");

static struct class * helloworld_class;
static struct cdev helloworld_cdev;
static dev_t devnum = 0;
static char * modname = "helloworld_mod";
static char * devicename = "helloworld";
static char * classname = "helloworld_class";

static int open_count = 0;
static struct semaphore sem;
static DEFINE_SPINLOCK(spin);
static char * inbuffer = NULL;
static char * outbuffer = NULL;
static lang_t langtype;

static int helloworld_mod_open(struct inode *, struct file *);
static int helloworld_mod_release(struct inode *, struct file *);
static ssize_t helloworld_mod_read(struct file *, char *, size_t, loff_t *);
static ssize_t helloworld_mod_write(struct file *, const char *, size_t, loff_t *);
static long helloworld_mod_ioctl(struct file *, unsigned int, unsigned long);

struct file_operations helloworld_mod_fops = 
{
    .owner = THIS_MODULE,
    .open = helloworld_mod_open,
    .read = helloworld_mod_read,
    .write = helloworld_mod_write,
    .unlocked_ioctl = helloworld_mod_ioctl,
    .release = helloworld_mod_release,
};

static int 
helloworld_mod_open(struct inode *inode, struct file *pfile)
{
    printk("+helloworld_mod_open()!/n");
    spin_lock(&spin);
    if(open_count)
    {
        spin_unlock(&spin);
        return -EBUSY;
    }
    open_count++;
    spin_unlock(&spin);
    printk("-helloworld_mod_open()!/n");
    return 0;
}

static int 
helloworld_mod_release(struct inode *inode, struct file *pfile)
{
    printk("+helloworld_mod_release()!/n");
    open_count--;
    printk("-helloworld_mod_release()!/n");
    return 0;
}

static ssize_t 
helloworld_mod_read(struct file *pfile, char *user_buf, size_t len, loff_t *off)
{
    printk("+helloworld_mod_read()!/n");

    if(down_interruptible(&sem))
    {
        return -ERESTARTSYS; 
    }
    memset(outbuffer, 0, OUT_BUF_LEN);
    printk("    +switch()/n");
    switch(langtype)
    {
        case english:
            printk("        >in case: english/n");
            sprintf(outbuffer, "Hello! %s.", inbuffer);
            break;
        case chinese:
            printk("        >in case: chinese/n");
            sprintf(outbuffer, "你好！ %s.", inbuffer);
            break;
        case pinyin:
            printk("        >in case: pinyin/n");
            sprintf(outbuffer, "ni hao! %s.", inbuffer);
            break;
        default:
            printk("        >in case: default/n");
            break;
    }
    printk("    -switch()/n");
    if(copy_to_user(user_buf, outbuffer, len))
    {
        up(&sem);
        return -EFAULT;
    }
    up(&sem);
    printk("-helloworld_mod_read()!/n");
    return 0;
}

static ssize_t 
helloworld_mod_write(struct file *pfile, const char *user_buf, size_t len, loff_t *off)
{
    printk("+helloworld_mod_write()!/n");
    if(down_interruptible(&sem))
    {
        return -ERESTARTSYS;
    }
    if(len > IN_BUF_LEN)
    {
        printk("Out of input buffer/n");
        return -ERESTARTSYS;
    }
    if(copy_from_user(inbuffer, user_buf, len))
    {
        up(&sem);
        return -EFAULT;
    }
    up(&sem);    
    printk("-helloworld_mod_write()!/n");
    return 0;
}

static long 
helloworld_mod_ioctl(struct file *pfile, unsigned int cmd, unsigned long arg)
{
    int err = 0;
    printk("+helloworld_mod_ioctl()!/n");
    printk("    +switch()/n");
    switch(cmd)
    {
        case HELLOWORLD_IOCTL_RESETLANG:
            printk("        >in case: HELLOWORLD_IOCTL_RESETLANG/n");
            langtype = english;
            break;
        case HELLOWORLD_IOCTL_GETLANG:
            printk("        >in case: HELLOWORLD_IOCTL_GETLANG/n");
            err = copy_to_user((int *)arg, &langtype, sizeof(int));
            break;
        case HELLOWORLD_IOCTL_SETLANG:
            printk("        >in case: HELLOWORLD_IOCTL_SETLANG/n");
            err = copy_from_user(&langtype,(int *)arg, sizeof(int));
            break;
        default:
            printk("        >in case: default/n");
            err = ENOTSUPP;
            break;
    }
    printk("    -switch()/n");
    printk("-helloworld_mod_ioctl()!/n");
    return err;
}

static int __init 
helloworld_mod_init(void)
{
    int result;
    printk("+helloworld_mod_init()!/n");
    devnum = MKDEV(MAJOR_NUM, MINOR_NUM);
    result = register_chrdev_region(devnum, 1, modname);

    if(result < 0)
    {
        printk("helloworld_mod : can't get major number!/n");
        return result;
    }    

    cdev_init(&helloworld_cdev, &helloworld_mod_fops);
    helloworld_cdev.owner = THIS_MODULE;
    helloworld_cdev.ops = &helloworld_mod_fops;
    result = cdev_add(&helloworld_cdev, devnum, 1);
    if(result)
        printk("Failed at cdev_add()");
    helloworld_class = class_create(THIS_MODULE, classname);
    if(IS_ERR(helloworld_class))
    {
        printk("Failed at class_create().Please exec [mknod] before operate the device/n");
    }
    else
    {
        device_create(helloworld_class, NULL, devnum,NULL, devicename);
    }

    open_count = 0;
    langtype = english;
    inbuffer = (char *)kmalloc(IN_BUF_LEN, GFP_KERNEL);
    outbuffer = (char *)kmalloc(OUT_BUF_LEN, GFP_KERNEL);
    sema_init(&sem, 1);
    printk("-helloworld_mod_init()!/n");
    return 0;
}

static void __exit 
helloworld_mod_exit(void)
{
    printk("+helloworld_mod_exit!/n");
    kfree(inbuffer);
    kfree(outbuffer);
    cdev_del(&helloworld_cdev);
    device_destroy(helloworld_class, devnum);
    class_destroy(helloworld_class);
    unregister_chrdev_region(devnum, 1);
    printk("-helloworld_mod_exit!/n");
    return ;
}

module_init(helloworld_mod_init);
module_exit(helloworld_mod_exit);
MODULE_LICENSE("GPL");
