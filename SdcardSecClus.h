/* 
 This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

   WARNING!:DONOT USE THIS WITH SENSITIVE DATA. DATA MAY BE LOST,CORRUPTED,INCOMPLETE.MAY DAMAGE
  YOUR SDCARD. USE AT OWN RISK.
  
 USE WITH SDCARD FORMATED WITH FAT32 ONLY,SKETCH WILL ONLY READ SEQUENTIAL FAT ENTRIES
 (NOT FRAGMENTED FAT ENTRIES).SKETCH WRITES ONLY SEQUENTIAL FAT ENTRIES.YOU CAN READ
 CHARACTER OR HEX DATA FROM SECTORS(TRUE OR FALSE).READS AND LISTS FILE FROM ONLY 2 DIRECTORY LEVELS.
 IF YOU DELETE DATA  FROM FILE IN WINDOWS,FILE WILL BE CORRUPTED(WINDOWS WRITES 0X20
  TO THE ENTIRE REMAINING FILE???)YOU MUST DELETE THE FILE AND LET THE SKETCH CREATE IT AGAIN(ALL 
  DATA LOST).
*/

#include <SPI.h>
#include <Arduino.h>
#ifndef SdcardSecClus_h
#define SdcardSecClus_h

class SdcardSecClus
{
private:
uint8_t mwaitPin;
uint8_t csPin;
byte readRes1;
byte readRes3[5];
byte resp1;
uint8_t sdBuf[512];
 uint32_t  linesPrinted=0;
uint32_t firstFatSec=0;
uint32_t firstDataSec=0;
uint32_t fat32Sz=0;
uint16_t resvSec=0;
uint16_t secPerClus=0;
uint16_t bytePerSec=0;
uint8_t  dirEnt32[32];
uint16_t nextRtDirFreeEnt=0;
uint32_t targetFileData[5];
uint32_t targetDirData[2];
uint32_t lastMaxDirSecLoc=0;
uint32_t lastMaxFileSecLoc=0;
const int waitPin=2;
boolean targetDirFoundFlag=false;
boolean targetFileFoundFlag=false;
uint32_t write1BlkIndex=0;
long timeStamp=0;
String timeStampString;
char timeStampChar[16];
 uint8_t strSize=0;
  char lastWriteFileName[12]="            ";        
  uint16_t loopIndex=0;
 boolean fileStatus=false;


void readResp3();
uint8_t readResp1();
void postCmd();
void printResp3();
void sdPwrUp();
void sdCmd(uint8_t cmd,uint32_t arg,uint8_t crc);
uint8_t sdRead1Blk(uint32_t addr,boolean printSec);
uint8_t sdWrite1Blk(uint32_t addr);
void findMbrBpb(uint32_t volOffset);
void exploreDir(uint8_t dirEnt31[32],boolean printList,char targetFileOrDirName[12]);
void exploreFiles(uint8_t dirEnt33[32],boolean printList,char targetFileName[12]);
void listDirFiles(char nameDir[12],uint32_t dirClus,uint32_t dirSecLoc,boolean printList,char targetFileName[12]);
void explore2ndDir(uint8_t dirEnt31[32],boolean printList,char targetFileName[12]);
boolean  printRead1Blk(boolean hexTex);
void cleanDataSecForClusVal(uint32_t cleanEmptyClusValDataSec,uint32_t ffEmptyClusLoc);
void write32EntSec(uint32_t sector,uint16_t offsetSec,byte data32[32]);
uint32_t nextEmptyFatClus(uint32_t startFatSec,uint32_t amtClusReqFile);
uint32_t amtFreeClus(uint32_t clusFree,uint32_t amtClusReqForFile);
void listRootDirFiles(uint32_t rootClusSec,boolean printList,char targetFileName[12]);
void cleanTargetFileData();
void makeFileorDir(char fileDirName[12],char dirOrFile[3],uint32_t fileSize,uint32_t fileToDirSec, uint16_t fileToDirSecOffset);
void writeToFile(char writeFileName[12],char dataWrite[],uint16_t fileWriteSize,boolean programMemOrData);

public:
SdcardSecClus();
void sdBegin(uint16_t baudRate,uint8_t waitPin,uint8_t chipSelectPin);
void rootDirFilesList();
void readFile(char searchFileName[12],boolean hexOrChar);
void createDirorFile( char fileDirName[12],char dirOrFile[3],uint32_t fileSize);
void fileMakeInDir(char targetDir[12],char dirFile[12],uint32_t dirFileSize);
void writeNumToFile(char writeFile[12],uint32_t numData);
void writeStringToFile(char writeFile[12],String myString);
void writeProgMemToFile(char writeFile[12],char progMemFile[],uint16_t progMemFileSize);
};
#endif

