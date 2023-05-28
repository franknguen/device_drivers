/*HEADER 
https://codebrowser.dev/linux/linux/include/linux/module.h.html 
https://codebrowser.dev/linux/linux/include/linux/moduleparam.h.html  
*/ 
#include <linux/module.h> 
#include <linux/moduleparam.h> 

#undef pr_fmt 
#define pr_fmt(fmt) "@frk: [%s] :" fmt,__func__ 
 
/* params to passing*/ 
int valETX, arr_valETX[4]; 
char* nameETX; 
int cb_valETX = 0; 
 
/*-------------------------------------------module_param_cb()--------------*/ 
module_param(valETX, int, S_IRUSR|S_IWUSR);                      // INTEGER values 
module_param(nameETX, charp, S_IRUSR|S_IWUSR);                   // String     
module_param_array(arr_valETX, int,NULL, S_IRUSR|S_IWUSR);       // Array of integers 
 
int set_notify_param(const char *val, const struct kernel_param *kp) 
{ 
        int res = param_set_int(val, kp); 
        if(res==0) { 
                pr_info("Call back function called...\n"); 
                pr_info("New value of cb_valETX = %d\n", cb_valETX); 
                return 0; 
        } 
        return -1; 
} 

const struct kernel_param_ops my_param_ops = { 
    .set = &set_notify_param, 
    .get = &param_get_int, 
}; 
 
module_param_cb(cb_valETX, &my_param_ops, &cb_valETX, S_IRUGO|S_IWUSR); 
/*-------------------------------------------module_param_cb()--------------*/ 
/*main code*/ 
static int __init passdd_init(void) 
{ 
    /**/ 
    int i; 
    pr_info("ValueETX = %d  \n", valETX); 
    pr_info("cb_valueETX = %d  \n", cb_valETX); 
    pr_info("NameETX = %s \n", nameETX); 
    for (i = 0; i < (sizeof arr_valETX / sizeof (int)); i++)  
    { 
        pr_info("Arr_value[%d] = %d\n", i, arr_valETX[i]); 
    } 
    pr_info("Insert successfully\n"); 
    return 0; 
} 

static void __exit passdd_exit(void) 
{ 
    pr_info("Remove successfully\n"); 
} 

/* 
* module_init() - driver initialization entry point 
* @x: function to be run at kernel boot time or module insertion  
* module_exit() - driver exit entry point 
* @x: function to be run when driver is removed 
*/ 

module_init(passdd_init); 
module_exit(passdd_exit); 
 
/**/ 
MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("FRANK"); 
MODULE_DESCRIPTION(""); 
MODULE_INFO(board, "RassberriPI"); 
