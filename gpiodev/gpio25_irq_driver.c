/***************************************************************************************************//**
*  \file       gpio25_irq_driver.c
*
*  \details    GPIO 25 IRQ device driver
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
#include <linux/interrupt.h>

#define EN_DEBOUNCE
#ifdef EN_DEBOUNCE
    #include <linux/jiffies.h>
    extern unsigned long volatile jiffies;
    unsigned long old_jiffie = 0;
#endif

/* LED is connected to below GPIO pin */
#define GPIO_21_OUT (21)

/* GPIO interupt input pin (GPIO25)*/
#define GPIO_25_IN (25)

/* print */
#undef pr_fmt
#define pr_fmt(fmt) "@frk-gpioirq_driver: [%s] :" fmt,__func__

/* device numbers, class, char device*/
dev_t dev = 0;
static struct class *dev_class;
static struct cdev gpioirq_cdev;

/* interupts number */
unsigned int gpio_irq_num;

/* interupts handler */
static irqreturn_t gpio_irq_handler(int irq, void *dev_id);

unsigned int led_toggle = 0;

/* module functions */
static int  __init gpioirq_driver_init(void);
static void __exit gpioirq_driver_exit(void);

/* file operations functions  */
static int     gpioirq_open(struct inode *inode, struct file *file);
static int     gpioirq_release(struct inode *inode, struct file *file);
static ssize_t gpioirq_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t gpioirq_write(struct file *filp, const char *buf, size_t len, loff_t * off);

static struct file_operations fops =
{
    .owner      = THIS_MODULE,
    .read       = gpioirq_read,
    .write      = gpioirq_write,
    .open       = gpioirq_open,
    .release    = gpioirq_release,
};

/******************************************************************************************************/
/* fops implementations */

/* This function of read the Device file */
static ssize_t gpioirq_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
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
static ssize_t gpioirq_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    pr_info("@frk: come to [gpio21_write]\n");
    uint8_t rec_buf[10] = {0};

    if(copy_from_user(rec_buf, buf, len) > 0){
        pr_err("ERROR: Not all the bytes have been copied from user \n");
    }

    if(rec_buf[0] == '1')
    {
        // set GPIO value to HIGH
        gpio_set_value(GPIO_21, 1);
    } 
    else if (rec_buf[0] == '0') 
    {
        // set GPIO value to LOW
        gpio_set_value(GPIO_21, 0);
    } else 
    {
        pr_err("Unknown command: please provide either 0 or 1\n");
    }

    pr_info("Write Function\n");

    return len;
}

/* This function of open the Device file */ 
static int gpioirq_open(struct inode *inode, struct file *file)
{
    pr_info("Device File Opened...!!!\n");
    return 0;
}

/* This function of close the Device file */ 
static int gpioirq_release(struct inode *inode, struct file *file)
{
    pr_info("Device File Closed...!!!\n");
    return 0;
}

/******************************************************************************************************/
/* interupt handler */
static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
    static unsigned long flags = 0;

#ifdef EN_DEBOUNCE
    unsigned long diff = jiffies - old_jiffie;
    if(diff < 20){
        return IRQ_HANDLED;
    }
    old_jiffie = jiffies;
#endif

    // 
    local_irq_save(flags);

    // 
    led_toggle = (0x01 ^ led_toggle);

    // 
    gpio_set_value(GPIO_21_OUT, led_toggle);
    pr_info("Interupt occured: GPIO_21_OUT = %d", gpio_get_value(GPIO_21_OUT));

    // 
    local_irq_restore(flags);

    return IRQ_HANDLED;
}

/******************************************************************************************************/
/* Module Init function */

static int __init gpioirq_driver_init(void)
{

/*
** create under /dev/... 
*/
    /* Allocating Major number */
    if((alloc_chrdev_region(&dev, 0, 1, "gpioirq_dev")) <0){
        pr_info("Cannot allocate major number\n");
        goto r_unreg;
    }

    pr_info("Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));

    /* Creating cdev structure */
    cdev_init(&gpioirq_cdev,&fops);

    /* Adding character device to the system */
    if((cdev_add(&gpioirq_cdev,dev,1)) < 0){
        pr_info("Cannot add the device to the system\n");
        goto r_del;
    }

    /* Create class */
    if(IS_ERR(dev_class = class_create(THIS_MODULE,"gpioirq_class"))){
        pr_info("Cannot create the struct class\n");
        goto r_class;
    }

    /* Creating device */
    if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"gpioirq_device"))){
        pr_info("Cannot create the Device 1\n");
        goto r_device;
    }

/*
** GPIO21 output pin steps
*/
    /* Checking GPIO is valid or not! */
    if(gpio_is_valid(GPIO_21_OUT) == false){
        pr_err("GPIO[%d] is not valid.\n", GPIO_21_OUT);
        goto r_gpio_out;
    }

    /* Requesting GPIO */
    if(gpio_request(GPIO_21_OUT, "GPIO_21_OUT") < 0){
        pr_err("ERROR: GPIO %d request error.\n", GPIO_21_OUT);
        goto r_gpio_out;
    }

    /* Configure GPIO as output */
    gpio_direction_output(GPIO_21_OUT, 0);

/*
** GPIO25 input pin steps
*/
    /* Checking GPIO is valid or not! */
    if(gpio_is_valid(GPIO_25_IN) == false){
        pr_err("GPIO[%d] is not valid.\n", GPIO_25_IN);
        goto r_gpio_in;
    }

    /* Requesting GPIO */
    if(gpio_request(GPIO_25_IN, "GPIO_25_IN") < 0){
        pr_err("ERROR: GPIO %d request error.\n", GPIO_25_IN);
        goto r_gpio_in;
    }

    /* Configure GPIO as output */
    gpio_direction_input(GPIO_25_IN);

/*
** Interupts process  
*/
#ifndef EN_DEBOUNCE
    /* Debounce the but with a delay of 200ms*/
    if(gpio_set_debounce(GPIO_25_IN, 200) < 0){
        pr_err("ERROR: gpio_set_debounce not work: %d.\n", GPIO_25_IN);
    }
#endif

    /* interupt number for GPIO25_OUT pin*/
    gpio_irq_num = gpio_to_irq(GPIO_25_IN);
    pr_info("gpio_irq_number = %d", gpio_irq_num);

    if(request_irq(
        gpio_irq_num,              // irq number
        (void *)gpio_irq_handler,  // irq handler
        IRQF_TRIGGER_RISING,       // Handler will be called in raising edge
        "gpioirq_device",          // device name for irq
        NULL)){
            pr_err("My driver cannot register IRQ_NUM");
            goto r_gpio_in;
    }

/**
  * 
 */
    pr_info("Device Driver Insert...Done!!!\n");

    return 0;

r_gpio_in:
    gpio_free(GPIO_25_IN);
r_gpio_out:
    gpio_free(GPIO_21_OUT);
r_device:
    device_destroy(dev_class, dev);
r_class:
    class_destroy(dev_class);
r_del:
    cdev_del(&gpioirq_cdev);
r_unreg:
    unregister_chrdev_region(dev,1);

    return -1;
}

/* Module exit function */
static void __exit gpioirq_driver_exit(void)
{
    free_irq(gpio_irq_num, NULL);
    gpio_free(GPIO_25_IN);
    gpio_free(GPIO_21_OUT);
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&gpioirq_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Device Driver Remove...Done!!!\n");
}
/******************************************************************************************************/
module_init(gpioirq_driver_init);
module_exit(gpioirq_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("FRANK <frank@bos-semi.com>");
MODULE_DESCRIPTION("GPIO25 IRQ DRIVER");
MODULE_VERSION("1.8");

/******************************************************************************************************/

