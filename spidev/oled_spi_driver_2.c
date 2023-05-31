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

/* */
#define OLED_SPI_NAME           "oled_ssd1315"    // 
#define OLED_SPI_BUS_AVAILABLE  0                 // SPI0 bus is used.
#define SSD1315_RST_PIN         (  24 )           // RST (Reset pin), GPIO 24
#define SSD1315_DC_PIN          (  23 )           // DC (Data/Command pin), GPIO 23

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

/******************************************************************************************************/
/* SSD1315-OLED headers */
/******************************************************************************************************/
#define SSD1315_MAX_SEG         ( 132 )           // Maximum segment

#define SSD1315_MAX_LINE        (   7 )           // Maximum line
#define SSD1315_DEF_FONT_SIZE   (   5 )           // Default font size

/*
** Variable to store Line Number and Cursor Position.
*/ 
static uint8_t SSD1315_LineNum   = 0;
static uint8_t SSD1315_CursorPos = 0;
static uint8_t SSD1315_FontSize  = SSD1315_DEF_FONT_SIZE;

static void ETX_SSD1315_fill( uint8_t data );

/* OLED EED1315 APIs, from EmbedTronix */
/****************************************************************************
 * Name: etx_spi_write
 *
 * Details : This function writes the 1-byte data to the slave device using SPI.
 ****************************************************************************/
int etx_spi_write( uint8_t data )
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
**  EmbeTronicX Logo
*/
static const uint8_t etx_logo[1024] = {
  0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xE1, 0xF9, 0xFF, 0xF9, 0xE1, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF,
  0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xF8, 0xFF, 0x0F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF,
  0xF8, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
  0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xFF, 0xFF, 0x0F, 0xF8, 0xF7, 0x00, 0xBF, 0xC0, 0x7F,
  0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
  0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x3F, 0xE0, 0x0F, 0x7F, 0x00, 0xFF, 0x7F, 0x80,
  0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x04, 0xFC, 0xFC, 0x0C, 0x0C, 0x0C, 0x0C, 0x7C, 0x00, 0x00, 0x00,
  0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x04, 0xFC, 0xF8,
  0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x08,
  0x04, 0x04, 0xFC, 0xFC, 0x04, 0x04, 0x04, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x98, 0x98, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x00, 0x00, 0x04, 0x0C, 0x38, 0xE0, 0x80, 0xE0, 0x38, 0x0C, 0x04, 0x00, 0x00, 0x00, 0x00, 0xFF,
  0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xC0, 0x3F, 0xFF, 0xFF, 0x00, 0xFD, 0xFE, 0xFF,
  0x01, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x06, 0x06, 0x06, 0x06, 0xE0, 0x00, 0x00, 0x00,
  0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
  0x00, 0x00, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x7C, 0xFF, 0x11, 0x10, 0x1F, 0x1F, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x7C, 0xFF, 0x01, 0x00, 0x01, 0xFF, 0x7C, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
  0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x87,
  0x00, 0x00, 0x00, 0x00, 0xE0, 0x3F, 0x1F, 0xFF, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
  0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x1F, 0x3F, 0xFC, 0xF7, 0x00, 0xDF, 0xE3, 0x7D,
  0x3E, 0x07, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00,
  0x00, 0x03, 0x03, 0x00, 0x03, 0x03, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03,
  0x02, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x02, 0x02, 0x01, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x01, 0x03, 0x02, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x03,
  0x03, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x02, 0x02, 0x03,
  0x00, 0x00, 0x02, 0x03, 0x01, 0x00, 0x00, 0x00, 0x01, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0xFF,
  0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0x00, 0xFF, 0x03, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x88, 0x88, 0x08, 0x00, 0xE0, 0x20, 0x20, 0xE0, 0x20, 0xE0,
  0x00, 0xFC, 0x20, 0x20, 0xE0, 0x00, 0xE0, 0x20, 0x20, 0xE0, 0x00, 0xE0, 0x20, 0x20, 0xFC, 0x00,
  0xE0, 0x20, 0x20, 0xFC, 0x00, 0xE0, 0x20, 0x20, 0xE0, 0x00, 0xE0, 0x20, 0x20, 0xFC, 0x00, 0x00,
  0x00, 0x08, 0x08, 0xF8, 0x08, 0x08, 0x00, 0xE0, 0x00, 0xE0, 0x00, 0xFC, 0x20, 0x20, 0x00, 0xE0,
  0x20, 0x20, 0xE0, 0x00, 0xE0, 0x20, 0x20, 0x00, 0xEC, 0x00, 0x20, 0x20, 0x20, 0xE0, 0x00, 0xFC,
  0x00, 0xE0, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0xC8, 0x38, 0x00, 0x00, 0xE0,
  0x20, 0x20, 0xE0, 0x00, 0xE0, 0x20, 0xE0, 0x00, 0xE0, 0x20, 0x20, 0xE0, 0x00, 0x00, 0x00, 0xFF,
  0xFF, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x9C, 0x9C, 0x88, 0x8E, 0x83, 0x98, 0x83, 0x8E, 0x88,
  0x88, 0x9C, 0x9C, 0x80, 0x80, 0x87, 0x84, 0x84, 0x84, 0x80, 0x87, 0x80, 0x80, 0x87, 0x80, 0x87,
  0x80, 0x87, 0x84, 0x84, 0x87, 0x80, 0x87, 0x85, 0x85, 0x85, 0x80, 0x87, 0x84, 0x84, 0x87, 0x80,
  0x87, 0x84, 0x84, 0x87, 0x80, 0x87, 0x85, 0x85, 0x85, 0x80, 0x87, 0x84, 0x84, 0x87, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x87, 0x80, 0x80, 0x80, 0x87, 0x84, 0x87, 0x80, 0x87, 0x84, 0x84, 0x80, 0x87,
  0x84, 0x84, 0x87, 0x80, 0x87, 0x80, 0x80, 0x80, 0x87, 0x80, 0x87, 0x85, 0x85, 0x87, 0x80, 0x87,
  0x80, 0x85, 0x85, 0x85, 0x87, 0x80, 0x80, 0x80, 0x80, 0x86, 0x85, 0x84, 0x84, 0x84, 0x80, 0x87,
  0x84, 0x84, 0x87, 0x80, 0x87, 0x80, 0x87, 0x80, 0x87, 0x85, 0x85, 0x85, 0x80, 0x80, 0x80, 0xFF
};

/*
** Array Variable to store the letters.
*/ 
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
/****************************************************************************
 * Name: ETX_SSD1315_ResetDcInit
 *
 * Details : This function Initializes and Configures the Reset and DC Pin
 ****************************************************************************/
static int ETX_SSD1315_ResetDcInit( void )
{
  int ret = 0;
    
  //do while(false) loop to break if any error
  do
  {
    
    /* Register the Reset GPIO */
    
    //Checking the Reset GPIO is valid or not
    if( gpio_is_valid( SSD1315_RST_PIN ) == false )
    {
      pr_err("Reset GPIO %d is not valid\n", SSD1315_RST_PIN);
      ret = -1;
      break;
    }
    
    //Requesting the Reset GPIO
    if( gpio_request( SSD1315_RST_PIN, "SSD1315_RST_PIN" ) < 0 )
    {
      pr_err("ERROR: Reset GPIO %d request\n", SSD1315_RST_PIN);
      ret = -1;
      break;
    }
    
    //configure the Reset GPIO as output
    gpio_direction_output( SSD1315_RST_PIN, 1 );
    
    /* Register the DC GPIO */
    
    //Checking the DC GPIO is valid or not
    if( gpio_is_valid( SSD1315_DC_PIN ) == false )
    {
      pr_err("DC GPIO %d is not valid\n", SSD1315_DC_PIN);
      gpio_free( SSD1315_RST_PIN );   // free the reset GPIO
      ret = -1;
      break;
    }
    
    //Requesting the DC GPIO
    if( gpio_request( SSD1315_DC_PIN, "SSD1315_DC_PIN" ) < 0 )
    {
      pr_err("ERROR: DC GPIO %d request\n", SSD1315_DC_PIN);
      gpio_free( SSD1315_RST_PIN );   // free the reset GPIO
      ret = -1;
      break;
    }
    
    //configure the Reset GPIO as output
    gpio_direction_output( SSD1315_DC_PIN, 1 );
    
  } while( false );
  
  //pr_info("DC Reset GPIOs init Done!\n");
  return( ret );
}

/****************************************************************************
 * Name: ETX_SSD1315_ResetDcDeInit
 *
 * Details : This function De-initializes the Reset and DC Pin
 ****************************************************************************/
static void ETX_SSD1315_ResetDcDeInit( void )
{
  gpio_free( SSD1315_RST_PIN );   // free the reset GPIO
  gpio_free( SSD1315_DC_PIN );    // free the DC GPIO
}

/****************************************************************************
 * Name: ETX_SSD1315_setRst
 *
 * Details : This function writes the value to the Reset GPIO
 * 
 * Argument: 
 *            value - value to be set ( 0 or 1 )
 * 
 ****************************************************************************/
static void ETX_SSD1315_setRst( uint8_t value )
{
  gpio_set_value( SSD1315_RST_PIN, value );
}

/****************************************************************************
 * Name: ETX_SSD1315_setDc
 *
 * Details : This function writes the value to the DC GPIO
 * 
 * Argument: 
 *            value - value to be set ( 0 or 1 )
 * 
 ****************************************************************************/
static void ETX_SSD1315_setDc( uint8_t value )
{
  gpio_set_value( SSD1315_DC_PIN, value );
}

/****************************************************************************
 * Name: ETX_SSD1315_Write
 *
 * Details : This function sends the command/data to the Display
 *
 * Argument: is_cmd
 *              true  - if we need to send command
 *              false - if we need to send data
 *           value
 *              value to be transmitted
 ****************************************************************************/
static int ETX_SSD1315_Write( bool is_cmd, uint8_t data )
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
  
  ETX_SSD1315_setDc( pin_value );
  
  //pr_info("Writing 0x%02X \n", data);
  
  //send the byte
  ret = etx_spi_write( data );
  
  return( ret );
}

/****************************************************************************
 * Name: ETX_SSD1315_SetCursor
 *
 * Details : This function is specific to the SSD_1306 OLED.
 *
 * Argument:
 *              lineNo    -> Line Number
 *              cursorPos -> Cursor Position
 * 
 ****************************************************************************/
void ETX_SSD1315_SetCursor( uint8_t lineNo, uint8_t cursorPos )
{
  pr_info("\n move to SetCursor");  

  /* Move the Cursor to specified position only if it is in range */
  if((lineNo <= SSD1315_MAX_LINE) && (cursorPos < SSD1315_MAX_SEG))
  {
    pr_info("\n move to inside SetCursor or not");

    SSD1315_LineNum   = lineNo;                    // Save the specified line number
    SSD1315_CursorPos = cursorPos;                 // Save the specified cursor position
    
    /* set page address */
    ETX_SSD1315_Write(true, 0xB0 | lineNo);

    /* set column address */
    ETX_SSD1315_Write(true, 0x00 | (cursorPos&0x0F));            // column start addr
    ETX_SSD1315_Write(true, 0x10 | (cursorPos>>4 + 0x10));    // column end addr

  }
}

/****************************************************************************
 * Name: ETX_SSD1315_GoToNextLine
 *
 * Details : This function is specific to the SSD_1306 OLED and move the cursor 
 *           to the next line.
 ****************************************************************************/
void ETX_SSD1315_GoToNextLine( void )
{
  /*
  ** Increment the current line number.
  ** roll it back to first line, if it exceeds the limit. 
  */
  pr_info("\n @frk: move to GoToNextLine_1");

  SSD1315_LineNum++;
  pr_info("\n @frk: move to GoToNextLine_2");

  SSD1315_LineNum = (SSD1315_LineNum & SSD1315_MAX_LINE);
  pr_info("\n @frk: move to GoToNextLine_3");  

  ETX_SSD1315_SetCursor(SSD1315_LineNum,0); /* Finally move it to next line */
  pr_info("\n @frk: move to GoToNextLine_4");
}

/****************************************************************************
 * Name: ETX_SSD1315_PrintChar
 *
 * Details : This function is specific to the SSD_1306 OLED and sends 
 *           the single char to the OLED.
 * 
 * Arguments:
 *           c   -> character to be written
 * 
 ****************************************************************************/
void ETX_SSD1315_PrintChar( unsigned char c )
{
  uint8_t data_byte;
  uint8_t temp = 0;
  
  pr_info("\n @frk: go into PrintChar");

  /*
  ** If we character is greater than segment len or we got new line charcter
  ** then move the cursor to the new line
  */ 
  if( (( SSD1315_CursorPos + SSD1315_FontSize ) >= SSD1315_MAX_SEG ) ||
      ( c == '\n' )
  )
  {
    pr_info("\n @frk: go into If condition in PrintChar");
    ETX_SSD1315_GoToNextLine();
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
      data_byte= SSD1315_font[c][temp];         // Get the data to be displayed from LookUptable
      ETX_SSD1315_Write(false, data_byte);  // write data to the OLED
      SSD1315_CursorPos++;
      
      temp++;
      
    } while ( temp < SSD1315_FontSize);
    
    ETX_SSD1315_Write(false, 0x00);         //Display the data
    SSD1315_CursorPos++;
  }
}

/****************************************************************************
 * Name: ETX_SSD1315_String
 *
 * Details : This function is specific to the SSD_1306 OLED and sends 
 *           the string to the OLED.
 * 
 * Arguments:
 *           str   -> string to be written
 * 
 ****************************************************************************/
void ETX_SSD1315_String(char *str)
{
  while( *str )
  {
    ETX_SSD1315_PrintChar(*str++);
  }
}

/****************************************************************************
 * Name: ETX_SSD1315_InvertDisplay
 *
 * Details : This function is specific to the SSD_1306 OLED and 
 *           inverts the display.
 * 
 * Arguments:
 *           need_to_invert   -> true  - invert display
 *                               false - normal display 
 * 
 ****************************************************************************/
void ETX_SSD1315_InvertDisplay(bool need_to_invert)
{
  if(need_to_invert)
  {
    ETX_SSD1315_Write(true, 0xA7); // Invert the display
  }
  else
  {
    ETX_SSD1315_Write(true, 0xA6); // Normal display
  }
}

/****************************************************************************
 * Name: ETX_SSD1315_SetBrightness
 *
 * Details : This function is specific to the SSD_1306 OLED and 
 *           sets the brightness of  the display.
 * 
 * Arguments:
 *           brightnessValue   -> brightness value ( 0 - 255 )
 * 
 ****************************************************************************/
void ETX_SSD1315_SetBrightness(uint8_t brightnessValue)
{
    ETX_SSD1315_Write(true, 0x81);            // Contrast command
    ETX_SSD1315_Write(true, brightnessValue); // Contrast value (default value = 0x7F)
}

/****************************************************************************
 * Name: ETX_SSD1315_StartScrollHorizontal
 *
 * Details : This function is specific to the SSD_1306 OLED and 
 *           Scrolls the data right/left in horizontally.
 * 
 * Arguments:
 *           is_left_scroll   -> true  - left horizontal scroll
 *                               false - right horizontal scroll
 *           start_line_no    -> Start address of the line to scroll 
 *           end_line_no      -> End address of the line to scroll 
 * 
 ****************************************************************************/
void ETX_SSD1315_StartScrollHorizontal( bool is_left_scroll,
                                        uint8_t start_line_no,
                                        uint8_t end_line_no
                                      )
{
  if(is_left_scroll)
  {
    // left horizontal scroll
    ETX_SSD1315_Write(true, 0x27);
  }
  else
  {
    // right horizontal scroll 
    ETX_SSD1315_Write(true, 0x26);
  }
  
  ETX_SSD1315_Write(true, 0x00);            // Dummy byte (dont change)
  ETX_SSD1315_Write(true, start_line_no);   // Start page address
  ETX_SSD1315_Write(true, 0x00);            // 5 frames interval
  ETX_SSD1315_Write(true, end_line_no);     // End page address
  ETX_SSD1315_Write(true, 0x00);            // Dummy byte (dont change)
  ETX_SSD1315_Write(true, 0xFF);            // Dummy byte (dont change)
  ETX_SSD1315_Write(true, 0x2F);            // activate scroll
}

/****************************************************************************
 * Name: ETX_SSD1315_StartScrollVerticalHorizontal
 *
 * Details : This function is specific to the SSD_1306 OLED and 
 *           Scrolls the data in vertically and right/left horizontally
 *           (Diagonally).
 * 
 * Arguments:
 *      is_vertical_left_scroll -> true  - vertical and left horizontal scroll
 *                                 false - vertical and right horizontal scroll
 *      start_line_no           -> Start address of the line to scroll 
 *      end_line_no             -> End address of the line to scroll 
 *      vertical_area           -> Area for vertical scroll (0-63)
 *      rows                    -> Number of rows to scroll vertically 
 * 
 ****************************************************************************/
void ETX_SSD1315_StartScrollVerticalHorizontal( 
                                                bool is_vertical_left_scroll,
                                                uint8_t start_line_no,
                                                uint8_t end_line_no,
                                                uint8_t vertical_area,
                                                uint8_t rows
                                              )
{
  
  ETX_SSD1315_Write(true, 0xA3);            // Set Vertical Scroll Area
  ETX_SSD1315_Write(true, 0x00);            // Check datasheet
  ETX_SSD1315_Write(true, vertical_area);   // area for vertical scroll
  
  if(is_vertical_left_scroll)
  {
    // vertical and left horizontal scroll
    ETX_SSD1315_Write(true, 0x2A);
  }
  else
  {
    // vertical and right horizontal scroll 
    ETX_SSD1315_Write(true, 0x29);
  }
  
  ETX_SSD1315_Write(true, 0x00);            // Dummy byte (dont change)
  ETX_SSD1315_Write(true, start_line_no);   // Start page address
  ETX_SSD1315_Write(true, 0x00);            // 5 frames interval
  ETX_SSD1315_Write(true, end_line_no);     // End page address
  ETX_SSD1315_Write(true, rows);            // Vertical scrolling offset
  ETX_SSD1315_Write(true, 0x2F);            // activate scroll
}

/****************************************************************************
 * Name: ETX_SSD1315_DeactivateScroll
 *
 * Details : This function disables the scroll.
 ****************************************************************************/
void ETX_SSD1315_DeactivateScroll( void )
{
  ETX_SSD1315_Write(true, 0x2E); // Deactivate scroll
}

/****************************************************************************
 * Name: ETX_SSD1315_fill
 *
 * Details : This function fills the data to the Display
 ****************************************************************************/
void ETX_SSD1315_fill( uint8_t data )
{
  // 8 pages x 128 segments x 8 bits of data
  unsigned int total  = ( SSD1315_MAX_SEG * (SSD1315_MAX_LINE + 1) );

  //unsigned int total  = ( 132 * 8 );

  unsigned int i      = 0;
  
  //Fill the Display
  for(i = 0; i < total; i++)
  {
    ETX_SSD1315_Write(false, data);
  }
}

/****************************************************************************
 * Name: ETX_SSD1315_ClearDisplay
 *
 * Details : This function clears the Display
 ****************************************************************************/
void ETX_SSD1315_ClearDisplay( void )
{
  //Set cursor
  ETX_SSD1315_SetCursor(0,0);
  
  ETX_SSD1315_fill( 0x00 );
}

/****************************************************************************
 * Name: ETX_SSD1315_PrintLogo
 *
 * Details : This function prints the EmbeTronicX Logo
 ****************************************************************************/
void ETX_SSD1315_PrintLogo( void )
{
  int i;
  
  //Set cursor
  ETX_SSD1315_SetCursor(0,0);
  
//   for( i = 0; i < ( SSD1315_MAX_SEG * (SSD1315_MAX_LINE + 1) ); i++ )
//   {
//     ETX_SSD1315_Write(false, etx_logo[i]);
//   }
}

/****************************************************************************
 * Name: ETX_SSD1315_DisplayInit
 *
 * Details : This function Initializes the Display
 ****************************************************************************/
int ETX_SSD1315_DisplayInit(void)
{
  int ret = 0;

  //Initialize the Reset and DC GPIOs
  ret = ETX_SSD1315_ResetDcInit();

  if( ret >= 0 )
  {
    //Make the RESET Line to 0
    ETX_SSD1315_setRst( 0u );
    msleep(100);                          // delay
    //Make the DC Line to 1
    ETX_SSD1315_setRst( 1u );
    msleep(100);                          // delay

#if 1
    /*
    ** Commands to initialize the SSD_1315 OLED Display
    */
    ETX_SSD1315_Write(true, 0xAE); // Entire Display OFF
    ETX_SSD1315_Write(true, 0xD5); // Set Display Clock Divide Ratio and Oscillator Frequency
    ETX_SSD1315_Write(true, 0x80); // Default Setting for Display Clock Divide Ratio and Oscillator Frequency that is recommended
    ETX_SSD1315_Write(true, 0xA8); // Set Multiplex Ratio
    ETX_SSD1315_Write(true, 0x3F); // 64 COM lines
    ETX_SSD1315_Write(true, 0xD3); // Set display offset
    ETX_SSD1315_Write(true, 0x00); // 0 offset
    ETX_SSD1315_Write(true, 0x40); // Set first line as the start line of the display
    ETX_SSD1315_Write(true, 0xAD); // Charge pump
    ETX_SSD1315_Write(true, 0x8B); // Enable charge dump during display on
    ETX_SSD1315_Write(true, 0xA1); // Set segment remap with column address 127 mapped to segment 0
    ETX_SSD1315_Write(true, 0xC8); // Set com output scan direction, scan from com63 to com 0
    ETX_SSD1315_Write(true, 0xDA); // Set com pins hardware configuration
    ETX_SSD1315_Write(true, 0x12); // Alternative com pin configuration, disable com left/right remap
    ETX_SSD1315_Write(true, 0x81); // Set contrast control
    ETX_SSD1315_Write(true, 0xBF); // Set Contrast to 128
    ETX_SSD1315_Write(true, 0xD9); // Set pre-charge period
    ETX_SSD1315_Write(true, 0x22); // Phase 1 period of 15 DCLK, Phase 2 period of 1 DCLK 
    ETX_SSD1315_Write(true, 0xDB); // Set Vcomh deselect level
    ETX_SSD1315_Write(true, 0x40); // Vcomh deselect level ~ 0.77 Vcc
    ETX_SSD1315_Write(true, 0x32); // Set VPP
    ETX_SSD1315_Write(true, 0xA6); // Set Display in Normal Mode, 1 = ON, 0 = OFF
    ETX_SSD1315_Write(true, 0xAF); // Display ON in normal mode
    
#endif

    // Clear the display
    ETX_SSD1315_ClearDisplay();
  }
    
  return( ret );
}

/****************************************************************************
 * Name: ETX_SSD1315_DisplayDeInit
 *
 * Details : This function De-initializes the Display
 ****************************************************************************/
void ETX_SSD1315_DisplayDeInit(void)
{
  ETX_SSD1315_ResetDcDeInit();  //Free the Reset and DC GPIO
}

/**
*   @frk: to develop some funcs based on APIs
*/

/* to display some shapes */
void display_rectangle(int len)
{
        int i;

        /* set ...*/
        ETX_SSD1315_SetCursor(0,0);  
        /* Display "Hallo world" to OLED */
        ETX_SSD1315_String("**********************\n");

        for ( i=1; i<len; i++)
        {
          ETX_SSD1315_SetCursor(i,0);  
          /* Display "Hallo world" to OLED */
          ETX_SSD1315_String("*                    *\n");
        }

        /* set ...*/
        ETX_SSD1315_SetCursor(len,0);  
        /* Display "Hallo world" to OLED */
        ETX_SSD1315_String("**********************\n");

}

/* display frank */
void display_frank(void)
{
  ETX_SSD1315_SetCursor(3,15);
  ETX_SSD1315_String("BOS/ SW/ Frank\n");
  //ETX_SSD1315_String("Hallo world!!!!");
}

/******************************************************************************************************/
/* module init func */
static int __init oled_spi_driver_init(void)
{
    pr_info("\n@frk: going to init...");

#if 0
    /* to reset the SPI: write 0 into RESET pin */    
    pr_info("\n@FRK: going to reset OLED.");
    ETX_SSD1315_setRst( 0u );

    return 0;
#endif

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

/* SSD1315 APIs here */

#if 1
        pr_info("\n@FRK: going to display DATA by OLED APIs.");    

        /* init  */
        ETX_SSD1315_DisplayInit();

        /* display rectangle */
        display_rectangle(7);       

        /* display Frank*/
        display_frank();
#endif

/* return success */
    pr_info("\n @frk: SPI-oled insert ... DONE!!! \n");
    return 0;
}

/* module exit func*/
static void __exit oled_spi_driver_exit(void)
{
    pr_info("\n@frk: going to remove...");

    pr_info("\n@FRK: going to reset OLED.");   
    ETX_SSD1315_setRst( 0u );

    pr_info("\n#FRK: going to clean screen by OLED API.");

    msleep(1000);

/* SSD1315 APIs here */
    /* Clear the display */
    ETX_SSD1315_ClearDisplay();                 // Clear Display
    ETX_SSD1315_DisplayDeInit();                // Deinit the SSD1315

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
