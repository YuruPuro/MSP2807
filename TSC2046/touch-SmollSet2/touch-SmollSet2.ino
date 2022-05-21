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
#define TS_CS 7

#define CMD_RDX 0XD0
#define CMD_RDY 0X90
#define CMD_RDZ1 0XB4
#define CMD_RDZ2 0XC4

// ▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼
boolean readPos(int &x,int &y,int &z) {
  boolean touch = false;
  uint16_t rx,ry;
  uint8_t z1,z2,rz ;

  digitalWrite(TS_CS, LOW);

  SPI.transfer(CMD_RDX);
  rx = SPI.transfer16(0);
  rx >>= 4 ;

  SPI.transfer(CMD_RDY);
  ry = SPI.transfer16(0);
  ry >>= 4 ;

  SPI.transfer(CMD_RDZ1);
  z1 = SPI.transfer(0);

  SPI.transfer(CMD_RDZ2);
  z2 = SPI.transfer(0);

  digitalWrite(TS_CS, HIGH);

  if (z1 > 0 && z2 > 0) {
    rz = 4096 - (int)((double)(z2 / z1 * rx / 4.0 ));
    x = rx ;
    y = ry ;
    z = rz ;
    touch = true ;
  }
  
  return touch ;
}

// ▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼
// SPISettings settings = SPISettings(2000000, MSBFIRST, SPI_MODE0);  // 2MHzで合ってるよね？変だと思ったら修正してください。
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

  delay(2000) ;
  Serial.println("@0000\r\n0000");
}

// ▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼▲▼
void loop() {
  for (int i=0;i<5;i++) {
    uint16_t color = 0x0000 ;
    switch(i) {
      case 0: color = 0x0000 ; break ;
      case 1: color = 0xF800 ; break ;
      case 2: color = 0x07E0 ; break ;
      case 3: color = 0x001F ; break ;
      case 4: color = 0xFFFF ; break ;
    }
    // --- 画面塗りつぶし
    digitalWrite(TFT_CS, LOW);  //TFTセレクト

    digitalWrite(TFT_DC, LOW);  // Command mode
    SPI.transfer(0x2a);         //<< Column Address Set
    digitalWrite(TFT_DC, HIGH); // Data mode
    SPI.transfer16(0);          // 開始カラム 0
    SPI.transfer16(239);        // 終了カラム 239

    digitalWrite(TFT_DC, LOW);  // Command mode
    SPI.transfer(0x2b);         //<< Page Address Set
    digitalWrite(TFT_DC, HIGH); // Data mode
    SPI.transfer16(0);          // 開始ページ 0
    SPI.transfer16(319);        // 終了ページ 319

    digitalWrite(TFT_DC, LOW);  // Command mode
    SPI.transfer(0x2c);         //<< Memory Write
    digitalWrite(TFT_DC, HIGH); // Data mode
    for (int y = 0 ; y < 320 ; y++) {
      for (int x = 0 ; x < 240 ; x++) {
        uint16_t data = color ;
        SPI.transfer16(color);
      }
    }
    digitalWrite(TFT_CS, HIGH); //TFT解放

    char str[16] ;
    int x,y,z ;
    for (int i=0;i<5;i++) {
      if (readPos(x,y,z)) {
        sprintf(str,"H%04d\r\n%04d",x,y) ;
        Serial.println(str);
//        Serial.print(x);
//        Serial.print(" , ");
//        Serial.print(y);
//        Serial.print(" ) Press( ");
//        Serial.print(z);
//        Serial.println(" )");
      }
      delay(100) ;
    }
  }
}
