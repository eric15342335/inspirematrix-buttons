// ===================================================================================
// SSD1306 128x64 Pixels OLED Minimal Functions                               * v1.0 *
// ===================================================================================
//
// Collection of the most necessary functions for controlling an SSD1306 128x64 pixels
// I2C OLED.
//
// References:
// -----------
// - TinyOLEDdemo: https://github.com/wagiminator/attiny13-tinyoleddemo
// - Neven Boyanov: https://github.com/tinusaur/ssd1306xled
// - Stephen Denne: https://github.com/datacute/Tiny4kOLED
//
// 2022 by Stefan Wagner: https://github.com/wagiminator

#include "oled_min.h"

// OLED initialisation sequence
const uint8_t OLED_INIT_CMD[] = {
  OLED_MULTIPLEX,   0x3F,                 // set multiplex ratio  
  OLED_CHARGEPUMP,  0x14,                 // set DC-DC enable  
  OLED_MEMORYMODE,  0x00,                 // set horizontal addressing mode
  OLED_COLUMNS,     0x00, 0x7F,           // set start and end column
  OLED_PAGES,       0x00, 0x3F,           // set start and end page
  OLED_COMPINS,     0x12,                 // set com pins
  OLED_XFLIP, OLED_YFLIP,                 // flip screen
  OLED_DISPLAY_ON                         // display on
};

// OLED init function
void OLED_init(void) {
  uint8_t i;
  I2C_init();                             // initialize I2C first
  I2C_start(OLED_ADDR);                   // start transmission to OLED
  I2C_write(OLED_CMD_MODE);               // set command mode
  for(i = 0; i < sizeof(OLED_INIT_CMD); i++)
    I2C_write(OLED_INIT_CMD[i]);          // send the command bytes
  I2C_stop();                             // stop transmission
}

// Start sending data
void OLED_data_start(void) {
  I2C_start(OLED_ADDR);                   // start transmission to OLED
  I2C_write(OLED_DAT_MODE);               // set data mode
}

// Start sending command
void OLED_command_start(void) {
  I2C_start(OLED_ADDR);                   // start transmission to OLED
  I2C_write(OLED_CMD_MODE);               // set command mode
}

// OLED send command
void OLED_send_command(uint8_t cmd) {
  I2C_start(OLED_ADDR);                   // start transmission to OLED
  I2C_write(OLED_CMD_MODE);               // set command mode
  I2C_write(cmd);                         // send command
  I2C_stop();                             // stop transmission
}

// OLED set cursor position
void OLED_setpos(uint8_t x, uint8_t y) {
  I2C_start(OLED_ADDR);                   // start transmission to OLED
  I2C_write(OLED_CMD_MODE);               // set command mode
  I2C_write(OLED_PAGE | y);	              // set page start address
  I2C_write(x & 0x0F);			              // set lower nibble of start column
  I2C_write(OLED_COLUMN_HIGH | (x >> 4)); // set higher nibble of start column
  I2C_stop();                             // stop transmission
}

// OLED fill screen
void OLED_fill(uint8_t p) {
  OLED_setpos(0, 0);                      // set cursor to display start
  I2C_start(OLED_ADDR);                   // start transmission to OLED
  I2C_write(OLED_DAT_MODE);               // set data mode
  for(uint16_t i=128*8; i; i--) I2C_write(p); // send pattern
  I2C_stop();                             // stop transmission
}

// OLED draw bitmap
void OLED_draw_bmp(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t* bmp) {
  for(uint8_t y = y0; y < y1; y++) {
    OLED_setpos(x0, y);
    I2C_start(OLED_ADDR);
    I2C_write(OLED_DAT_MODE);
    for(uint8_t x = x0; x < x1; x++)
      I2C_write(*bmp++);
    I2C_stop();
  }
}

const uint8_t OLED_FONT[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2F, 0x00, 0x00, 0x00, 0x07, 0x00, 0x07, 0x00,
  0x14, 0x7F, 0x14, 0x7F, 0x14, 0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x23, 0x13, 0x08, 0x64, 0x62,
  0x36, 0x49, 0x55, 0x22, 0x50, 0x00, 0x05, 0x03, 0x00, 0x00, 0x00, 0x1C, 0x22, 0x41, 0x00,
  0x00, 0x41, 0x22, 0x1C, 0x00, 0x14, 0x08, 0x3E, 0x08, 0x14, 0x08, 0x08, 0x3E, 0x08, 0x08,
  0x00, 0x00, 0xA0, 0x60, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x60, 0x60, 0x00, 0x00,
  0x20, 0x10, 0x08, 0x04, 0x02, 0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00, 0x42, 0x7F, 0x40, 0x00,
  0x42, 0x61, 0x51, 0x49, 0x46, 0x21, 0x41, 0x45, 0x4B, 0x31, 0x18, 0x14, 0x12, 0x7F, 0x10,
  0x27, 0x45, 0x45, 0x45, 0x39, 0x3C, 0x4A, 0x49, 0x49, 0x30, 0x01, 0x71, 0x09, 0x05, 0x03,
  0x36, 0x49, 0x49, 0x49, 0x36, 0x06, 0x49, 0x49, 0x29, 0x1E, 0x00, 0x36, 0x36, 0x00, 0x00,
  0x00, 0x56, 0x36, 0x00, 0x00, 0x08, 0x14, 0x22, 0x41, 0x00, 0x14, 0x14, 0x14, 0x14, 0x14,
  0x00, 0x41, 0x22, 0x14, 0x08, 0x02, 0x01, 0x51, 0x09, 0x06, 0x32, 0x49, 0x59, 0x51, 0x3E,
  0x7C, 0x12, 0x11, 0x12, 0x7C, 0x7F, 0x49, 0x49, 0x49, 0x36, 0x3E, 0x41, 0x41, 0x41, 0x22,
  0x7F, 0x41, 0x41, 0x22, 0x1C, 0x7F, 0x49, 0x49, 0x49, 0x41, 0x7F, 0x09, 0x09, 0x09, 0x01,
  0x3E, 0x41, 0x49, 0x49, 0x7A, 0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00, 0x41, 0x7F, 0x41, 0x00,
  0x20, 0x40, 0x41, 0x3F, 0x01, 0x7F, 0x08, 0x14, 0x22, 0x41, 0x7F, 0x40, 0x40, 0x40, 0x40,
  0x7F, 0x02, 0x0C, 0x02, 0x7F, 0x7F, 0x04, 0x08, 0x10, 0x7F, 0x3E, 0x41, 0x41, 0x41, 0x3E,
  0x7F, 0x09, 0x09, 0x09, 0x06, 0x3E, 0x41, 0x51, 0x21, 0x5E, 0x7F, 0x09, 0x19, 0x29, 0x46,
  0x46, 0x49, 0x49, 0x49, 0x31, 0x01, 0x01, 0x7F, 0x01, 0x01, 0x3F, 0x40, 0x40, 0x40, 0x3F,
  0x1F, 0x20, 0x40, 0x20, 0x1F, 0x3F, 0x40, 0x38, 0x40, 0x3F, 0x63, 0x14, 0x08, 0x14, 0x63,
  0x07, 0x08, 0x70, 0x08, 0x07, 0x61, 0x51, 0x49, 0x45, 0x43, 0x00, 0x7F, 0x41, 0x41, 0x00,
  0x02, 0x04, 0x08, 0x10, 0x20, 0x00, 0x41, 0x41, 0x7F, 0x00, 0x04, 0x02, 0x01, 0x02, 0x04,
  0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 0x01, 0x02, 0x04, 0x00, 0x20, 0x54, 0x54, 0x54, 0x78,
  0x7F, 0x48, 0x44, 0x44, 0x38, 0x38, 0x44, 0x44, 0x44, 0x20, 0x38, 0x44, 0x44, 0x48, 0x7F,
  0x38, 0x54, 0x54, 0x54, 0x18, 0x08, 0x7E, 0x09, 0x01, 0x02, 0x18, 0xA4, 0xA4, 0xA4, 0x7C,
  0x7F, 0x08, 0x04, 0x04, 0x78, 0x00, 0x44, 0x7D, 0x40, 0x00, 0x40, 0x80, 0x84, 0x7D, 0x00,
  0x7F, 0x10, 0x28, 0x44, 0x00, 0x00, 0x41, 0x7F, 0x40, 0x00, 0x7C, 0x04, 0x18, 0x04, 0x78,
  0x7C, 0x08, 0x04, 0x04, 0x78, 0x38, 0x44, 0x44, 0x44, 0x38, 0xFC, 0x24, 0x24, 0x24, 0x18,
  0x18, 0x24, 0x24, 0x18, 0xFC, 0x7C, 0x08, 0x04, 0x04, 0x08, 0x48, 0x54, 0x54, 0x54, 0x20,
  0x04, 0x3F, 0x44, 0x40, 0x20, 0x3C, 0x40, 0x40, 0x20, 0x7C, 0x1C, 0x20, 0x40, 0x20, 0x1C,
  0x3C, 0x40, 0x30, 0x40, 0x3C, 0x44, 0x28, 0x10, 0x28, 0x44, 0x1C, 0xA0, 0xA0, 0xA0, 0x7C,
  0x44, 0x64, 0x54, 0x4C, 0x44, 0x08, 0x36, 0x41, 0x41, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x00,
  0x00, 0x41, 0x41, 0x36, 0x08, 0x08, 0x04, 0x08, 0x10, 0x08, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};


// OLED global variables
uint8_t line, column, scroll;

// OLED set cursor to line start
void OLED_setline(uint8_t line) {
  I2C_start(OLED_ADDR);                   // start transmission to OLED
  I2C_write(OLED_CMD_MODE);               // set command mode
  I2C_write(OLED_PAGE + line);            // set line
  I2C_write(0x00); I2C_write(0x10);       // set column to "0"
  I2C_stop();                             // stop transmission
}

// OLED clear line
void OLED_clearline(uint8_t line) {
  uint8_t i;
  OLED_setline(line);                     // set cursor to line start
  I2C_start(OLED_ADDR);                   // start transmission to OLED
  I2C_write(OLED_DAT_MODE);               // set data mode
  for(i=128; i; i--) I2C_write(0x00);     // clear the line
  I2C_stop();                             // stop transmission
}

// OLED clear screen
void OLED_clear(void) {
  uint8_t i;
  for(i=0; i<8; i++) OLED_clearline(i);
  line = scroll;
  column = 0;
  OLED_setline((line + scroll) & 0x07);
}

// OLED clear the top line, then scroll the display up by one line
void OLED_scrollDisplay(void) {
  OLED_clearline(scroll);                 // clear line
  scroll = (scroll + 1) & 0x07;           // set next line
  I2C_start(OLED_ADDR);                   // start transmission to OLED
  I2C_write(OLED_CMD_MODE);               // set command mode
  I2C_write(OLED_OFFSET);                 // set display offset:
  I2C_write(scroll << 3);                 // scroll up
  I2C_stop();                             // stop transmission
}


// OLED plot a single character
void OLED_plotChar(char c) {
  uint8_t i;
  uint16_t ptr = c - 32;                  // character pointer
  ptr += ptr << 2;                        // -> ptr = (ch - 32) * 5;
  I2C_start(OLED_ADDR);                   // start transmission to OLED
  I2C_write(OLED_DAT_MODE);               // set data mode
  for(i=5 ; i; i--) I2C_write(OLED_FONT[ptr++]);
  I2C_write(0x00);                        // write space between characters
  I2C_stop();                             // stop transmission
}

// OLED write a character or handle control characters
void OLED_write(char c) {
  c = c & 0x7F;                           // ignore top bit
  // normal character
  if(c >= 32) {
    OLED_plotChar(c);
    if(++column > 20) {
      column = 0;
      if(line == 7) OLED_scrollDisplay();
      else line++;
      OLED_setline((line + scroll) & 0x07);
    }
  }
  // new line
  else if(c == '\n') {
    column = 0;
    if(line == 7) OLED_scrollDisplay();
    else line++;
    OLED_setline((line + scroll) & 0x07);
  }
  // carriage return
  else if(c == '\r') {
    column = 0;
    OLED_setline((line + scroll) & 0x07);
  }
}

// OLED print string
void OLED_print(char* str) {
  while(*str) OLED_write(*str++);
}

// OLED print string with newline
void OLED_println(char* str) {
  OLED_print(str);
  OLED_write('\n');
}

// For BCD conversion
const uint32_t DIVIDER[] = {1, 10, 100, 1000, 10000, 100000, 1000000,
                            10000000, 100000000, 1000000000};

// Print decimal value (BCD conversion by substraction method)
void OLED_printD(uint32_t value) {
  uint8_t digits   = 10;                          // print 10 digits
  uint8_t leadflag = 0;                           // flag for leading spaces
  while(digits--) {                               // for all digits
    uint8_t digitval = 0;                         // start with digit value 0
    uint32_t divider = DIVIDER[digits];           // read current divider
    while(value >= divider) {                     // if current divider fits into the value
      leadflag = 1;                               // end of leading spaces
      digitval++;                                 // increase digit value
      value -= divider;                           // decrease value by divider
    }
    if(!digits)  leadflag++;                      // least digit has to be printed
    if(leadflag) OLED_write(digitval + '0');      // print the digit
  }
}

// Convert byte nibble into hex character and print it
void OLED_printN(uint8_t nibble) {
  OLED_write((nibble <= 9) ? ('0' + nibble) : ('A' - 10 + nibble));
}

// Convert byte into hex characters and print it
void OLED_printB(uint8_t value) {
  OLED_printN(value >> 4);
  OLED_printN(value & 0x0f);
}

// Convert word into hex characters and print it
void OLED_printW(uint16_t value) {
  OLED_printB(value >> 8);
  OLED_printB(value);
}

// Convert long into hex characters and print it
void OLED_printL(uint32_t value) {
  OLED_printW(value >> 16);
  OLED_printW(value);
}
