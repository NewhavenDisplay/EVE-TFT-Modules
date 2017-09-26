/*

Copyright (c) Future Technology Devices International 2014

THIS SOFTWARE IS PROVIDED BY FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES LOSS OF USE, DATA, OR PROFITS OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FTDI DRIVERS MAY BE USED ONLY IN CONJUNCTION WITH PRODUCTS BASED ON FTDI PARTS.

FTDI DRIVERS MAY BE DISTRIBUTED IN ANY FORM AS LONG AS LICENSE INFORMATION IS NOT MODIFIED.

IF A CUSTOM VENDOR ID AND/OR PRODUCT ID OR DESCRIPTION STRING ARE USED, IT IS THE
RESPONSIBILITY OF THE PRODUCT MANUFACTURER TO MAINTAIN ANY CHANGES AND SUBSEQUENT WHQL
RE-CERTIFICATION AS A RESULT OF MAKING THESE CHANGES.

Abstract:

This file contains is functions for all UI fields.

Author : FTDI

Revision History: 
0.1 - date 2013.04.24 - initial version
0.2 - date 2013.08.19 - few minor edits

*/

#ifndef _SDCARD_H_
#define _SDCARD_H_
#include "FT_DataTypes.h"
#define VERBOSE 0

struct dirent {
  char name[8];
  char ext[3];
  byte attribute;
  byte reserved[8];
  uint16_t cluster_hi;  // FAT32 only
  uint16_t time;
  uint16_t date;
  uint16_t cluster;
  uint32_t size;
};

// https://www.sdcard.org/downloads/pls/simplified_specs/Part_1_Physical_Layer_Simplified_Specification_Ver_3.01_Final_100518.pdf
// page 22
// http://mac6.ma.psu.edu/space2008/RockSat/microController/sdcard_appnote_foust.pdf
// http://elm-chan.org/docs/mmc/mmc_e.html
// http://www.pjrc.com/tech/8051/ide/fat32.html

#define FAT16 0
#define FAT32 1

class sdcard {
  public:
  void sel() { digitalWrite(pin, LOW); }
  void desel() {
    digitalWrite(pin, HIGH);
    SPI.transfer(0xff); // force DO release
  }
  void sd_delay(byte n) {
    while (n--)
      SPI.transfer(0xff);
  }

  void cmd(byte cmd, uint32_t lba = 0, uint8_t crc = 0x95) {
#if VERBOSE
    Serial.print("cmd ");
    Serial.print(cmd, DEC);
    Serial.print(" ");
    Serial.print(lba, HEX);
    Serial.println();
#endif

    sel();
    SPI.transfer(0xff);
    SPI.transfer(0x40 | cmd);
    SPI.transfer(0xff & (lba >> 24));
    SPI.transfer(0xff & (lba >> 16));
    SPI.transfer(0xff & (lba >> 8));
    SPI.transfer(0xff & (lba));
    SPI.transfer(crc);
    SPI.transfer(0xff);
  }

  byte R1() 
  {     // read response R1
    byte r;
    
   while ((r = SPI.transfer(0xff)) & 0x80);
      desel();
      SPI.transfer(0xff);  
    return r;
  }

  byte sdR3(uint32_t &ocr) {  // read response R3
    uint32_t r;
    while ((r = SPI.transfer(0xff)) & 0x80)
      ;
    for (byte i = 4; i; i--)
      ocr = (ocr << 8) | SPI.transfer(0xff);
    SPI.transfer(0xff);   // trailing byte

    desel();
    return r;
  }

  byte sdR7() {  // read response R3
    uint32_t r;
    while ((r = SPI.transfer(0xff)) & 0x80)
      ;
    for (byte i = 4; i; i--)
      // Serial.println(SPI.transfer(0xff), HEX);
      SPI.transfer(0xff);
    desel();

    return r;
  }

  void appcmd(byte cc, uint32_t lba = 0) {
    cmd(55); R1();
    cmd(cc, lba);
  }

  ft_uint8_t begin(byte p)
  {
    ft_uint16_t cts=2000;
    pin = p;


    pinMode(pin, OUTPUT);
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV64);
    desel();

    delay(10);      // wait for boot
    sd_delay(10);   // deselected, 80 pulses

    // Tty.printf("Attempting card reset... ");
    // attempt reset
    byte r1,r;
    do
    {       // reset, enter idle
      cmd(0);
      do
      {      
         if(cts>0) cts--; else 
         {
           desel();
           return  0;
         }  
       }while ((r = SPI.transfer(0xff)) & 0x80);
      desel();
      SPI.transfer(0xff);   // trailing byte
      r1 = r;
    }while(r1!=1);
    // Tty.printf("reset ok\n");

    byte sdhc = 0;
    cmd(8, 0x1aa, 0x87);
    r1 = sdR7();
    sdhc = (r1 == 1);

    // Tty.printf("card %s SDHC\n", sdhc ? "is" : "is not");

    // Tty.printf("Sending card init command... ");
    while (1) {
      appcmd(41, sdhc ? (1UL << 30) : 0); // card init
      r1 = R1();
      if ((r1 & 1) == 0)
        break;
      delay(100);
    }
    // Tty.printf("OK\n");

    if (sdhc) {
      cmd(58);
      uint32_t OCR;
      sdR3(OCR);
      ccs = 1UL & (OCR >> 30);
      // Tty.printf("OCR register is %#010lx\n", long(OCR));
    } else {
      ccs = 0;
    }
    // Tty.printf("ccs = %d\n", ccs);

    byte type_code = rd(0x1be + 0x4);
    switch (type_code) {
      default:
        type = FAT16;
        break;
      case 0x0b:
      case 0x0c:
        type = FAT32;
        break;
    }
    // Tty.printf("Type code %#02x means FAT%d\n", type_code, (type == FAT16) ? 16 : 32);

    o_partition = 512L * rd4(0x1be + 0x8);
    sectors_per_cluster = rd(o_partition + 0xd);
    reserved_sectors = rd2(o_partition + 0xe);
    cluster_size = 512L * sectors_per_cluster;

    // Tty.printf("Bytes per sector:    %d\n", rd2(o_partition + 0xb));
    // Tty.printf("Sectors per cluster: %d\n", sectors_per_cluster);

    if (type == FAT16) {
      max_root_dir_entries = rd2(o_partition + 0x11);
      sectors_per_fat = rd2(o_partition + 0x16);
      o_fat = o_partition + 512L * reserved_sectors;
      o_root = o_fat + (2 * 512L * sectors_per_fat);
      // data area starts with cluster 2, so offset it here
      o_data = o_root + (max_root_dir_entries * 32L) - (2L * cluster_size); 
    } else {
      uint32_t sectors_per_fat = rd4(o_partition + 0x24);
      root_dir_first_cluster = rd4(o_partition + 0x2c);
      uint32_t fat_begin_lba = (o_partition >> 9) + reserved_sectors;
      uint32_t cluster_begin_lba = (o_partition >> 9) + reserved_sectors + (2 * sectors_per_fat);

      o_fat = 512L * fat_begin_lba;
      o_root = (512L * (cluster_begin_lba + (root_dir_first_cluster - 2) * sectors_per_cluster));
      o_data = (512L * (cluster_begin_lba - 2 * sectors_per_cluster));
    }
    return 1;
  }
  void cmd17(uint32_t off) {
    if (ccs)
      cmd(17, off >> 9);
    else
      cmd(17, off & ~511L);
    R1();
    sel();
    while (SPI.transfer(0xff) != 0xfe)
      ;
  }
  void rdn(byte *d, uint32_t off, uint16_t n) {
    cmd17(off);
    uint16_t i;
    uint16_t bo = (off & 511);
    for (i = 0; i < bo; i++)
      SPI.transfer(0xff);
    for (i = 0; i < n; i++)
      *d++ = SPI.transfer(0xff);
    for (i = 0; i < (514 - bo - n); i++)
      SPI.transfer(0xff);
    desel();
  }

  uint32_t rd4(uint32_t off) {
    uint32_t r;
    rdn((byte*)&r, off, sizeof(r));
    return r;
  }

  uint16_t rd2(uint32_t off) {
    uint16_t r;
    rdn((byte*)&r, off, sizeof(r));
    return r;
  }

  byte rd(uint32_t off) {
    byte r;
    rdn((byte*)&r, off, sizeof(r));
    return r;
  }
  byte pin;
  byte ccs;

  byte type;
  uint16_t sectors_per_cluster;
  uint16_t reserved_sectors;
  uint16_t max_root_dir_entries;
  uint16_t sectors_per_fat;
  uint16_t cluster_size;
  uint32_t root_dir_first_cluster;

  // These are all linear addresses, hence the o_ prefix
  uint32_t o_partition;
  uint32_t o_fat;
  uint32_t o_root;
  uint32_t o_data;
};

sdcard SD;

static void dos83(byte dst[11], const char *ps)
{
  byte i = 0;
  while (*ps) {
    if (*ps != '.')
      dst[i++] = toupper(*ps);
    else {
      while (i < 8)
        dst[i++] = ' ';
    }
    ps++;
  }
  while (i < 11)
    dst[i++] = ' ';
}

class Reader {
public:
  int openfile(const char *filename) {
    int i = 0;
    byte dosname[11];
    dirent de;

    dos83(dosname, filename);
    do {
      SD.rdn((byte*)&de, SD.o_root + i * 32, sizeof(de));
      // Serial.println(de.name);
      if (0 == memcmp(de.name, dosname, 11)) {
        begin(de);
        return 1;
      }
      i++;
    } while (de.name[0]);
    return 0;
  }
  
  void begin(dirent &de) {
    size = de.size;
    cluster = de.cluster;
    if (SD.type == FAT32)
      cluster |= ((long)de.cluster_hi << 16);
    sector = 0;
    offset = 0;
  }
  void nextcluster() {
    if (SD.type == FAT16)
      cluster = SD.rd2(SD.o_fat + 2 * cluster);
    else
      cluster = SD.rd4(SD.o_fat + 4 * cluster);
#if VERBOSE
    Serial.print("nextcluster=");
    Serial.println(cluster, DEC);
#endif
  }
  void skipcluster() {
    nextcluster();
    offset += SD.cluster_size;
  }
  void skipsector() {
    if (sector == SD.sectors_per_cluster) {
      sector = 0;
      nextcluster();
    }
    sector++;
    offset += 512;
  }
  void seek(uint32_t o) {
    while (offset < o) {
      if ((sector == SD.sectors_per_cluster) && ((o - offset) > (long)SD.cluster_size))
        skipcluster();
      else
        skipsector();
    }
  }
  void readsector() {
    if (sector == SD.sectors_per_cluster) {
      sector = 0;
      nextcluster();
    }
    uint32_t off = SD.o_data + ((long)SD.cluster_size * cluster) + (512L * sector);
#if VERBOSE
    Serial.print("off=0x");
    Serial.println(off, HEX);
#endif
    SD.cmd17(off & ~511L);
// Serial.println(2 * (micros() - t0), DEC);
    sector++;
    offset += 512;
  }
  void readsector(byte *dst) {
    readsector();
    for (int i = 0; i < 64; i++) {
      *dst++ = SPI.transfer(0xff);
      *dst++ = SPI.transfer(0xff);
      *dst++ = SPI.transfer(0xff);
      *dst++ = SPI.transfer(0xff);
      *dst++ = SPI.transfer(0xff);
      *dst++ = SPI.transfer(0xff);
      *dst++ = SPI.transfer(0xff);
      *dst++ = SPI.transfer(0xff);
    }
    SPI.transfer(0xff);   // consume CRC
    SPI.transfer(0xff);
    SD.desel();
  }
  uint32_t cluster;
  uint32_t offset;
  uint32_t size;
  byte sector;
};


#endif
