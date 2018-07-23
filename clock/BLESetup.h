/*
 * Handles all of the BLE stuff
 */
#if(USE_BLE)

#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"

#define BLE_READPACKET_TIMEOUT    500   // Timeout in ms waiting to read a response
#define FACTORYRESET_ENABLE         1
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

bool BLE_Connected;   //Current BLE connection status

//Checks to see if BLE is connected and if the status has changed it reports it
//by voice or serial message.
bool Update_BLE_Connected(void) {
  if (BLE_Connected) {
    if (! ble.isConnected()) {
      BLE_Connected= false;
      MESSAGE ("ble_dis.mp3", "BLE disconnected\n");
    }
  } else {
    if (ble.isConnected()) {
      BLE_Connected= true;
      MESSAGE("ble_con.mp3", "BLE connected\n");
    }
  }
  return BLE_Connected;
}

bool BluefruitSetup(void) {
  BLE_Connected=false;
  if ( !ble.begin(false))  {
    return false;
  } 
  if ( FACTORYRESET_ENABLE ) {
    if ( ! ble.factoryReset() ){
      return false;
    }
  }
  if (! ble.sendCommandCheckOK(F( "AT+GAPDEVNAME=NeoPixel-Clock" )) ) {
    return false;
  }
  ble.echo(false);
  ble.verbose(false);
  ble.setMode(BLUEFRUIT_MODE_DATA);
  return true;
}

/*
 * This is a stripped-down version of packetParser.cpp using only parts we need.
 * Probably could be stripped-down more.
 */
#define PACKET_ACC_LEN                  (15)
#define PACKET_GYRO_LEN                 (15)
#define PACKET_MAG_LEN                  (15)
#define PACKET_QUAT_LEN                 (19)
#define PACKET_BUTTON_LEN               (5)
#define PACKET_COLOR_LEN                (6)
#define PACKET_LOCATION_LEN             (15)
#define READ_BUFSIZE                    (20)

// Buffer to hold incoming characters 
uint8_t packetbuffer[READ_BUFSIZE+1];

uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout) {
  uint16_t origtimeout = timeout, replyidx = 0;
  memset(packetbuffer, 0, READ_BUFSIZE);
  while (timeout--) {
    if (replyidx >= 20) break;
    if ((packetbuffer[1] == 'A') && (replyidx == PACKET_ACC_LEN))
      break;
    if ((packetbuffer[1] == 'G') && (replyidx == PACKET_GYRO_LEN))
      break;
    if ((packetbuffer[1] == 'M') && (replyidx == PACKET_MAG_LEN))
      break;
    if ((packetbuffer[1] == 'Q') && (replyidx == PACKET_QUAT_LEN))
      break;
    if ((packetbuffer[1] == 'B') && (replyidx == PACKET_BUTTON_LEN))
      break;
    if ((packetbuffer[1] == 'C') && (replyidx == PACKET_COLOR_LEN))
      break;
    if ((packetbuffer[1] == 'L') && (replyidx == PACKET_LOCATION_LEN))
      break;
    while (ble->available()) {
      char c =  ble->read();
      if (c == '!') {
        replyidx = 0;
      }
      packetbuffer[replyidx] = c;
      replyidx++;
      timeout = origtimeout;
    }
    if (timeout == 0) break;
    delay(1);
  }
  packetbuffer[replyidx] = 0;  // null term
  if (!replyidx)  // no data or timeout 
    return 0;
  if (packetbuffer[0] != '!')  // doesn't start with '!' packet beginning
    return 0;
    // check checksum!
  uint8_t xsum = 0;
  uint8_t checksum = packetbuffer[replyidx-1];
  for (uint8_t i=0; i<replyidx-1; i++) {
    xsum += packetbuffer[i];
  }
  xsum = ~xsum;
  // Throw an error message if the checksum's don't match
  if (xsum != checksum)
  {
    DEBUG("Checksum mismatch in packet\n");
    return 0;
  }
  // checksum passed!
  return replyidx;
}

#endif //USE_BLE

