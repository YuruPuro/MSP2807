// TOUCH PANEL TEST
#include <SPI.h>

#define TFT_CLK 13
#define TFT_MOSI 12
#define TFT_MISO 11
#define TFT_CS 7

#define CMD_RDX 0XD0
#define CMD_RDY 0X90
#define CMD_RDZ1 0XB8
#define CMD_RDZ2 0XC8

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

// SPISettings settings = SPISettings(4000000, MSBFIRST, SPI_MODE0);  // 4MHz
void setup() {
  Serial.begin(9600);
  Serial.println(F("TOUCH PANEL(TSC2046) TEST"));

  pinMode(TFT_CS,OUTPUT) ;

  SPI.begin();  //SPIを初期化、SCK、MOSI、SSの各ピンの動作は出力、SCK、MOSIはLOW、SSはHIGH
  SPI.setClockDivider(SPI_CLOCK_DIV4);  // 4MHz位にしておかないとTouch Panelの処理が追い付かないっぽい
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
//  SPI.beginTransaction(settings); // M5StampやRaspberryPi PICOで使う時はコッチの設定でないとコンパイルが通らない
}

void loop() {
  int x,y ;
  if (readPos(x,y)) {
    Serial.print("( ");
    Serial.print(x);
    Serial.print(" , ");
    Serial.print(y);
    Serial.println(" )");
  }
  delay(100) ;
}
