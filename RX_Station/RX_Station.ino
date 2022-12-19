#include <RS-FEC.h>
#include <Wire.h>
#include <SPI.h>
#include <RH_RF95.h>
#include <U8g2lib.h>

#include "images.h"

// Display Settings (I2C)
const int DISPLAY_ADDRESS = 0x3c;
const int DISPLAY_SDA = 4;
const int DISPLAY_SCL = 15;
const int DISPLAY_RST = 16;
const int FONT_HEIGHT = 10;

// LoRa Settings (SPI)
const int LORA_SCK = 5;
const int LORA_MISO = 19;
const int LORA_MOSI = 27;
const int LORA_SS = 18;
const int LORA_RST = 15;
const int LORA_DI00 = 26;
const int LORA_FREQ = 915.0;
const int LORA_SF = 7;
const int LORA_CODING_RATE = 5;
const int LORA_BANDWIDTH = 125E3;

const uint8_t ECC_LENGTH = 8;

RH_RF95 rf95(LORA_SS, LORA_DI00);
const uint8_t msg_len = 15;
char message[msg_len];
uint8_t lora_len = msg_len + ECC_LENGTH;
uint8_t lora_buf[msg_len + ECC_LENGTH];

RS::ReedSolomon<msg_len, ECC_LENGTH> rs;
U8G2_SSD1306_128X64_NONAME_F_SW_I2C display(U8G2_R0, DISPLAY_SCL, DISPLAY_SDA, DISPLAY_RST);

int payload_count = 0;

//================================================================================================

inline void displayConfig()
{
  display.setDisplayRotation(U8G2_R2);
  display.setFont(u8g2_font_9x15_me);
  //display.setDrawColor(0);
  display.clearBuffer();
  display.drawXBM(0, 0, logo_width, logo_height, (uint8_t*) logo_bits);
  display.sendBuffer();
  delay(2000);
}

//------------------------------------------------------------------------------------------------

inline void displayInit()
{
  pinMode(DISPLAY_RST, OUTPUT);
  digitalWrite(DISPLAY_RST, LOW);
  delay(50);
  digitalWrite(DISPLAY_RST, HIGH);
  delay(50);
  if(!display.begin())
    Serial.println("Display: WARNING! Init failed!");
  else
    Serial.println("Display: Init OK!"); 
  displayConfig();
}

//------------------------------------------------------------------------------------------------

inline void loraInit()
{
  pinMode(LORA_RST, OUTPUT);
  digitalWrite(LORA_RST, LOW);
  delay(100);
  digitalWrite(LORA_RST, HIGH);

  if (!rf95.init()) 
    Serial.println("LoRa Radio: WARNING! Init failed.");
  else
    Serial.println("LoRa Radio: Init OK!");

  //125 KHz, 4/5 CR, SF7, CRC On, Low Data Rate Op. Off, AGC On  
  RH_RF95::ModemConfig myconfig = {RH_RF95_BW_125KHZ | RH_RF95_CODING_RATE_4_5,
                                   RH_RF95_SPREADING_FACTOR_128CPS | RH_RF95_PAYLOAD_CRC_ON,
                                   RH_RF95_AGC_AUTO_ON}; 
  rf95.setModemRegisters(&myconfig);

  if (!rf95.setFrequency(LORA_FREQ))
    Serial.println("LoRa Radio: WARNING! setFrequency failed.");
  else
    printf("LoRa Radio: Freqency set to %.1f MHz\n", LORA_FREQ);

  printf("LoRa Radio: Max Msg size: %u Bytes\n", RH_RF95_MAX_MESSAGE_LEN); 
}

//================================================================================================

void setup() {
  Serial.begin(9600);
  
  Serial.println("Starting Transmitter...");

  displayInit();
  loraInit();
  
  display.clearBuffer();
  display.drawStr(0, (FONT_HEIGHT + 2), "Waiting Msg...");
  Serial.println("Waiting payload");
  display.sendBuffer();
}

//------------------------------------------------------------------------------------------------

void loop() {
  if (rf95.available())
  {
    if (rf95.recv(lora_buf, &lora_len))
    {
      payload_count++;

      int line = 1;
  
      rs.Decode(message, (char *)lora_buf);
      display.clearBuffer();
  
      display.drawStr(0, line * (FONT_HEIGHT + 2), "Payload Recv:");
      line = 3;
      display.setCursor(0, line * (FONT_HEIGHT + 2));
      display.print(payload_count);
      Serial.println("Payload Recieved");
  
      display.sendBuffer();
    }
  }
  delay(10);
}
