/***************************************************************************************************//**
*  \file       oled_spi_driver.c
*
*  \details    SPI client device driver (OLED-1315)
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
#include <linux/spi/spi.h>

/* print */
#undef pr_fmt
#define pr_fmt(fmt) "@frk-spi_device_driver: [%s] :" fmt,__func__

#define OLED_SPI_NAME           "oled_ssd1315"    // 
#define OLED_SPI_BUS_AVAILABLE  0                 // SPI1 bus is used.

/* SPI device */
static struct spi_device* oled_spi_device;

/* */
static struct spi_board_info oled_info = 
{
    .modalias       = "oled_spi_driver",
    .max_speed_hz   = 2000000,
    .bus_num        = OLED_SPI_BUS_AVAILABLE,
    .chip_select    = 1,
    .mode           = SPI_MODE_0 
    
};

/******************************************************************************************************/
/******************************************************************************************************/
/* SSD1315-OLED headers */
//...

/******************************************************************************************************/
/* OLED EED1315 APIs, from EmbedTronix */
// ...

/******************************************************************************************************/
/******************************************************************************************************/

/******************************************************************************************************/
/* module init func */
static int __init oled_spi_driver_init(void)
{
    pr_info("\n@frk: going to init...");
    // ...
    int ret;
    struct spi_master* master;

    /* get SPI master */
    master = spi_busnum_to_master(oled_info.bus_num);
    if( !master ){
        pr_err("\n@frk: Failed to get SPI master!!!");
        return -ENODEV;
    }

pr_info("\n@frk: going to init... 1");
    /* */
    oled_spi_device = spi_new_device(master, &oled_info);
pr_info("\n@frk: going to init... 1.1");
    if( oled_spi_device == NULL){
        pr_err("\n@frk: Failed to create spi device.");
        return -ENODEV;
    }

pr_info("\n@frk: going to init... 2");
    /* */
    oled_spi_device->bits_per_word = 8;

    /* setup SPI slave device*/
    ret = spi_setup(oled_spi_device);
    if(ret){
        pr_err("\n@frk: Failed to setup slave.");
        spi_unregister_device(oled_spi_device);
        return -ENODEV;
    }

pr_info("\n@frk: going to init...3");

    /* SSD1315 APIs here*/


    /* return success */
    pr_info("\n @frk: SPI-oled insert ... DONE!!! \n");
    return 0;
}

/* module exit func*/
static void __exit oled_spi_driver_exit(void)
{
    pr_info("\n@frk: going to remove...");

    /*  */

    /* unregister the device from kernel */ 
    spi_unregister_device(oled_spi_device);

    /* return success */
    pr_info("\n @frk: SPI-oled remove ... DONE!!! \n");
}

/******************************************************************************************************/
module_init(oled_spi_driver_init);
module_exit(oled_spi_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("FRANK <frank@bos-semi.com>");
MODULE_DESCRIPTION("SPI OLED DRIVER");
MODULE_VERSION("1.8");

/******************************************************************************************************/

