/***************************************************************************************************//**
*  \file       oled_spi_driver.c
*
*  \details    SPI client device driver (OLED-SSH1106)
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

#include <linux/jiffies.h>

extern unsigned long volatile jiffies;
unsigned long old_jiffie = 0;

/* print */
#undef pr_fmt
#define pr_fmt(fmt) "@frk-spi_device_driver: [%s] :" fmt,__func__

/* sysfs */
volatile int frk_spi_value = 1;
volatile char* frk_spi_string = "Hallo to sysfs";

dev_t dev = 0;
static struct class *dev_class;
static struct cdev frk_spi_cdev;
struct kobject *kobj_ref;

/* */
#define OLED_SPI_NAME           "oled_ssh1106"    // 
#define OLED_SPI_BUS_AVAILABLE  0                 // SPI0 bus is used.
#define SSH1106_RST_PIN         (  24 )           // RST (Reset pin), GPIO 24
#define SSH1106_DC_PIN          (  23 )           // DC (Data/Command pin), GPIO 23

/* SPI device */
static struct spi_device* oled_spi_device;

/* */
static struct spi_board_info oled_info = 
{
    .modalias       = "oled_spi_driver",
    .max_speed_hz   = 2000000,
    .bus_num        = OLED_SPI_BUS_AVAILABLE,
    .chip_select    = 1,                          // CS1 is used.
    .mode           = SPI_MODE_0 
};

/*******************************************************/
void ETX_SSH1106_ClearDisplay( void );
void display_rectangle(int len);
void display_frank(void);
void ETX_SSH1106_String(char *str);
void ETX_SSH1106_SetCursor( uint8_t lineNo, uint8_t cursorPos );

/*************** Driver functions **********************/
static int      frk_spi_open(struct inode *inode, struct file *file);
static int      frk_spi_release(struct inode *inode, struct file *file);
static ssize_t  frk_spi_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  frk_spi_write(struct file *filp, const char *buf, size_t len, loff_t * off);
 
/*************** Sysfs functions **********************/
static ssize_t  sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t  sysfs_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count);
static ssize_t  sysfs_show_1(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t  sysfs_store_1(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count);
struct kobj_attribute frk_spi_attr   = __ATTR(frk_spi_value, 0660, sysfs_show, sysfs_store);
struct kobj_attribute frk_spi_attr_1 = __ATTR(frk_spi_string, 0660, sysfs_show_1, sysfs_store_1);

/* file operation structure */
static struct file_operations fops = {
  .owner = THIS_MODULE,
  .read  = frk_spi_read,
  .write = frk_spi_write,
  .open  = frk_spi_release,
};

/*************** Sysfs functions ***************************************************************************/
/*
** This function will be called when we read the sysfs file
*/
static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
        pr_info("Sysfs - Read!!!\n");
        return sprintf(buf, "%d\n", frk_spi_value);
}

static ssize_t sysfs_show_1(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
        pr_info("Sysfs - Read!!!\n");
        return sprintf(buf, "%s\n", frk_spi_string);
}

/*
** This function will be called when we write the sysfsfs file
*/
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count)
{
      pr_info("Sysfs - Write!!!\n");

      /* get update spi_value from user space */
      sscanf(buf,"%d\n",&frk_spi_value);

      if( frk_spi_value ) 
      {
        // Clear the display
        ETX_SSH1106_ClearDisplay();
        /* display rectangle */
        display_rectangle(7);       
        /* display Frank*/
        display_frank();
      }

      #if 0
      else {
        // Clear the display
        ETX_SSH1106_ClearDisplay();
        //
        ETX_SSH1106_SetCursor(3,15);
        // display string
        ETX_SSH1106_String(frk_spi_string);
      }
      #endif

        return count;
}

static ssize_t sysfs_store_1(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count)
{
      pr_info("Sysfs - Write!!!\n");

      /* get update spi_value from user space */
      sscanf(buf,"%s",&frk_spi_string);

      if( frk_spi_value ) 
      {
        // Clear the display
        ETX_SSH1106_ClearDisplay();
        //
        ETX_SSH1106_SetCursor(3,15);
        // display string
        ETX_SSH1106_String(frk_spi_string);
      }

        return count;
}

/*************** Driver functions *************************************************************************/
/*
** This function will be called when we open the Device file
*/ 
static int frk_spi_open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened...!!!\n");
        return 0;
}
/*
** This function will be called when we close the Device file
*/ 
static int frk_spi_release(struct inode *inode, struct file *file)
{
        pr_info("Device File Closed...!!!\n");
        return 0;
}
 
/*
** This function will be called when we read the Device file
*/
static ssize_t frk_spi_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        pr_info("Read function\n");
        return 0;
}
/*
** This function will be called when we write the Device file
*/
static ssize_t frk_spi_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        pr_info("Write Function\n");
        return len;
}

/******************************************************************************************************/
/* SSH1106-OLED headers */
/******************************************************************************************************/
#define SSH1106_MAX_SEG         ( 132 )           // Maximum segment

#define SSH1106_MAX_LINE        (   7 )           // Maximum line
#define SSH1106_DEF_FONT_SIZE   (   5 )           // Default font size

/*
** Variable to store Line Number and Cursor Position.
*/ 
static uint8_t SSH1106_LineNum   = 0;
static uint8_t SSH1106_CursorPos = 0;
static uint8_t SSH1106_FontSize  = SSH1106_DEF_FONT_SIZE;

static void ETX_SSH1106_fill( uint8_t data );

/* OLED EED1106 APIs, from EmbedTronix */
/****************************************************************************
 * Name: frk_spi_spi_write
 *
 * Details : This function writes the 1-byte data to the slave device using SPI.
 ****************************************************************************/
int frk_spi_spi_write( uint8_t data )
{
  int     ret = -1;
  uint8_t rx  = 0x00;
  
  if( oled_spi_device )
  {    
  
    struct spi_transfer  tr = 
    {
      .tx_buf  = &data,
      .rx_buf  = &rx,
      .len     = 1,
    };
  
    spi_sync_transfer( oled_spi_device, &tr, 1 );
  }
  
  //pr_info("Received = 0x%02X \n", rx);
  
  return( ret );
}

/*
** Array Variable to store the letters.
*/ 
static const unsigned char SSH1106_font[][SSH1106_DEF_FONT_SIZE]= 
{
    {0x00, 0x00, 0x00, 0x00, 0x00},   // space
    {0x00, 0x00, 0x2f, 0x00, 0x00},   // !
    {0x00, 0x07, 0x00, 0x07, 0x00},   // "
    {0x14, 0x7f, 0x14, 0x7f, 0x14},   // #
    {0x24, 0x2a, 0x7f, 0x2a, 0x12},   // $
    {0x23, 0x13, 0x08, 0x64, 0x62},   // %
    {0x36, 0x49, 0x55, 0x22, 0x50},   // &
    {0x00, 0x05, 0x03, 0x00, 0x00},   // '
    {0x00, 0x1c, 0x22, 0x41, 0x00},   // (
    {0x00, 0x41, 0x22, 0x1c, 0x00},   // )
    {0x14, 0x08, 0x3E, 0x08, 0x14},   // *
    {0x08, 0x08, 0x3E, 0x08, 0x08},   // +
    {0x00, 0x00, 0xA0, 0x60, 0x00},   // ,
    {0x08, 0x08, 0x08, 0x08, 0x08},   // -
    {0x00, 0x60, 0x60, 0x00, 0x00},   // .
    {0x20, 0x10, 0x08, 0x04, 0x02},   // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E},   // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00},   // 1
    {0x42, 0x61, 0x51, 0x49, 0x46},   // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31},   // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10},   // 4
    {0x27, 0x45, 0x45, 0x45, 0x39},   // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30},   // 6
    {0x01, 0x71, 0x09, 0x05, 0x03},   // 7
    {0x36, 0x49, 0x49, 0x49, 0x36},   // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E},   // 9
    {0x00, 0x36, 0x36, 0x00, 0x00},   // :
    {0x00, 0x56, 0x36, 0x00, 0x00},   // ;
    {0x08, 0x14, 0x22, 0x41, 0x00},   // <
    {0x14, 0x14, 0x14, 0x14, 0x14},   // =
    {0x00, 0x41, 0x22, 0x14, 0x08},   // >
    {0x02, 0x01, 0x51, 0x09, 0x06},   // ?
    {0x32, 0x49, 0x59, 0x51, 0x3E},   // @
    {0x7C, 0x12, 0x11, 0x12, 0x7C},   // A
    {0x7F, 0x49, 0x49, 0x49, 0x36},   // B
    {0x3E, 0x41, 0x41, 0x41, 0x22},   // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C},   // D
    {0x7F, 0x49, 0x49, 0x49, 0x41},   // E
    {0x7F, 0x09, 0x09, 0x09, 0x01},   // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A},   // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F},   // H
    {0x00, 0x41, 0x7F, 0x41, 0x00},   // I
    {0x20, 0x40, 0x41, 0x3F, 0x01},   // J
    {0x7F, 0x08, 0x14, 0x22, 0x41},   // K
    {0x7F, 0x40, 0x40, 0x40, 0x40},   // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F},   // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F},   // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E},   // O
    {0x7F, 0x09, 0x09, 0x09, 0x06},   // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E},   // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46},   // R
    {0x46, 0x49, 0x49, 0x49, 0x31},   // S
    {0x01, 0x01, 0x7F, 0x01, 0x01},   // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F},   // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F},   // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F},   // W
    {0x63, 0x14, 0x08, 0x14, 0x63},   // X
    {0x07, 0x08, 0x70, 0x08, 0x07},   // Y
    {0x61, 0x51, 0x49, 0x45, 0x43},   // Z
    {0x00, 0x7F, 0x41, 0x41, 0x00},   // [
    {0x55, 0xAA, 0x55, 0xAA, 0x55},   // Backslash (Checker pattern)
    {0x00, 0x41, 0x41, 0x7F, 0x00},   // ]
    {0x04, 0x02, 0x01, 0x02, 0x04},   // ^
    {0x40, 0x40, 0x40, 0x40, 0x40},   // _
    {0x00, 0x03, 0x05, 0x00, 0x00},   // `
    {0x20, 0x54, 0x54, 0x54, 0x78},   // a
    {0x7F, 0x48, 0x44, 0x44, 0x38},   // b
    {0x38, 0x44, 0x44, 0x44, 0x20},   // c
    {0x38, 0x44, 0x44, 0x48, 0x7F},   // d
    {0x38, 0x54, 0x54, 0x54, 0x18},   // e
    {0x08, 0x7E, 0x09, 0x01, 0x02},   // f
    {0x18, 0xA4, 0xA4, 0xA4, 0x7C},   // g
    {0x7F, 0x08, 0x04, 0x04, 0x78},   // h
    {0x00, 0x44, 0x7D, 0x40, 0x00},   // i
    {0x40, 0x80, 0x84, 0x7D, 0x00},   // j
    {0x7F, 0x10, 0x28, 0x44, 0x00},   // k
    {0x00, 0x41, 0x7F, 0x40, 0x00},   // l
    {0x7C, 0x04, 0x18, 0x04, 0x78},   // m
    {0x7C, 0x08, 0x04, 0x04, 0x78},   // n
    {0x38, 0x44, 0x44, 0x44, 0x38},   // o
    {0xFC, 0x24, 0x24, 0x24, 0x18},   // p
    {0x18, 0x24, 0x24, 0x18, 0xFC},   // q
    {0x7C, 0x08, 0x04, 0x04, 0x08},   // r
    {0x48, 0x54, 0x54, 0x54, 0x20},   // s
    {0x04, 0x3F, 0x44, 0x40, 0x20},   // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C},   // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C},   // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C},   // w
    {0x44, 0x28, 0x10, 0x28, 0x44},   // x
    {0x1C, 0xA0, 0xA0, 0xA0, 0x7C},   // y
    {0x44, 0x64, 0x54, 0x4C, 0x44},   // z
    {0x00, 0x10, 0x7C, 0x82, 0x00},   // {
    {0x00, 0x00, 0xFF, 0x00, 0x00},   // |
    {0x00, 0x82, 0x7C, 0x10, 0x00},   // }
    {0x00, 0x06, 0x09, 0x09, 0x06}    // ~ (Degrees)
};

/****************************************************************************
 * Name: ETX_SSH1106_ResetDcInit
 *
 * Details : This function Initializes and Configures the Reset and DC Pin
 ****************************************************************************/
static int ETX_SSH1106_ResetDcInit( void )
{
  int ret = 0;
    
  //do while(false) loop to break if any error
  do
  {
    
    /* Register the Reset GPIO */
    
    //Checking the Reset GPIO is valid or not
    if( gpio_is_valid( SSH1106_RST_PIN ) == false )
    {
      pr_err("Reset GPIO %d is not valid\n", SSH1106_RST_PIN);
      ret = -1;
      break;
    }
    
    //Requesting the Reset GPIO
    if( gpio_request( SSH1106_RST_PIN, "SSH1106_RST_PIN" ) < 0 )
    {
      pr_err("ERROR: Reset GPIO %d request\n", SSH1106_RST_PIN);
      ret = -1;
      break;
    }
    
    //configure the Reset GPIO as output
    gpio_direction_output( SSH1106_RST_PIN, 1 );
    
    /* Register the DC GPIO */
    
    //Checking the DC GPIO is valid or not
    if( gpio_is_valid( SSH1106_DC_PIN ) == false )
    {
      pr_err("DC GPIO %d is not valid\n", SSH1106_DC_PIN);
      gpio_free( SSH1106_RST_PIN );   // free the reset GPIO
      ret = -1;
      break;
    }
    
    //Requesting the DC GPIO
    if( gpio_request( SSH1106_DC_PIN, "SSH1106_DC_PIN" ) < 0 )
    {
      pr_err("ERROR: DC GPIO %d request\n", SSH1106_DC_PIN);
      gpio_free( SSH1106_RST_PIN );   // free the reset GPIO
      ret = -1;
      break;
    }
    
    //configure the Reset GPIO as output
    gpio_direction_output( SSH1106_DC_PIN, 1 );
    
  } while( false );
  
  //pr_info("DC Reset GPIOs init Done!\n");
  return( ret );
}

/****************************************************************************
 * Name: ETX_SSH1106_ResetDcDeInit
 *
 * Details : This function De-initializes the Reset and DC Pin
 ****************************************************************************/
static void ETX_SSH1106_ResetDcDeInit( void )
{
  gpio_free( SSH1106_RST_PIN );   // free the reset GPIO
  gpio_free( SSH1106_DC_PIN );    // free the DC GPIO
}

/****************************************************************************
 * Name: ETX_SSH1106_setRst
 *
 * Details : This function writes the value to the Reset GPIO
 * 
 * Argument: 
 *            value - value to be set ( 0 or 1 )
 * 
 ****************************************************************************/
static void ETX_SSH1106_setRst( uint8_t value )
{
  gpio_set_value( SSH1106_RST_PIN, value );
}

/****************************************************************************
 * Name: ETX_SSH1106_setDc
 *
 * Details : This function writes the value to the DC GPIO
 * 
 * Argument: 
 *            value - value to be set ( 0 or 1 )
 * 
 ****************************************************************************/
static void ETX_SSH1106_setDc( uint8_t value )
{
  gpio_set_value( SSH1106_DC_PIN, value );
}

/****************************************************************************
 * Name: ETX_SSH1106_Write
 *
 * Details : This function sends the command/data to the Display
 *
 * Argument: is_cmd
 *              true  - if we need to send command
 *              false - if we need to send data
 *           value
 *              value to be transmitted
 ****************************************************************************/
static int ETX_SSH1106_Write( bool is_cmd, uint8_t data )
{
  int     ret = 0;
  uint8_t pin_value;
  if( is_cmd )
  {
    //DC pin has to be low, if this is command.
    pin_value = 0u;
  }
  else
  {
    //DC pin has to be high, if this is data.
    pin_value = 1u;
  }
  
  ETX_SSH1106_setDc( pin_value );
  
  //pr_info("Writing 0x%02X \n", data);
  
  //send the byte
  ret = frk_spi_spi_write( data );
  
  return( ret );
}

/****************************************************************************
 * Name: ETX_SSH1106_SetCursor
 *
 * Details : This function is specific to the SSD_1306 OLED.
 *
 * Argument:
 *              lineNo    -> Line Number
 *              cursorPos -> Cursor Position
 * 
 ****************************************************************************/
void ETX_SSH1106_SetCursor( uint8_t lineNo, uint8_t cursorPos )
{

  /* Move the Cursor to specified position only if it is in range */
  if((lineNo <= SSH1106_MAX_LINE) && (cursorPos < SSH1106_MAX_SEG))
  {

    SSH1106_LineNum   = lineNo;                    // Save the specified line number
    SSH1106_CursorPos = cursorPos;                 // Save the specified cursor position
    
    /* set page address */
    ETX_SSH1106_Write(true, 0xB0 | lineNo);

    /* set column address */
    ETX_SSH1106_Write(true, 0x00 | (cursorPos&0x0F));            // column start addr
    ETX_SSH1106_Write(true, 0x10 | ( (cursorPos>>4) + 0x10));    // column end addr

  }
}

/****************************************************************************
 * Name: ETX_SSH1106_GoToNextLine
 *
 * Details : This function is specific to the SSD_1306 OLED and move the cursor 
 *           to the next line.
 ****************************************************************************/
void ETX_SSH1106_GoToNextLine( void )
{
  /*
  ** Increment the current line number.
  ** roll it back to first line, if it exceeds the limit. 
  */

  SSH1106_LineNum++;

  SSH1106_LineNum = (SSH1106_LineNum & SSH1106_MAX_LINE);

  ETX_SSH1106_SetCursor(SSH1106_LineNum,0); /* Finally move it to next line */

}

/****************************************************************************
 * Name: ETX_SSH1106_PrintChar
 *
 * Details : This function is specific to the SSD_1306 OLED and sends 
 *           the single char to the OLED.
 * 
 * Arguments:
 *           c   -> character to be written
 * 
 ****************************************************************************/
void ETX_SSH1106_PrintChar( unsigned char c )
{
  uint8_t data_byte;
  uint8_t temp = 0;

  /*
  ** If we character is greater than segment len or we got new line charcter
  ** then move the cursor to the new line
  */ 
  if( (( SSH1106_CursorPos + SSH1106_FontSize ) >= SSH1106_MAX_SEG ) ||
      ( c == '\n' )
  )
  {
    ETX_SSH1106_GoToNextLine();
  }
  
  // print charcters other than new line
  if( c != '\n' )
  {
  
    /*
    ** In our font array (SSH1106_font), space starts in 0th index.
    ** But in ASCII table, Space starts from 32 (0x20).
    ** So we need to match the ASCII table with our font table.
    ** We can subtract 32 (0x20) in order to match with our font table.
    */
    c -= 0x20;  //or c -= ' ';
    do
    {
      data_byte= SSH1106_font[c][temp];         // Get the data to be displayed from LookUptable
      ETX_SSH1106_Write(false, data_byte);  // write data to the OLED
      SSH1106_CursorPos++;
      
      temp++;
      
    } while ( temp < SSH1106_FontSize);
    
    ETX_SSH1106_Write(false, 0x00);         //Display the data
    SSH1106_CursorPos++;
  }
}

/****************************************************************************
 * Name: ETX_SSH1106_String
 *
 * Details : This function is specific to the SSD_1306 OLED and sends 
 *           the string to the OLED.
 * 
 * Arguments:
 *           str   -> string to be written
 * 
 ****************************************************************************/
void ETX_SSH1106_String(char *str)
{
  while( *str )
  {
    ETX_SSH1106_PrintChar(*str++);
  }
}

/****************************************************************************
 * Name: ETX_SSH1106_InvertDisplay
 *
 * Details : This function is specific to the SSD_1306 OLED and 
 *           inverts the display.
 * 
 * Arguments:
 *           need_to_invert   -> true  - invert display
 *                               false - normal display 
 * 
 ****************************************************************************/
void ETX_SSH1106_InvertDisplay(bool need_to_invert)
{
  if(need_to_invert)
  {
    ETX_SSH1106_Write(true, 0xA7); // Invert the display
  }
  else
  {
    ETX_SSH1106_Write(true, 0xA6); // Normal display
  }
}

/****************************************************************************
 * Name: ETX_SSH1106_fill
 *
 * Details : This function clears the Display
 ****************************************************************************/
static void ETX_SSH1106_fill(unsigned char data)
{
  unsigned int total  = 128 * 8;  // 8 pages x 128 segments x 8 bits of data
  unsigned int i      = 0;
  
  //Fill the Display
  for(i = 0; i < total; i++)
  {
      ETX_SSH1106_Write(false, data);
  }
}

/****************************************************************************
 * Name: ETX_SSH1106_ClearDisplay
 *
 * Details : This function clears the Display
 ****************************************************************************/
void ETX_SSH1106_ClearDisplay( void )
{
  unsigned int i = 0;
  
  for (i=0; i<SSH1106_MAX_LINE+1; i++){
    ETX_SSH1106_SetCursor(i,0);
    ETX_SSH1106_fill( 0x00 );
  }

}

/****************************************************************************
 * Name: ETX_SSH1106_DisplayInit
 *
 * Details : This function Initializes the Display
 ****************************************************************************/
int ETX_SSH1106_DisplayInit(void)
{
  int ret = 0;

  //Initialize the Reset and DC GPIOs
  ret = ETX_SSH1106_ResetDcInit();

  if( ret >= 0 )
  {
    //Make the RESET Line to 0
    ETX_SSH1106_setRst( 0u );
    msleep(100);                          // delay
    //Make the DC Line to 1
    ETX_SSH1106_setRst( 1u );
    msleep(100);                          // delay

#if 1
    /*
    ** Commands to initialize the SSD_1106 OLED Display
    */
    ETX_SSH1106_Write(true, 0xAE);        // Entire Display OFF
    ETX_SSH1106_Write(true, 0xD5);        // Set Display Clock Divide Ratio and Oscillator Frequency
    ETX_SSH1106_Write(true, 0x80);        // Default Setting for Display Clock Divide Ratio and Oscillator Frequency that is recommended
    ETX_SSH1106_Write(true, 0xA8);        // Set Multiplex Ratio
    ETX_SSH1106_Write(true, 0x3F);        // 64 COM lines
    ETX_SSH1106_Write(true, 0xD3);        // Set display offset
    ETX_SSH1106_Write(true, 0x00);        // 0 offset
    ETX_SSH1106_Write(true, 0x40);        // Set first line as the start line of the display
    ETX_SSH1106_Write(true, 0xAD);        // Charge pump
    ETX_SSH1106_Write(true, 0x8B);        // Enable charge dump during display on
    ETX_SSH1106_Write(true, 0xA1);        // Set segment remap with column address 127 mapped to segment 0
    ETX_SSH1106_Write(true, 0xC8);        // Set com output scan direction, scan from com63 to com 0
    ETX_SSH1106_Write(true, 0xDA);        // Set com pins hardware configuration
    ETX_SSH1106_Write(true, 0x12);        // Alternative com pin configuration, disable com left/right remap
    ETX_SSH1106_Write(true, 0x81);        // Set contrast control
    ETX_SSH1106_Write(true, 0xBF);        // Set Contrast to 128
    ETX_SSH1106_Write(true, 0xD9);        // Set pre-charge period
    ETX_SSH1106_Write(true, 0x22);        // Phase 1 period of 15 DCLK, Phase 2 period of 1 DCLK 
    ETX_SSH1106_Write(true, 0xDB);        // Set Vcomh deselect level
    ETX_SSH1106_Write(true, 0x40);        // Vcomh deselect level ~ 0.77 Vcc
    ETX_SSH1106_Write(true, 0x32);        // Set VPP
    ETX_SSH1106_Write(true, 0xA6);        // Set Display in Normal Mode, 1 = ON, 0 = OFF
    ETX_SSH1106_Write(true, 0xAF);        // Display ON in normal mode
    
#endif

  }

  return( ret );
}

/****************************************************************************
 * Name: ETX_SSH1106_DisplayDeInit
 *
 * Details : This function De-initializes the Display
 ****************************************************************************/
void ETX_SSH1106_DisplayDeInit(void)
{
  ETX_SSH1106_ResetDcDeInit();  //Free the Reset and DC GPIO
}

/****************************************************************************
*   @frk: to develop some funcs based on APIs
*
*****************************************************************************/

/* to display some shapes */
void display_rectangle(int len)
{
  int i;

  /* set ...*/
  ETX_SSH1106_SetCursor(0,0);  
  /* Display "Hallo world" to OLED */
  ETX_SSH1106_String("**********************\n");

  for ( i=1; i<len; i++)
  {
    ETX_SSH1106_SetCursor(i,0);  
    /* Display "Hallo world" to OLED */
    ETX_SSH1106_String("*                    *\n");
  }

  /* set ...*/
  ETX_SSH1106_SetCursor(len,0);  
  /* Display "Hallo world" to OLED */
  ETX_SSH1106_String("**********************\n");

}

/* display frank */
void display_frank(void)
{
  ETX_SSH1106_SetCursor(3,15);
  ETX_SSH1106_String("BOS/ SW/ Frank\n");
  //ETX_SSH1106_String("Hallo world!!!!");
}

/******************************************************************************************************/
/* module init func */
static int __init oled_spi_driver_init(void)
{
    pr_info("\n@frk: going to init...");

/* sysfs part */
    /*Allocating Major number*/
    if((alloc_chrdev_region(&dev, 0, 1, "frk_spi_Dev")) <0){
            pr_info("Cannot allocate major number\n");
            return -1;
    }
    pr_info("Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));
 
    /*Creating cdev structure*/
    cdev_init(&frk_spi_cdev,&fops);
 
    /*Adding character device to the system*/
    if((cdev_add(&frk_spi_cdev,dev,1)) < 0){
        pr_info("Cannot add the device to the system\n");
        goto r_class;
    }
 
    /*Creating struct class*/
    if(IS_ERR(dev_class = class_create(THIS_MODULE,"frk_spi_class"))){
        pr_info("Cannot create the struct class\n");
        goto r_class;
    }
 
    /*Creating device*/
    if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"frk_spi_device"))){
        pr_info("Cannot create the Device 1\n");
        goto r_device;
    }
 
    /*Creating a directory in /sys/kernel/ */
    kobj_ref = kobject_create_and_add("frk_spi_sysfs",kernel_kobj);
 
    /*Creating sysfs file for frk_spi_value*/
    if(sysfs_create_file(kobj_ref,&frk_spi_attr.attr)){
            pr_err("Cannot create sysfs file......\n");
            goto r_sysfs;
    }
    if(sysfs_create_file(kobj_ref,&frk_spi_attr_1.attr)){
            pr_err("Cannot create sysfs file......\n");
            goto r_sysfs;
    }

/* */
    int ret; 
    struct spi_master* master;

    /* get SPI master */
    master = spi_busnum_to_master(oled_info.bus_num);
    if( !master ){
        pr_err("\n@frk: Failed to get SPI master!!!");
        return -ENODEV;
    }

    /* */
    oled_spi_device = spi_new_device(master, &oled_info);
    if( oled_spi_device == NULL){
        pr_err("\n@frk: Failed to create spi device.");
        return -ENODEV;
    }

    /* */
    oled_spi_device->bits_per_word = 8;

    /* setup SPI slave device */
    ret = spi_setup(oled_spi_device);
    if(ret){
        pr_err("\n@frk: Failed to setup slave.");
        spi_unregister_device(oled_spi_device);
        return -ENODEV;
    }

/* SSH1106 APIs here */

#if 1
    pr_info("\n@FRK: going to display DATA by OLED APIs.");    

    /* init  */
    ETX_SSH1106_DisplayInit();
#endif

#if 1
    // Clear the display
    ETX_SSH1106_ClearDisplay();

    /* display rectangle */
    display_rectangle(7);       

    /* display Frank*/
    display_frank();
#endif

/* return success */
    pr_info("\n @frk: SPI-oled insert ... DONE!!! \n");
    return 0;

/* exception */
r_sysfs:
        kobject_put(kobj_ref); 
        sysfs_remove_file(kernel_kobj, &frk_spi_attr.attr);
        sysfs_remove_file(kernel_kobj, &frk_spi_attr_1.attr);
 
r_device:
        class_destroy(dev_class);

r_class:
        unregister_chrdev_region(dev,1);
        cdev_del(&frk_spi_cdev);
        return -1;

}

/* module exit func*/
static void __exit oled_spi_driver_exit(void)
{
    pr_info("\n@frk: going to remove...");

/* sysfs */
    kobject_put(kobj_ref); 
    sysfs_remove_file(kernel_kobj, &frk_spi_attr.attr);
    sysfs_remove_file(kernel_kobj, &frk_spi_attr_1.attr);
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&frk_spi_cdev);
    unregister_chrdev_region(dev, 1);

/* SSH1106 APIs here */
    pr_info("\n@FRK: going to reset OLED.");   
    ETX_SSH1106_setRst( 0u );

    /* Clear the display */
    pr_info("\n#FRK: going to clean screen by OLED API.");
    msleep(1000);
    ETX_SSH1106_ClearDisplay();                 // Clear Display
    ETX_SSH1106_DisplayDeInit();                // Deinit the SSH1106

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
