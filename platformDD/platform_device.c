
/*platform data of the pcdev */ 
struct pcdev_platform_data 
{ 
    int         size; 
    int         perm; 
    const char *serial_number; 
}; 

/*Permission codes */ 
#define RDWR   0x11 
#define RDONLY 0x01 
#define WRONLY 0x10 

/***************************************************************************************/ 
#include <linux/module.h> 
#include <linux/platform_device.h> 
#include "platform.h" 
 
#undef pr_fmt 
#define pr_fmt(fmt) "@frk-setup: [%s] :" fmt,__func__ 
 
void pcdev_release(struct device *dev) 
{ 
    pr_info("Device released \n"); 
} 

/*1. create n platform data */ 
struct pcdev_platform_data  pcdev_pdata[] = { 
    [4] = {.size = 512, .perm = RDWR,   .serial_number = "FRK_PLAT_DEVICE_1"}, 
    [5] = {.size = 1024,.perm = RDWR,   .serial_number = "FRK_PLAT_DEVICE_2"}, 
    [2] = {.size = 128, .perm = RDONLY, .serial_number = "FRK_PLAT_DEVICE_3"}, 
    [3] = {.size = 32,  .perm = WRONLY, .serial_number = "FRK_PLAT_DEVICE_4"} 
}; 
 
/*2. create n platform devices */  
struct platform_device platform_pcdev_1 = { 
    .name = "f_platA1x", 
    .id = 4, 
    .dev = { 
        .platform_data = &pcdev_pdata[0], 
        .release = pcdev_release 
    } 
}; 
 
struct platform_device platform_pcdev_2 = { 
    .name = "f_platB1x", 
    .id = 5, 
    .dev = { 
        .platform_data = &pcdev_pdata[1], 
        .release = pcdev_release 
    } 
}; 

struct platform_device platform_pcdev_3 = { 
    .name = "f_platC1x", 
    .id = 2, 
    .dev = { 
        .platform_data = &pcdev_pdata[2], 
        .release = pcdev_release 
    } 
}; 
 
struct platform_device platform_pcdev_4 = { 
    .name = "f_platD1x", 
    .id = 3, 
    .dev = { 
        .platform_data = &pcdev_pdata[3], 
        .release = pcdev_release 
    } 
}; 

struct platform_device *platform_pcdevs[] =  
{ 
    &platform_pcdev_1, 
    &platform_pcdev_2, 
    &platform_pcdev_3, 
    &platform_pcdev_4 
}; 

/***************************************************************************************/ 
static int __init f_plat_init(void) 
{ 
    /* register n platform devices */ 
    //platform_device_register(&platform_pcdev_1); 
    //platform_device_register(&platform_pcdev_2);    

    platform_add_devices(platform_pcdevs,ARRAY_SIZE(platform_pcdevs) ); 
    pr_info("Device setup module loaded \n"); 

    return 0; 
} 

static void __exit f_plat_exit(void) 
{ 
    platform_device_unregister(&platform_pcdev_1); 
    platform_device_unregister(&platform_pcdev_2); 
    platform_device_unregister(&platform_pcdev_3); 
    platform_device_unregister(&platform_pcdev_4); 
    pr_info("Device setup module unloaded \n"); 
} 
/***************************************************************************************/ 
module_init(f_plat_init); 
module_exit(f_plat_exit); 

MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("FRANK"); 
MODULE_DESCRIPTION("HALLO-PLATFORM-DEVICE-SETUP"); 
MODULE_INFO(board, "RassberriPI"); 
/***************************************************************************************/ 

 
 

