/***************************************************************************************************//**
*  \file       oled_i2c_driver.c
*
*  \details    I2C client device driver (OLED-1315)
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
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

/* print */
#undef pr_fmt
#define pr_fmt(fmt) "@frk-i2c_client_driver: [%s] :" fmt,__func__

#define OLED_I2C_NAME           "oled_ssd1315"
#define OLED_I2C_SLAVE_ADDR     0x3c
#define OLED_I2C_BUS_AVAILABLE  1

static struct i2c_client* oled_client;

static struct i2c_board_info oled_info = {
    I2C_BOARD_INFO(OLED_I2C_NAME, OLED_I2C_SLAVE_ADDR)
};

/******************************************************************************************************/
/******************************************************************************************************/
/* SSD1315-OLED headers */

#define SSD1315_MAX_SEG         (        128 )              // Maximum segment
#define SSD1315_MAX_LINE        (          7 )              // Maximum line
#define SSD1315_DEF_FONT_SIZE   (          5 )              // Default font size

static uint8_t SSD1315_LineNum   = 0;
static uint8_t SSD1315_CursorPos = 0;
static uint8_t SSD1315_FontSize  = SSD1315_DEF_FONT_SIZE;

static void SSD1315_Fill(unsigned char data);

/******************************************************************************************************/
/* OLED EED1315 APIs, from EmbedTronix */

static const unsigned char SSD1315_font[][SSD1315_DEF_FONT_SIZE]= 
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

static int I2C_Write(unsigned char *buf, unsigned int len)
{
  int ret = i2c_master_send(oled_client, buf, len);
  
  return ret;
}

static int I2C_Read(unsigned char *out_buf, unsigned int len)
{
  int ret = i2c_master_recv(oled_client, out_buf, len);
  
  return ret;
}

static void SSD1315_Write(bool is_cmd, unsigned char data)
{
  unsigned char buf[2] = {0};
  int ret;
  
  if( is_cmd == true )
  {
      buf[0] = 0x00;
  }
  else
  {
      buf[0] = 0x40;
  }
  
  buf[1] = data;
  
  ret = I2C_Write(buf, 2);
}


static void SSD1315_SetCursor( uint8_t lineNo, uint8_t cursorPos )
{
  /* Move the Cursor to specified position only if it is in range */
  if((lineNo <= SSD1315_MAX_LINE) && (cursorPos < SSD1315_MAX_SEG))
  {
    SSD1315_LineNum   = lineNo;             // Save the specified line number
    SSD1315_CursorPos = cursorPos;          // Save the specified cursor position
    SSD1315_Write(true, 0x21);              // cmd for the column start and end address
    SSD1315_Write(true, cursorPos);         // column start addr
    SSD1315_Write(true, SSD1315_MAX_SEG-1); // column end addr
    SSD1315_Write(true, 0x22);              // cmd for the page start and end address
    SSD1315_Write(true, lineNo);            // page start addr
    SSD1315_Write(true, SSD1315_MAX_LINE);  // page end addr
  }
}

static void  SSD1315_GoToNextLine( void )
{
  SSD1315_LineNum++;
  SSD1315_LineNum = (SSD1315_LineNum & SSD1315_MAX_LINE);

  SSD1315_SetCursor(SSD1315_LineNum,0); /* Finally move it to next line */
}

static void SSD1315_PrintChar(unsigned char c)
{
  uint8_t data_byte;
  uint8_t temp = 0;

  /*
  ** If we character is greater than segment len or we got new line charcter
  ** then move the cursor to the new line
  */ 
  if( (( SSD1315_CursorPos + SSD1315_FontSize ) >= SSD1315_MAX_SEG ) ||
      ( c == '\n' )
  )
  {
    SSD1315_GoToNextLine();
  }

  // print charcters other than new line
  if( c != '\n' )
  {
  
    /*
    ** In our font array (SSD1315_font), space starts in 0th index.
    ** But in ASCII table, Space starts from 32 (0x20).
    ** So we need to match the ASCII table with our font table.
    ** We can subtract 32 (0x20) in order to match with our font table.
    */
    c -= 0x20;  //or c -= ' ';

    do
    {
      data_byte= SSD1315_font[c][temp]; // Get the data to be displayed from LookUptable

      SSD1315_Write(false, data_byte);  // write data to the OLED
      SSD1315_CursorPos++;
      
      temp++;
      
    } while ( temp < SSD1315_FontSize);
    SSD1315_Write(false, 0x00);         //Display the data
    SSD1315_CursorPos++;
  }
}

static void SSD1315_String(unsigned char *str)
{
  while(*str)
  {
    SSD1315_PrintChar(*str++);
  }
}

static void SSD1315_InvertDisplay(bool need_to_invert)
{
  if(need_to_invert)
  {
    SSD1315_Write(true, 0xA7); // Invert the display
  }
  else
  {
    SSD1315_Write(true, 0xA6); // Normal display
  }
}

static void SSD1315_SetBrightness(uint8_t brightnessValue)
{
    SSD1315_Write(true, 0x81); // Contrast command
    SSD1315_Write(true, brightnessValue); // Contrast value (default value = 0x7F)
}

static void SSD1315_StartScrollHorizontal( bool is_left_scroll,
                                           uint8_t start_line_no,
                                           uint8_t end_line_no
                                         )
{
  if(is_left_scroll)
  {
    // left horizontal scroll
    SSD1315_Write(true, 0x27);
  }
  else
  {
    // right horizontal scroll 
    SSD1315_Write(true, 0x26);
  }
  
  SSD1315_Write(true, 0x00);            // Dummy byte (dont change)
  SSD1315_Write(true, start_line_no);   // Start page address
  SSD1315_Write(true, 0x00);            // 5 frames interval
  SSD1315_Write(true, end_line_no);     // End page address
  SSD1315_Write(true, 0x00);            // Dummy byte (dont change)
  SSD1315_Write(true, 0xFF);            // Dummy byte (dont change)
  SSD1315_Write(true, 0x2F);            // activate scroll
}

static void SSD1315_StartScrollVerticalHorizontal( bool is_vertical_left_scroll,
                                                   uint8_t start_line_no,
                                                   uint8_t end_line_no,
                                                   uint8_t vertical_area,
                                                   uint8_t rows
                                                 )
{
  
  SSD1315_Write(true, 0xA3);            // Set Vertical Scroll Area
  SSD1315_Write(true, 0x00);            // Check datasheet
  SSD1315_Write(true, vertical_area);   // area for vertical scroll
  
  if(is_vertical_left_scroll)
  {
    // vertical and left horizontal scroll
    SSD1315_Write(true, 0x2A);
  }
  else
  {
    // vertical and right horizontal scroll 
    SSD1315_Write(true, 0x29);
  }
  
  SSD1315_Write(true, 0x00);            // Dummy byte (dont change)
  SSD1315_Write(true, start_line_no);   // Start page address
  SSD1315_Write(true, 0x00);            // 5 frames interval
  SSD1315_Write(true, end_line_no);     // End page address
  SSD1315_Write(true, rows);            // Vertical scrolling offset
  SSD1315_Write(true, 0x2F);            // activate scroll
}


static int SSD1315_DisplayInit(void)
{
  msleep(100);               // delay
  /*
  ** Commands to initialize the SSD_1306 OLED Display
  */
  SSD1315_Write(true, 0xAE); // Entire Display OFF
  SSD1315_Write(true, 0xD5); // Set Display Clock Divide Ratio and Oscillator Frequency
  SSD1315_Write(true, 0x80); // Default Setting for Display Clock Divide Ratio and Oscillator Frequency that is recommended
  SSD1315_Write(true, 0xA8); // Set Multiplex Ratio
  SSD1315_Write(true, 0x3F); // 64 COM lines
  SSD1315_Write(true, 0xD3); // Set display offset
  SSD1315_Write(true, 0x00); // 0 offset
  SSD1315_Write(true, 0x40); // Set first line as the start line of the display
  SSD1315_Write(true, 0x8D); // Charge pump
  SSD1315_Write(true, 0x14); // Enable charge dump during display on
  SSD1315_Write(true, 0x20); // Set memory addressing mode
  SSD1315_Write(true, 0x00); // Horizontal addressing mode
  SSD1315_Write(true, 0xA1); // Set segment remap with column address 127 mapped to segment 0
  SSD1315_Write(true, 0xC8); // Set com output scan direction, scan from com63 to com 0
  SSD1315_Write(true, 0xDA); // Set com pins hardware configuration
  SSD1315_Write(true, 0x12); // Alternative com pin configuration, disable com left/right remap
  SSD1315_Write(true, 0x81); // Set contrast control
  SSD1315_Write(true, 0x80); // Set Contrast to 128
  SSD1315_Write(true, 0xD9); // Set pre-charge period
  SSD1315_Write(true, 0xF1); // Phase 1 period of 15 DCLK, Phase 2 period of 1 DCLK
  SSD1315_Write(true, 0xDB); // Set Vcomh deselect level
  SSD1315_Write(true, 0x20); // Vcomh deselect level ~ 0.77 Vcc
  SSD1315_Write(true, 0xA4); // Entire display ON, resume to RAM content display
  SSD1315_Write(true, 0xA6); // Set Display in Normal Mode, 1 = ON, 0 = OFF
  SSD1315_Write(true, 0x2E); // Deactivate scroll
  SSD1315_Write(true, 0xAF); // Display ON in normal mode
  
  //Clear the display
  SSD1315_Fill(0x00);
  return 0;
}

static void SSD1315_Fill(unsigned char data)
{
  unsigned int total  = 128 * 8;  // 8 pages x 128 segments x 8 bits of data
  unsigned int i      = 0;
  
  //Fill the Display
  for(i = 0; i < total; i++)
  {
      SSD1315_Write(false, data);
  }
}

/******************************************************************************************************/
/******************************************************************************************************/

#if 0
/* for <driver_name>_dt_ids */
static const struct of_device_id <driver_name>_dt_ids[] = {
    { .compatible = "<vendor>, <device_name>", },
    {}    
};
#endif

/* for oled_i2c_idtable */
static struct i2c_device_id oled_i2c_idtable[] = 
{
    { OLED_I2C_NAME, 0 },
    {}
};
MODULE_DEVICE_TABLE(i2c, oled_i2c_idtable);

/* for oled_i2c_probe func */
static int oled_i2c_probe(struct i2c_client* client, const struct i2c_device_id *id)
{
    pr_info("\n going to probe. ");
    /* perform initialization tasks for OLED display module*/

    if( strcmp(client->dev.driver->name, OLED_I2C_NAME) == 0)
    {
        /* save the client handle for later use*/
        oled_client = client;

        /* init  */
        SSD1315_DisplayInit();
        
        /* set ...*/
        SSD1315_SetCursor(0,0);  

        /* Display "Hallo world" to OLED */
        SSD1315_String("Hallo world\n");

    }

    pr_info("\n probeded successfully. ");

    /* return success */
    return 0;
}

/* for oled_i2c_remove func */
static int oled_i2c_remove(struct i2c_client* client)
{
    pr_info("\n going to remove. ");
    /* perform clean up for OLED display module */

    //fill the OLED with this data
    msleep(1000);
    
    //Set cursor
    SSD1315_SetCursor(0,0);  

    //clear the display
    SSD1315_Fill(0x00);
    
    SSD1315_Write(true, 0xAE); // Entire Display OFF

    pr_info("\n removed successfully. ");

    /* return success */
    return 0;
}

/******************************************************************************************************/

/* for our client driver: oled_i2c_driver */
static struct i2c_driver oled_i2c_driver = 
{
    .probe    = oled_i2c_probe,
    .remove   = oled_i2c_remove,
    .id_table = oled_i2c_idtable,
    .driver = {
        .name  = OLED_I2C_NAME,
        .owner = THIS_MODULE,
        //.of_match_table = of_match_ptr(<driver_name>_dt_ids),
    }
};

/* module init func */
static int __init oled_i2c_driver_init(void)
{
    pr_info("\n@frk: going to init...");
    // ...
    int ret;
    struct i2c_adapter* adapter;

    /* get I2C adapter corresponding to the desired bus number */
    adapter = i2c_get_adapter(OLED_I2C_BUS_AVAILABLE);
    if( !adapter ){
        pr_err("\n@frk: FAILED to get I2C adapter. ");
        return -ENODEV;
    }

    /* create I2c device using adapter & board*/
    oled_client = i2c_new_client_device(adapter, &oled_info);
    if ( !oled_client){
        pr_err("\n FAILED to create I2C device.");
        i2c_put_adapter(adapter);
        return -ENODEV;
    }

    /* register the driver with the kernel */
    ret = i2c_add_driver(&oled_i2c_driver);
    if (ret<0){
        pr_err("\n Error to register OLED-I2C driver. ");
        i2c_unregister_device(oled_client);
        i2c_put_adapter(adapter);
        return ret;
    }

    i2c_put_adapter(adapter);

    /* return success */
    pr_info("\n @frk: I2C-oled insert ... DONE!!! \n");
    return 0;
}

/* module exit func*/
static void __exit oled_i2c_driver_exit(void)
{
    pr_info("\n@FRK: going to remove...");
    
    /* unregister the driver from kernel */ 
    i2c_del_driver(&oled_i2c_driver);

    /* unregister the device from kernel*/
    i2c_unregister_device(oled_client);

    /* return success */
    pr_info("\n @frk: I2C-oled remove ... DONE!!! \n");
}

/******************************************************************************************************/
module_init(oled_i2c_driver_init);
module_exit(oled_i2c_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("FRANK <frank@bos-semi.com>");
MODULE_DESCRIPTION("I2C OLED DRIVER");
MODULE_VERSION("1.8");

/******************************************************************************************************/

