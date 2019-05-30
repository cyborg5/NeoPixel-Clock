//Adapted from sample code in "Adafruit_TinyUSB_Arduino\examples\MassStorage\msc_sdcard.ino
// The MIT License (MIT)
// Copyright (c) 2019 Ha Thach for Adafruit Industries

#include "Adafruit_TinyUSB.h"

//Used by USB drive code
const int chipSelect = CARDCS;
Adafruit_USBD_MSC usb_msc;
Sd2Card card;
SdVolume volume;

int32_t msc_read_cb (uint32_t lba, void* buffer, uint32_t bufsize)
{
  (void) bufsize;
  return card.readBlock(lba, (uint8_t*) buffer) ? 512 : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and 
// return number of written bytes (must be multiple of block size)
int32_t msc_write_cb (uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
  return card.writeBlock(lba, buffer) ? 512 : -1;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void msc_flush_cb (void)
{
  // nothing to do
}

void USB_setup()
{
  // Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
  usb_msc.setID("Adafruit", "SD Card", "1.0");

  // Set read write callback
  usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);

  // Still initialize MSC but tell usb stack that MSC is not ready to read/write
  // If we don't initialize, board will be enumerated as CDC only
  usb_msc.setUnitReady(false);
  usb_msc.begin();

//  Serial.begin(115200);
//No  while ( !Serial ) delay(10);   // wait for native usb

//  Serial.println("Adafruit TinyUSB Mass Storage SD Card example");

//  Serial.println("\nInitializing SD card...");

  if ( !card.init(SPI_HALF_SPEED, chipSelect) )
  {
    DEBUG ("Failed to set up USB drive/n");
    return;
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    DEBUG ("Could not find partition. Card not formatted?");
    return;
  }
  
  uint32_t block_count = volume.blocksPerCluster()*volume.clusterCount();

  DEBUG("Volume size (MB):  ");
  DEBUG((block_count/2) / 1024);
  DEBUG("\n");
  
  // Set disk size, SD block size is always 512
  usb_msc.setCapacity(block_count, 512);

  // MSC is ready for read/write
  usb_msc.setUnitReady(true);
}
