/***************************************************************************************************//**
*  \file       gpio21_driver.c
*
*  \details    GPIO 21 device driver
*
*  \author     Frank, (refer from EmbedTronic)
*
*  \board      Linux raspberrypi 5.15.91-v8+
*
******************************************************************************************************/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>       
#include <linux/gpio.h>
#include <linux/err.h>

/* LED is connected to below GPIO pin */
#define GPIO_21 (21)

/* print */
#undef pr_fmt
#define pr_fmt(fmt) "@frk-gpio21_driver: [%s] :" fmt,__func__

/* device numbers, class, char device*/
dev_t dev = 0;
static struct class *dev_class;
static struct cdev gpio21_cdev;

/* module functions */
static int  __init gpio21_driver_init(void);
static void __exit gpio21_driver_exit(void);

/* fops functions  */
static int     gpio21_open(struct inode *inode, struct file *file);
static int     gpio21_release(struct inode *inode, struct file *file);
static ssize_t gpio21_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t gpio21_write(struct file *filp, const char *buf, size_t len, loff_t * off);

/* fops */
static struct file_operations fops =
{
    .owner     = THIS_MODULE,
    .read      = gpio21_read,
    .write     = gpio21_write,
    .open      = gpio21_open,
    .release   = gpio21_release,
};

/******************************************************************************************************/
/* fops implementations */

/* This function of read the Device file */
static ssize_t gpio21_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    /* */
    pr_info("@frk: come to [gpio21_read]\n");
    uint8_t gpio_state = 0;

    /* read GPIO value */
    gpio_state = gpio_get_value(GPIO_21);

    /* write to user */
    len = 1;
    if( copy_to_user(buf, &gpio_state, len) > 0){
        pr_err("ERROR: Not all bytes have been copied!!!\n");
    }

    pr_info("Read function: GPIO_21 value = [%d]\n", gpio_state);

    return 0;

}

/* This function of write the Device file */
static ssize_t gpio21_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    pr_info("@frk: come to [gpio21_write]\n");
    uint8_t rec_buf[10] = {0};

    if(copy_from_user(rec_buf, buf, len) > 0){
        pr_err("ERROR: Not all the bytes have been copied from user \n");
    }

    if(rec_buf[0] == '1'){
        // set GPIO value to HIGH
        gpio_set_value(GPIO_21, 1);
    } else if (rec_buf[0] == '0') {
        // set GPIO value to LOW
        gpio_set_value(GPIO_21, 0);
    } else {
        pr_err("Unknown command: please provide either 0 or 1\n");
    }

    pr_info("Write Function\n");

    return len;
}

/* This function of open the Device file */ 
static int gpio21_open(struct inode *inode, struct file *file)
{
    pr_info("Device File Opened...!!!\n");
    return 0;
}

/* This function of close the Device file */ 
static int gpio21_release(struct inode *inode, struct file *file)
{
    pr_info("Device File Closed...!!!\n");
    return 0;
}

/******************************************************************************************************/
/* module init func */
static int __init gpio21_driver_init(void)
{
    /* Allocating Major number */
    if((alloc_chrdev_region(&dev, 0, 1, "gpio21_dev")) <0){
        pr_info("Cannot allocate major number\n");
        goto r_unreg;
    }

    pr_info("Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));

    /* Creating cdev structure */
    cdev_init(&gpio21_cdev,&fops);

    /* Adding character device to the system */
    if((cdev_add(&gpio21_cdev,dev,1)) < 0){
      pr_info("Cannot add the device to the system\n");
      goto r_del;
    }

    /* Create class */
    if(IS_ERR(dev_class = class_create(THIS_MODULE,"gpio21_class"))){
      pr_info("Cannot create the struct class\n");
      goto r_class;
    }

    /* Creating device */
    if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"gpio21_device"))){
      pr_info("Cannot create the Device 1\n");
      goto r_device;
    }
/*
** GPIO steps
*/
    /* Checking GPIO is valid or not! */
    if(gpio_is_valid(GPIO_21) == false){
        pr_err("GPIO[%d] is not valid.\n", GPIO_21);
        goto r_device;
    }

    /* Requesting GPIO */
    if(gpio_request(GPIO_21, "GPIO_21") < 0){
        pr_err("ERROR: GPIO %d request error.\n", GPIO_21);
        goto r_gpio;
    }

    /* Configure GPIO as output */
    gpio_direction_output(GPIO_21, 0);

    /* for debuging purpose */
    gpio_export(GPIO_21, false);

    pr_info("Device Driver Insert...Done!!!\n");
    return 0;

r_gpio:
    gpio_free(GPIO_21);
r_device:
    device_destroy(dev_class, dev);
r_class:
    class_destroy(dev_class);
r_del:
    cdev_del(&gpio21_cdev);
r_unreg:
    unregister_chrdev_region(dev,1);
 
    return -1;
}

/* module exit func*/
static void __exit gpio21_driver_exit(void)
{
    gpio_unexport(GPIO_21);
    gpio_free(GPIO_21);
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&gpio21_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Device Driver Remove...Done!!!\n");
}

/******************************************************************************************************/
module_init(gpio21_driver_init);
module_exit(gpio21_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("FRANK <frank@bos-semi.com>");
MODULE_DESCRIPTION("GPIO21 DRIVER");
MODULE_VERSION("1.8");

/******************************************************************************************************/

