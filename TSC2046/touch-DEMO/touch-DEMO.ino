// TSC2046 Touch Screen & ILI9341 SPI Graphics TEST
#include <SPI.h>

#define TFT_CLK 13
#define TFT_MISO 12
#define TFT_MOSI 11
#define TFT_DC 8
#define TFT_RST 9
#define TFT_CS 10
#define TS_CS 7

#define CMD_RDX 0XD0
#define CMD_RDY 0X90
#define CMD_RDZ1 0XB0
#define CMD_RDZ2 0XC0

int cxs,cxe,cys,cye,ox,oy ;
int pbx,pby ;
uint16_t tftWidth ;
uint16_t tftHeight ;
uint16_t tftDir ;

#define ILI9341_WIDTH 320
#define ILI9341_HEIGHT 240

#define HBYTE(u) ((u >> 8) & 0xFF)
#define LBYTE(u) (u & 0xFF)

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a ; a = b; b = t; }
#endif

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
  tftDir = dir ;
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

  digitalWrite(TFT_CS, LOW);  //通信開始

  digitalWrite(TFT_DC, LOW); // Command mode
  SPI.transfer(0x36);
  digitalWrite(TFT_DC, HIGH); // data mode
  SPI.transfer(PR);

  digitalWrite(TFT_CS, HIGH);  //通信開始
}

// △▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽△▽
void fillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t hight, uint16_t color) {
  long num = (long)hight * (long)width ;
  uint16_t endX = x + width - 1;
  uint16_t endY = y + hight - 1;
  digitalWrite(TFT_CS, LOW);  //通信開始

  digitalWrite(TFT_DC, LOW); // Command mode
  SPI.transfer(0x2a);
  digitalWrite(TFT_DC, HIGH); // data mode
  SPI.transfer16(x);
  SPI.transfer16(endX);

  digitalWrite(TFT_DC, LOW); // Command mode
  SPI.transfer(0x2b);
  digitalWrite(TFT_DC, HIGH); // data mode
  SPI.transfer16(y);
  SPI.transfer16(endY);

  digitalWrite(TFT_DC, LOW); // Command mode
  SPI.transfer(0x2c);
  digitalWrite(TFT_DC, HIGH); // data mode
  for (long i = 0 ; i < num ; i++) {
    SPI.transfer16(color);
  }

  digitalWrite(TFT_DC, LOW); // Command mode
  digitalWrite(TFT_CS, HIGH); //通信終了
}

// ▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼
boolean readPos(int &x,int &y) {
  boolean touch = false;
  uint8_t z1,z2 ;

  digitalWrite(TFT_CS, LOW);

  SPI.transfer(CMD_RDX);
  x = SPI.transfer16(0);
  x >>= 3 ;
  SPI.transfer(CMD_RDY);
  y = SPI.transfer16(0);
  y >>= 3 ;
  SPI.transfer(CMD_RDZ1);
  z1 = SPI.transfer(0);
  SPI.transfer(CMD_RDZ2);
  z2 = SPI.transfer(0);
  digitalWrite(TFT_CS, HIGH);

  if (z1 > 0 && z2 > 0) {
    touch = true ;
  }
  
  return touch ;
}
// ▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼
//calibration
void calibration( ) {
  screenDir(2) ;
  fillRect(0,0,tftWidth-1,tftHeight-1,0x0000) ;
  fillRect(0,9,20,1,0xFFFF) ; fillRect(9,0,1,20,0xFFFF) ;
  int stat = 0 ;
  int x,y ;
  char str[24] ;
  while (stat != 2) {
    if ( readPos(x,y) ) {
      switch(stat) {
        case 0:
          cxs = x ; cys = y ; stat = 1 ;
          fillRect(0,0,tftWidth-1,tftHeight-1,0x0000) ;
          fillRect(tftWidth-20,tftHeight-10,20,1,0xFFFF) ;
          fillRect(tftWidth-10,tftHeight-20,1,20,0xFFFF) ;
          sprintf(str,"CXS=%4d  CYS=%4d",cxs,cys) ;
          Serial.println(str) ;
          break ;
        case 1:
          cxe = x ; cye = y ; stat = 2 ;
          sprintf(str,"CXE=%4d  CYE=%4d",cxe,cye) ;
          Serial.println(str) ;
          break ;
      }
    }
    delay(100) ;
  }
  int px,py ;
  px = abs(cxs-cxe)/ (tftWidth - 20) ;
  py = abs(cys-cye) / (tftHeight - 20) ;
  ox = cxs - px * 10 ;
  oy = cys - py * 10 ;
  sprintf(str,"PX=%d  PY=%d",px,py) ;
  Serial.println(str) ;
  sprintf(str,"OX=%d OY=%d",ox,oy) ;
  Serial.println(str) ;
}

void ConvPos(int &x,int &y) {
  int wx ,wy ;
  wx = (220. * (x - ox) ) / abs(cxs-cxe) ;
  wy = (300. * (y - oy) ) / abs(cys-cye) ;

  char str[24] ;
  sprintf(str,"TSC X=%4d Y=%4d",x,y) ;
  Serial.println(str) ;

  switch(tftDir) { // Dir = 2 以外は動作確認してない
    case 1: x = wx ; y = tftHeight - wy ; break ;
    case 2: x = wy ; y = wx ; break ;
    case 3: x = tftWidth - wx ; y = wy ; break ;
    case 4: x = tftHeight - wy ; y = tftWidth - wx ; break ;
  }

  sprintf(str,"TFT X=%4d Y=%4d",x,y) ;
  Serial.println(str) ;
}

void DrowButton( ) {
  fillRect(0,0,tftWidth-1,tftHeight-1,0x0000) ;
  int yn = (tftHeight - 40) / 50 ; 
  int xn = (tftWidth - 20) / 50 ; 
  for (int y=0;y<yn;y++) {
    for (int x=0;x<xn;x++) {
      fillRect(20+x*50,20+y*50,30,30,0xFFFF) ;
    }
  }
}

void PushButton(int x,int y) {
  int xs,ys,bx,by ;

  xs = (x - 20) / 50 ;
  ys = (y - 20) / 50 ;
  if ((x >= 20 + 50 * xs) && (x <= 20 + 50 * xs +30) && 
      (y >= 20 + 50 * ys) && (y <= 20 + 50 * ys +30)) {
    bx = xs ;
    by = ys ; 
  } else {
    bx = -1 ;
    by = -1 ;
  }
  if ((pbx >= 0 || pby >= 0) && (pbx != bx || pby != by)) {
    xs = 20 + 50 * pbx ;
    ys = 20 + 50 * pby ;
    fillRect(xs,ys,30,30,0xFFFF) ;
  }

  if (bx >= 0 && by >= 0){
    xs = 20 + 50 * bx ;
    ys = 20 + 50 * by ;
    fillRect(xs,ys,30,30,0xF800) ;
    pbx = bx ; pby = by ;
  }
}

// ▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼
// SPISettings settings = SPISettings(4000000, MSBFIRST, SPI_MODE0);  // 4MHz
void setup() {
  Serial.begin(9600);
  Serial.println(F("TOUCH PANEL(TSC2046) TEST"));

  SPI.begin();  //SPIを初期化、SCK、MOSI、SSの各ピンの動作は出力、SCK、MOSIはLOW、SSはHIGH
  SPI.setClockDivider(SPI_CLOCK_DIV4);
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
//  SPI.beginTransaction(settings); // M5StampやRaspberryPi PICOで使う時はコッチの設定でないとコンパイルが通らない

  // --- ST3375 INITIAL
  pinMode(TFT_CS, OUTPUT);
  pinMode(TFT_DC, OUTPUT);
  pinMode(TFT_RST, OUTPUT);
  pinMode(TS_CS, OUTPUT);

  // --- HARDWARE Reset
  digitalWrite(TFT_RST, LOW);
  delay(20);
  digitalWrite(TFT_RST, HIGH);
  delay(20);    // このdelayは必要

  // --- ILI9341 動作設定
  digitalWrite(TFT_CS, LOW);  // TFTセレクト

  digitalWrite(TFT_DC, LOW);  // Command mode
  SPI.transfer(0x01);         //<< SOFTWARE RESET
  delay(150);                 // ※このdelayはなくても大丈夫かも

  SPI.transfer(0x13) ;        //<< Normal Display Mode ON

  SPI.transfer(0x36) ;        //<< Memory Access Control
  digitalWrite(TFT_DC, HIGH); // Data mode
  SPI.transfer(0x48) ;        //   MX = ON , GBR = ON (縦画面,左から右に表示)

  digitalWrite(TFT_DC, LOW);  // Command mode
  SPI.transfer(0x3a) ;        //<< COLMOD: Pixel Format Set
  digitalWrite(TFT_DC, HIGH); // Data mode
  SPI.transfer(0x55) ;        //<< 16Bits / 1Pixcel

  digitalWrite(TFT_DC, LOW);  // Command mode
  SPI.transfer(0x11) ;        //<< Sleep Out

  delay(60);                  // 【重要】このdelayは絶対に必要：電源が不安定な場合は長めに(120ms位に)する

  SPI.transfer(0x29) ;        //<< Display ON

  digitalWrite(TFT_CS, HIGH); // TFT解放

  // ----- APRI INIT -----
  tftDir = 1 ;
  tftWidth = ILI9341_WIDTH ;
  tftHeight = ILI9341_HEIGHT ;
  cxs = 0 ; cxe = tftWidth -1 ; 
  cys = 0 ; cye = tftHeight -1 ;
  pbx = -1  ; pby = -1 ;
  calibration( ) ;

  DrowButton( ) ;
}

// ▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼
void loop() {
  int x,y ;
  if ( readPos(x,y) ) {
    ConvPos(x,y) ;
    PushButton(x,y) ;
  }
  delay(100) ;
}
