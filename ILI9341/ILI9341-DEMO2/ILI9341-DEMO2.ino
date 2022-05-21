// ILI9341 SPI Graphics TEST
#include <SPI.h>
#include "image2.h"
#include "font8x16.h"

#define TFT_CLK 13
#define TFT_MISO 12
#define TFT_MOSI 11
#define TFT_DC 8
#define TFT_RST 9
#define TFT_CS 10
// BackLightはHIGH固定にするのでソフトがらは制御しない
// #define TFT_BL 7

uint8_t SPIBuf[320*2] ;   // SPI転送用バッファ
uint8_t fontColorF[2] ;   // TEXT FOR GROND COLOR
uint8_t fontColorB[2] ;   // TEXT BACK GROND COLOR

uint16_t tftWidth ;
uint16_t tftHeight ;

#define HBYTE(u) ((u >> 8) & 0xFF)
#define LBYTE(u) (u & 0xFF)

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a ; a = b; b = t; }
#endif

// ----- ILI9341 COMMNDS -----
#define ILI9341_SOFTRESET 0x01              // SOFTWARE RESET
#define ILI9341_SLEEP_OUT 0x11              // SLEEP OUT
#define ILI9341_DISPLAY_OFF 0x28            // DISPLAY OFF
#define ILI9341_DISPLAY_ON 0x29             // DISPLAY ON
#define ILI9341_COLUMN_ADDRESS_SET 0x2a     // COL SET
#define ILI9341_PAGE_ADDRESS_SET 0x2b       // ROW SET
#define ILI9341_MEMORY_WRITE 0x2c           // MEMORY WRITE
#define ILI9341_MEMORY_ACCESS_CONTROL 0x36  // 表示向き

#define ILI9341_NORON 0x13    // Normal Display Mode ON
#define ILI9341_PTLON 0x12    // Partial Mode ON
#define ILI9341_INVOFF 0x20   // Display Inversion OFF
#define ILI9341_INVON 0x21    // Display Inversion ON

#define ILI9341_COLMOD 0x3a   // 表示モード
#define ILI9341_COLOR16 0x55  // 1Dot/16Bitモード
#define ILI9341_COLOR18 0x66  // 1Dot/18Bitモード

#define ILI9341_WIDTH 320
#define ILI9341_HEIGHT 240

// ▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼
// TFTにコマンドを送信
void tftSendCommand(uint8_t command) {
  digitalWrite(TFT_CS, LOW); // TFTセレクト
  digitalWrite(TFT_DC, LOW); // Command mode
  SPI.transfer(command);
  digitalWrite(TFT_CS, HIGH);  // TFT解放
}

// TFTにコマンド+1バイトデータを送信
void tftSendCommand1(uint8_t command, uint8_t data1) {
  digitalWrite(TFT_CS, LOW); // TFTセレクト
  digitalWrite(TFT_DC, LOW); // Command mode
  SPI.transfer(command);
  digitalWrite(TFT_DC, HIGH); // data mode
  SPI.transfer(data1);
  digitalWrite(TFT_CS, HIGH);  // TFT解放
}

// ▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼
void lcd_init() {
  // --- HARDWARE Reset
  digitalWrite(TFT_RST, LOW);
  delay(20);
  digitalWrite(TFT_RST, HIGH);
  delay(20);    // このdelayは必要

  // --- SOFTWARE Reset
  tftSendCommand(ILI9341_SOFTRESET) ;   // SOFTWARE RESET
  delay(150);                           // このdelayはなくても大丈夫かも

  // ----- ILI9431動作モード設定 -----
  tftSendCommand(ILI9341_NORON) ;  //  Normal Display Mode ON
  tftSendCommand(ILI9341_INVOFF) ; //  Display Inversion OFF
  screenDir(1) ;

  // ピクセルフォーマットを1Dot/16bitに設定
  tftSendCommand1(ILI9341_COLMOD,ILI9341_COLOR16);

  // PowerOFFにならないモード
  tftSendCommand(ILI9341_SLEEP_OUT);
  delay(60);  // このdelayは必要

  // 画面表示ON
  tftSendCommand(ILI9341_DISPLAY_ON);
}

// △▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽
void screenDir(int dir) {
  //  MY - 0x80 // Row Address Order
  //  MX - 0x40 // Column Address Order
  //  MV - 0x20 // Row / Column Exchange
  //  ML - 0x10 // Vertical Refresh Order
  //  MH - 0x04 // Horizontal Refresh ORDER
  //  MRGB - 0x08 // RBG - BGR
  uint8_t PR = 0 ;

  tftWidth = ILI9341_WIDTH ;
  tftHeight=ILI9341_HEIGHT ;
  switch(dir) {
    case 1:
      tftWidth = ILI9341_HEIGHT ;
      tftHeight= ILI9341_WIDTH ;
      PR = 0x48 ;   // MX | MRGB // DIR - 1 : 縦
      break ;
    case 3:
      tftWidth = ILI9341_HEIGHT ;
      tftHeight= ILI9341_WIDTH ;
      PR = 0x8C;    // MY | MH | MRGB // DIR - 3 : 縦
      break ;
    case 2:
      tftWidth = ILI9341_WIDTH ;
      tftHeight= ILI9341_HEIGHT ;
      PR = 0xE8 ;   // MY | MX | MV | MRGB //　dir-2 : 横
      break ;
    case 4:
      tftWidth = ILI9341_WIDTH ;
      tftHeight= ILI9341_HEIGHT ;
      PR = 0x28 ;   // MV | MRGB //　dir-4 : 横
      break ;
  }
  tftSendCommand1(ILI9341_MEMORY_ACCESS_CONTROL,PR) ;
}

// △▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽
void dispStr(char str[] , uint8_t x , uint8_t y) {
  for (int i=0;str[i] != 0 ; i++) {
      dispFont(str[i],x,y) ;
      x += 8 ;
      if (x+8>=tftWidth) {
        x = 0 ;
        y += 16 ;
        if (y + 16 >= tftHeight) {
          y = 0 ; 
        }
      }
  }
}

// △▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽
void dispFont(uint8_t code , uint8_t x, uint8_t y) {
  int pos = (code - 0x20) * 16 ;
  memset(SPIBuf,0,8*16*2) ;
  for (int i=0;i<16;i++) {
    uint8_t data = pgm_read_byte(ssd1306xled_font8x16 + pos + i);
    uint8_t bitMast = 0x01 ; 
    for (int j=0;j<8;j++) {
      int fpos = 0 ;
      if (i<8) {
        fpos = (j*8+i)*2 ;
      } else {
        fpos = ((j+8)*8+(i-8))*2 ;
      }
      if ((data & bitMast) == 0) {
        SPIBuf[fpos] = fontColorB[0] ;
        SPIBuf[fpos+1] = fontColorB[1] ;
      } else {
        SPIBuf[fpos] = fontColorF[0] ;
        SPIBuf[fpos+1] = fontColorF[1] ;
      }
      bitMast <<= 1 ;
    }
  }

  uint8_t endX = x + 7 ;
  uint8_t endY = y + 15;

  digitalWrite(TFT_CS, LOW);  //通信開始

  digitalWrite(TFT_DC, LOW); // Command mode
  SPI.transfer(ILI9341_COLUMN_ADDRESS_SET);
  digitalWrite(TFT_DC, HIGH); // data mode
  SPI.transfer16(x);
  SPI.transfer16(endX);

  digitalWrite(TFT_DC, LOW); // Command mode
  SPI.transfer(ILI9341_PAGE_ADDRESS_SET);
  digitalWrite(TFT_DC, HIGH); // data mode
  SPI.transfer16(y);
  SPI.transfer16(endY);

  digitalWrite(TFT_DC, LOW); // Command mode
  SPI.transfer(ILI9341_MEMORY_WRITE);
  digitalWrite(TFT_DC, HIGH); // data mode
  SPI.transfer(SPIBuf, 8*16*2);
  digitalWrite(TFT_DC, LOW); // Command mode
  digitalWrite(TFT_CS, HIGH); //通信終了
}

// △▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽
// BITマップを表示する::基本fillと同じ作り
void dispBitMap(uint16_t startX , uint16_t startY , uint16_t width, uint16_t hight, uint8_t bitMap[]) {
  uint16_t endX = startX + width - 1;
  uint16_t endY = startY + hight - 1;

  digitalWrite(TFT_CS, LOW);  //通信開始

  digitalWrite(TFT_DC, LOW); // Command mode
  SPI.transfer(ILI9341_COLUMN_ADDRESS_SET);
  digitalWrite(TFT_DC, HIGH); // data mode
  SPI.transfer16(startX);
  SPI.transfer16(endX);

  digitalWrite(TFT_DC, LOW); // Command mode
  SPI.transfer(ILI9341_PAGE_ADDRESS_SET);
  digitalWrite(TFT_DC, HIGH); // data mode
  SPI.transfer16(startY);
  SPI.transfer16(endY);

  int pos = 0 ;
  digitalWrite(TFT_DC, LOW); // Command mode
  SPI.transfer(ILI9341_MEMORY_WRITE);
  digitalWrite(TFT_DC, HIGH); // data mode
  for (int row = 0 ; row < hight ; row ++) {
    // 表示データ転送
    // Arduinoだとメモリが足りなくて１画面分を一気に転送する事ができない
    // なので、1Lineずつ送信している
    for (int col = 0 ; col < width * 2 ; col ++ ) {
      SPIBuf[col] = pgm_read_byte(bitMap + pos);
      pos ++ ;
    }
    SPI.transfer(SPIBuf, width * 2);
  }
  digitalWrite(TFT_DC, LOW); // Command mode
  digitalWrite(TFT_CS, HIGH); //通信終了
}

// △▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽
void fillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t hight, uint16_t color) {
  long num = (long)hight * (long)width ;
  uint16_t endX = x + width - 1;
  uint16_t endY = y + hight - 1;

  digitalWrite(TFT_CS, LOW);  //通信開始

  digitalWrite(TFT_DC, LOW); // Command mode
  SPI.transfer(ILI9341_COLUMN_ADDRESS_SET);
  digitalWrite(TFT_DC, HIGH); // data mode
  SPI.transfer16(x);
  SPI.transfer16(endX);

  digitalWrite(TFT_DC, LOW); // Command mode
  SPI.transfer(ILI9341_PAGE_ADDRESS_SET);
  digitalWrite(TFT_DC, HIGH); // data mode
  SPI.transfer16(y);
  SPI.transfer16(endY);

  digitalWrite(TFT_DC, LOW); // Command mode
  SPI.transfer(ILI9341_MEMORY_WRITE);
  digitalWrite(TFT_DC, HIGH); // data mode
  for (long i = 0 ; i < num ; i++) {
    SPI.transfer16(color);
  }

  digitalWrite(TFT_DC, LOW); // Command mode
  digitalWrite(TFT_CS, HIGH); //通信終了
}

void fill(uint16_t xs,uint16_t ys,uint16_t xe,uint16_t ye,uint16_t color) {
  uint16_t hight = ye - ys + 1 ;
  uint16_t width = xe - xs + 1 ;
  fillRect(xs,ys,width,hight,color) ;
}

void fillScreen(uint16_t color) {
  fill(0,0,tftWidth-1,tftHeight-1,color) ;
}

// △▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽
void scroll(uint16_t startAddr) {
  digitalWrite(TFT_CS, LOW);  //通信開始
  digitalWrite(TFT_DC, LOW); // Command mode
  SPI.transfer(0x37);
  digitalWrite(TFT_DC, HIGH); // data mode
  SPI.transfer16(startAddr);
  digitalWrite(TFT_DC, LOW); // Command mode
  digitalWrite(TFT_CS, HIGH); //通信終了  
}

// ▼▼▼▼▼▼ Adafruit_GFX_Libraryからパクった描画関数
// ローテーションなし、Windowクリップなし、描画フレームへなしにして不要部分を削ってます

// 1Dot描画
void drawPixel(uint16_t x, uint16_t y, uint16_t color) {
  if (x < 0 || x >= tftWidth || y < 0 || y >= tftHeight) return ;

  uint16_t xe = x+1 ;
  uint16_t ye = y+1 ;

  digitalWrite(TFT_CS, LOW); // TFTセレクト
  digitalWrite(TFT_DC, LOW); // Command mode
  SPI.transfer(ILI9341_COLUMN_ADDRESS_SET);
  digitalWrite(TFT_DC, HIGH); // data mode
  SPI.transfer16(x);
  SPI.transfer16(xe);

  digitalWrite(TFT_DC, LOW); // Command mode
  SPI.transfer(ILI9341_PAGE_ADDRESS_SET);
  digitalWrite(TFT_DC, HIGH); // datamand mode
  SPI.transfer16(y);
  SPI.transfer16(ye);

  digitalWrite(TFT_DC, LOW); // Command mode
  SPI.transfer(ILI9341_MEMORY_WRITE);
  digitalWrite(TFT_DC, HIGH); // data mode
  SPI.transfer16(color);

  digitalWrite(TFT_DC, LOW); // Command mode
  digitalWrite(TFT_CS, HIGH); // TFT解放
}

// Line描画
void drawLine(int x0, int y0, int x1, int y1, uint16_t color) {
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }
  if (x0 > x1) {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

//  if (x0 == x1 || y0 == y1) {
//    return fill(x0,x1,y0,y1,color) ;
//  }
//
  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0 <= x1; x0++) {
    if (steep) {
      drawPixel(y0, x0, color);
    } else {
      drawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

// 矩形
void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  int xe = x + w - 1 ;
  int ye = y + h - 1 ;
  drawLine(x, y, xe,y ,color);
  drawLine(x, y, x ,ye,color);
  drawLine(xe,y, xe,ye,color);
  drawLine(x ,ye,xe,ye,color);
}

// 円
void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  drawPixel(x0, y0 + r, color);
  drawPixel(x0, y0 - r, color);
  drawPixel(x0 + r, y0, color);
  drawPixel(x0 - r, y0, color);

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    drawPixel(x0 + x, y0 + y, color);
    drawPixel(x0 - x, y0 + y, color);
    drawPixel(x0 + x, y0 - y, color);
    drawPixel(x0 - x, y0 - y, color);
    drawPixel(x0 + y, y0 + x, color);
    drawPixel(x0 - y, y0 + x, color);
    drawPixel(x0 + y, y0 - x, color);
    drawPixel(x0 - y, y0 - x, color);
  }
}

// 塗りつぶ円
void fillCircle(int16_t x0, int16_t y0, int16_t r,uint16_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;
  int16_t px = x;
  int16_t py = y;
  int16_t delta = 1 ;
  
  fillRect(x0, y0 - r , 1 , r * 2 , color);

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
    // These checks avoid double-drawing certain lines, important
    // for the SSD1306 library which has an INVERT drawing mode.
    if (x < (y + 1)) {
        fillRect(x0 + x , y0 - y , 1 , y * 2 + delta -1 , color);
        fillRect(x0 - x , y0 - y , 1 , y * 2 + delta -1 , color);
    }
    if (y != py) {
        fillRect(x0 + py , y0 - px , 1 , px * 2 + delta -1 , color);
        fillRect(x0 - py , y0 - px , 1 , px * 2 + delta -1 , color);
      py = y;
    }
    px = x;
  }
}

// ▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼
// SPISettings settings = SPISettings(8000000, MSBFIRST, SPI_MODE0);

void setup() {
  Serial.begin(9600);
  Serial.println(F("ILI9341 DEMO"));

  SPI.begin();  //SPIを初期化、SCK、MOSI、SSの各ピンの動作は出力、SCK、MOSIはLOW、SSはHIGH
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);

  // -----  ST3375 INITIAL -----
  pinMode(TFT_CS, OUTPUT);
  pinMode(TFT_CS, OUTPUT);
  pinMode(TFT_DC, OUTPUT);
  pinMode(TFT_RST, OUTPUT);

  lcd_init( ) ;
}

// ▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼
void loop() {
  tftSendCommand(ILI9341_NORON) ; //  Normal Display Mode ON
  screenDir(2) ;

//  fillScreen(0xF800) ;  // B11111 000 000 00000 : RED
//  delay(500) ;
//  fillScreen(0x07E0) ;  // B00000 111 111 00000 : GREEN
//  delay(500) ;
//  fillScreen(0x001F) ;  // B00000 000 000 11111 : BLUE
//  delay(500) ;

  // ----- TEXT -----
  fillScreen(0xFFFF) ;  // B11111 111 111 11111 :  WHITE
  fontColorB[0] = 0x00 ; fontColorB[1] = 0x00 ; // TEXT BACK GROND COLOR
  fontColorF[0] = 0xFF ; fontColorF[1] = 0xCF ; // TEXT FOR GROND COLOR
  dispStr("ABCDEFGHIJ",0,0) ;
  fontColorF[0] = 0x07 ; fontColorF[1] = 0xE0 ; // TEXT FOR GROND COLOR
  dispStr("0123456789",0,16) ;
  fontColorF[0] = 0x00 ; fontColorF[1] = 0x1F ; // TEXT FOR GROND COLOR
  dispStr("ABCDEFGHIJ",0,32) ;
  fontColorF[0] = 0xF8 ; fontColorF[1] = 0x00 ; // TEXT FOR GROND COLOR
  dispStr("0123456789",0,48) ;
  fontColorF[0] = 0xFE ; fontColorF[1] = 0xE0 ; // TEXT FOR GROND COLOR
  dispStr("ABCDEFGHIJ",0,64) ;
  fontColorF[0] = 0x07 ; fontColorF[1] = 0xFF ; // TEXT FOR GROND COLOR
  dispStr("0123456789",0,80) ;
  delay(1000) ;

  // --- SCROLL
  for (int i=0, sc=16;i<10;i++,sc+=16) {
    scroll(sc) ;
    delay(1000) ;
  }
  delay(2000) ;
  tftSendCommand(ILI9341_NORON) ; //  Normal Display Mode ON

  // --- BITMAP ---
  fontColorF[0] = 0x00 ; fontColorF[1] = 0x00 ; // TEXT FOR GROND COLOR
  fillScreen(0x07E0) ; 
  screenDir(1) ;
  fontColorB[0] = 0x07 ; fontColorB[1] = 0xE0 ; // TEXT BACK GROND COLOR
  dispStr("DIR-1",0,0) ;
  dispBitMap(0,   20, 96, 64, bitmapData2) ; delay(2000) ;

  fillScreen(0xFFFF) ; 
  screenDir(2) ;
  fontColorB[0] = 0xFF ; fontColorB[1] = 0xFF ; // TEXT BACK GROND COLOR
  dispStr("DIR-2",0,0) ;
  dispBitMap(0,   20, 96, 64, bitmapData2) ; delay(2000) ;

  fillScreen(0x07E0) ; 
  screenDir(3) ;
  fontColorB[0] = 0x07 ; fontColorB[1] = 0xE0 ; // TEXT BACK GROND COLOR
  dispStr("DIR-3",0,0) ;
  dispBitMap(0,   20, 96, 64, bitmapData2) ; delay(2000) ;

  fillScreen(0xFFFF) ; 
  screenDir(4) ;
  fontColorB[0] = 0xFF ; fontColorB[1] = 0xFF ; // TEXT BACK GROND COLOR
  dispStr("DIR-4",0,0) ;
  dispBitMap(0,   20, 96, 64, bitmapData2) ; delay(2000) ;

  screenDir(2) ;
  tftSendCommand1(ILI9341_MEMORY_ACCESS_CONTROL,0xe8) ;
  fillScreen(0x0000) ;  // B00000 000 000 00000 :  BLACK
  for (int y=0;y<ILI9341_HEIGHT;y+=18) {
    fill(0,y,ILI9341_WIDTH-1,y+8,0x07E0) ; // B00000 111 111 00000 : GREEN
  }
  for (int x=0;x<ILI9341_WIDTH;x+=16) {
    fill(x,0,x+8,ILI9341_HEIGHT-1,0xFFE0) ; // B00000 111 111 00000 : GREEN
  }
  delay(2000) ;

  int xs , ys , xe , ye , w , h ;
  int c ;

  // -- LINE
  fillScreen(0x0000) ; 
  xs = 0 ;  ys = 0 ;  xe = 0 ;  ye = 239 ;
  for (int i=0;i<10;i++) {
    Serial.print("drawLine(");
    Serial.print(xs); Serial.print(",");
    Serial.print(ys); Serial.print(",");
    Serial.print(xe); Serial.print(",");
    Serial.print(ye); Serial.println(")");
    drawLine(xs,ys,xe,ye,0xF800) ;
    ys += 24 ;
    xe += 32 ;
  }

  xs = 0 ;  ys = 239 ;  xe = 329 ;  ye = 239 ;
  for (int i=0;i<10;i++) {
    drawLine(xs,ys,xe,ye,0x07E0) ;
    xs += 32 ;
    ye -= 24 ;
  }
  
  xs = 0 ;  ys = 0 ;  xe = 329 ;  ye = 0 ;
  for (int i=0;i<10;i++) {
    drawLine(xs,ys,xe,ye,0x001F) ;
    xs += 32 ;
    ye += 24 ;
  }
  
  xs = 0 ;  ys = 239 ;  xe = 0 ;  ye = 0 ;
  for (int i=0;i<10;i++) {
    drawLine(xs,ys,xe,ye,0xFFFF) ;
    ys -= 24 ;
    xe += 32 ;
  }
  delay(2000);

  // -- rect
  fillScreen(0x0000) ; 
  xs = 144 ;  ys = 108 ;  w = 32 ;  h = 24 ;
  for (int i=0;i<10;i++) {
    switch (i % 3) {
      case 0: drawRect(xs,ys,w,h,0xF800) ; break ;
      case 1: drawRect(xs,ys,w,h,0x07E0) ; break ;
      case 2: drawRect(xs,ys,w,h,0x001F) ; break ;
    }
    xs -= 16 ;    ys -= 12 ;
    w += 32 ;    h += 24 ;
  }
  delay(2000);

  // -- fillRect
  xs = 0 ;  ys = 0 ;  w = 320 ;  h = 240 ;
  for (int i=0;i<10;i++) {
    switch (i % 3) {
      case 0: fillRect(xs,ys,w,h,0xF800) ; break ;
      case 1: fillRect(xs,ys,w,h,0x07E0) ; break ;
      case 2: fillRect(xs,ys,w,h,0x001F) ; break ;
    }
    xs += 16 ;    ys += 12 ;
    w -= 32 ;    h -= 24 ;
  }
  delay(2000);

  // -- Circle
  fillScreen(0x0000) ; 
  xs = 159 ;  ys = 119 ;  w = 10 ;
  for (int i=0;i<10;i++) {
    switch (i % 3) {
      case 0: drawCircle(xs,ys,w,0xF800) ; break ;
      case 1: drawCircle(xs,ys,w,0x07E0) ; break ;
      case 2: drawCircle(xs,ys,w,0x001F) ; break ;
    }
    w += 24 ;
  }
  delay(2000);

  // -- FillCircle
  fillScreen(0x0000) ; 
  xs = 159 ;  ys = 119 ;  w = 119 ;
  for (int i=0;i<10;i++) {
    switch (i % 3) {
      case 0: fillCircle(xs,ys,w,0xF800) ; break ;
      case 1: fillCircle(xs,ys,w,0x07E0) ; break ;
      case 2: fillCircle(xs,ys,w,0x001F) ; break ;
    }
    w -= 24 ;
  }
  delay(3000);

}
