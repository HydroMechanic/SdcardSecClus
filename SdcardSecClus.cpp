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
  DATA DELETED FROM FILE IN WINDOWS WILL BE UNREADABE(WINDOWS WRITES 0X20
  TO THE ENTIRE REMAINING FILE???)YOU MUST DELETE THE FILE AND LET THE SKETCH CREATE IT AGAIN(ALL 
  DATA LOST).
 
*/

#include<SdcardSecClus.h>
#include<Arduino.h>


SdcardSecClus::SdcardSecClus()
{

}

void SdcardSecClus::sdBegin(uint16_t baudRate,uint8_t waitPin,uint8_t chipSelectPin)
{
csPin=chipSelectPin;
mwaitPin=waitPin;

  Serial.begin(baudRate);
  if(Serial.available())Serial.read();
  Serial.println(F("VERIFY SD CARD(FAT32)INSERTED THEN CLICK SEND OR PRESS ENTER TO CONTINUE"));
 SPI.begin();
  while(Serial.available()==0)
  {
       delay(500);
  }
  
 
pinMode(mwaitPin,INPUT_PULLUP);
 
  pinMode(csPin,OUTPUT);
 digitalWrite(csPin,HIGH);
 sdPwrUp();

 sdCmd(0,0x00000000,0x94);
resp1=readResp1();

postCmd();
Serial.println(F("send cmd0"));
Serial.println(resp1,BIN);
Serial.println(F("---------------"));

  sdCmd(8,0x000001aa,0x86);
readResp3();
  
postCmd();
Serial.println(F("send cmd8"));
printResp3();
Serial.println(F("--------------------"));

  sdCmd(58,0x00000000,0x00);
readResp3();
  
postCmd();
Serial.println(F("send cmd58"));
printResp3();
Serial.println(F("--------------------"));

do
{
 sdCmd(55,0x00000000,0x00);
postCmd();
Serial.println(F("send cmd55"));
Serial.println(F("---------------"));
  sdCmd(41,0x40000000,0x00);
resp1=readResp1();
   
postCmd();
Serial.println(F("send acmd41"));
Serial.println(resp1,BIN);
Serial.println(F("--------------------"));
}while(resp1!=0);

  sdCmd(58,0x00000000,0x00);
readResp3();
 
postCmd();
Serial.println(F("send cmd58"));
printResp3();
Serial.println(F("--------------------"));

findMbrBpb(0);
}

void SdcardSecClus::readResp3()
{
  SPI.transfer(0xFF);
  for (int h=0;h<5;h++)
  {
    readRes3[h]=SPI.transfer(0xff);
  }
}

byte SdcardSecClus::readResp1()
{
 uint8_t i=0;
  SPI.transfer(0xFF);

 do
  {
   readRes1=SPI.transfer(0xFF);
i++;
if(i>8){Serial.println(F("Response 1 error"));return;}
  } while(readRes1==0xff);

return readRes1;
}

void SdcardSecClus::postCmd()
{
  SPI.transfer(0xFF);
  digitalWrite(csPin,HIGH);
SPI.transfer(0xFF);
}

void SdcardSecClus::printResp3()
{
  for(int j=0;j<5;j++)
{
   Serial.println(readRes3[j],BIN);
}
}

 void SdcardSecClus::sdPwrUp()
{
  digitalWrite(csPin,HIGH);
delay(1);
  
    for(uint8_t i = 0; i < 10; i++)
    {
        SPI.transfer(0xFF);
    }
 
   SPI.transfer(0xFF);
       
}

void SdcardSecClus::sdCmd(uint8_t cmd,uint32_t arg,uint8_t crc)
{
     SPI.transfer(0xFF);
  digitalWrite(csPin,LOW);
  SPI.transfer(0xFF);
  SPI.transfer(cmd|0x40);
  SPI.transfer((byte)(arg>>24));
 SPI.transfer((byte)(arg>>16));
   SPI.transfer((byte)(arg>>8));
      SPI.transfer((byte)(arg));
   SPI.transfer(crc|0x01);
 
}

uint8_t SdcardSecClus::sdRead1Blk(uint32_t addr,boolean printSec)
{
uint16_t passes=1,itr=0,maxItr=3600;
 uint8_t res1=0xff,readFF=0xff;

    // send CMD17
   sdCmd(17, addr, 0x00);
 res1 = readResp1();//readResp1 functions return anything 
 //but 0xff; res1 received valid response from sd card
  
 if(res1==0)
 {
  while(itr++!=maxItr)
{
  readFF=SPI.transfer(0xff);
   if( readFF != 0xFF) break;
   }
 
  if(readFF==0xfe)
  {
   
for(int w=0;w<512;w++)
{
 sdBuf[w]=SPI.transfer(0xff);
}
 // read 16-bit CRC
SPI.transfer(0xFF);
 SPI.transfer(0xFF);
}

 }
      SPI.transfer(0xFF);
    digitalWrite(csPin,HIGH);
    SPI.transfer(0xFF);
    return res1; 
  }

uint8_t SdcardSecClus::sdWrite1Blk(uint32_t addr)
{
 byte res1Write=0xff,writeToken=0xff,busyToken=0xff;
 uint16_t writeItr=0,maxWriteItr=7000;
   sdCmd(24,addr,0x00);
 res1Write=readResp1();

 if(res1Write==0)
 {

  SPI.transfer(0xfe);
 
  for(int n=0;n<512;n++)
  {
  SPI.transfer(sdBuf[n]);
   }
  
  while(writeItr++!=maxWriteItr)
   {
     writeToken=SPI.transfer(0xff);
if(writeToken!=0xff)break;
   }
    if((writeToken&0x1f)==0x05)
    {
      writeItr=0;
    while(SPI.transfer(0xff)==0x00)
    {
      if(writeItr==maxWriteItr||SPI.transfer(0xff)!=0x00)
      {
        if(writeItr==maxWriteItr)
        {
          Serial.println(F("Maximum write iterations exceeded"));
        }
      break;
      }
    }
    }
   
     SPI.transfer(0xFF);
    digitalWrite(csPin,HIGH);
    SPI.transfer(0xFF);
      write1BlkIndex++;
    if(write1BlkIndex==1)
    {
            Serial.println(F("Writing Data Or Cleaning Cluster Please Wait"));
    }
  return res1Write;
}
}

void SdcardSecClus::findMbrBpb(uint32_t volOffset)
{
 sdRead1Blk(0+volOffset,false);

  if(sdBuf[0]==0xeb&&sdBuf[2]==0x90)
  {
    Serial.print(F("Master Boot Record detected Sector="));Serial.println(volOffset);
     bytePerSec=sdBuf[11] | sdBuf[12]<<8;
    Serial.println(bytePerSec);
  secPerClus=sdBuf[13];
   Serial.print(F("Sec Per Clus:")); Serial.println(secPerClus);
 resvSec=sdBuf[14]|sdBuf[15]<<8;
  Serial.print(F("Resv Sectors:")); Serial.println(resvSec);
   fat32Sz=sdBuf[36]|sdBuf[37]<<8|sdBuf[38]<<16|sdBuf[39]<<24;
Serial.print(F("Fat32 Size(sectors):"));  Serial.println(fat32Sz);
  firstDataSec=resvSec+(2*fat32Sz)+volOffset;
  Serial.print(F("First Data Sector(Root Directory):"));Serial.println(firstDataSec);
 firstFatSec=resvSec+volOffset;
  Serial.print(F("First FAT sector:"));Serial.println(firstFatSec);
  }
  else
  {
    unsigned long  volOffset=sdBuf[454]|sdBuf[455]<<8|sdBuf[456]<<16|sdBuf[457]<<24;
    //Serial.println(volOffset);
    findMbrBpb(volOffset);
  }
  Serial.println();
}


void SdcardSecClus::exploreDir(uint8_t dirEnt31[32],boolean printList,char targetFileOrDirName[12])
{
 char dirName[]="           ";
uint32_t dirClus=0;
uint32_t dirSecLoc=0;
uint8_t index=0;
char targetDirName[]="           ";

for(uint8_t u=0;u<8;u++)
{
  targetDirName[u]=targetFileOrDirName[u];
}
     for(int n=0;n<8;n++)
    {
    dirName[n]=dirEnt31[n];
     }
     
    dirClus=(uint32_t(dirEnt31[26]))|uint32_t((dirEnt31[27]))<<8|uint32_t((dirEnt31[20]))<<16|uint32_t((dirEnt31[21]))<<24;
    dirSecLoc=((dirClus-2)*secPerClus)+firstDataSec;
    for(uint8_t u=0;u<8;u++)
    {
      if(dirName[u]==targetDirName[u])
      {
        index++;
        if(index==8)
        {
          Serial.println(F("target Directory detected"));
          targetDirFoundFlag=true;
          targetDirData[0]=dirClus;
          targetDirData[1]=dirSecLoc;
    
          break;
        }
      }
    }
   
          listDirFiles(dirName,dirClus,dirSecLoc,printList,targetFileOrDirName);
      lastMaxDirSecLoc=max(lastMaxDirSecLoc,dirSecLoc);
      
      for(uint8_t v=0;v<32;v++)
{
  dirEnt31[v]=0;
}
 }

void SdcardSecClus::exploreFiles(uint8_t dirEnt33[32],boolean printList,char targetFileName[12])
{
   uint32_t fileLocClus=0;
 uint32_t fileFatSecNum=0;
   uint16_t fileFatSecOffset=0;
  char fileName[]="            ";
  uint8_t index=0;
uint32_t fileLocSec=0;
uint32_t fileSize=0;

     for(int w=0;w<11;w++)
    {
    fileName[w]=dirEnt33[w];
    }
    
     fileLocClus=uint32_t((dirEnt33[26]))|uint32_t((dirEnt33[27]))<<8|uint32_t((dirEnt33[20]))<<16|uint32_t((dirEnt33[21]))<<24;
     fileLocSec=((fileLocClus-2)*secPerClus)+firstDataSec;
     fileFatSecNum=((fileLocClus*4)/512)+firstFatSec;
     fileFatSecOffset=(fileLocClus*4)%512;
     fileSize=uint32_t((dirEnt33[28]))|uint32_t((dirEnt33[29]))<<8|uint32_t((dirEnt33[30]))<<16|uint32_t((dirEnt33[31]))<<24;
     lastMaxFileSecLoc=max(lastMaxFileSecLoc,fileLocSec);

   for(uint8_t r=0;r<11;r++)
   {
        if(targetFileName[r]==fileName[r])
      {
        index++;
        if(index==11)
        {
          Serial.println();
          Serial.println(F("targetFile detected"));
          targetFileFoundFlag=true;
          targetFileData[0]=fileLocClus;
          targetFileData[1]=fileLocSec;
          targetFileData[2]=fileFatSecNum;
          targetFileData[3]=fileFatSecOffset;
          targetFileData[4]=fileSize;
          return;
        }
      }
     }
   
     if(printList==true)
     {
    Serial.print(F("FileName:"));Serial.print(fileName);
    Serial.print(F("FAT Cluster(DEC):"));Serial.print(fileLocClus);
    Serial.print(F(" Sector:"));Serial.print(fileLocSec);
    Serial.print(F(" Fat Sector No.:"));Serial.print(fileFatSecNum);
    Serial.print(F(" Fat Sector Offset:"));Serial.println(fileFatSecOffset);
     }
 }

void SdcardSecClus::listDirFiles(char nameDir[12],uint32_t dirClus,uint32_t dirSecLoc,boolean printList,char targetFileName[12])
{
  uint16_t  secDirIndex=0;
  uint16_t dirOffset32=0;
  
 if(printList==true)
 {
   Serial.println(F("***************************************"));
  Serial.print(F(" Root Directory Name:"));Serial.print(nameDir);
  Serial.print(F("FAT Cluster(DEC):"));Serial.print(dirClus);
  Serial.print(F(" Sector:"));Serial.println(dirSecLoc);
  Serial.println(F("***************************************"));
 }
 do{
   dirOffset32=0;
  
 sdRead1Blk(dirSecLoc+secDirIndex, false);//r1Val removed
  while(dirOffset32<512)
  {
  
    if(sdBuf[dirOffset32]==0x2e){dirOffset32+=32;continue;}
 if(sdBuf[dirOffset32]==0xe5){dirOffset32+=32;continue;}
if(sdBuf[dirOffset32]==0x00){return;}
  for(int j=0;j<32;j++)
  {
    dirEnt32[j]=sdBuf[j+dirOffset32];
   }
       if(dirEnt32[11]==0x20)
  {
       exploreFiles(dirEnt32,printList,targetFileName);
     }

  if(dirEnt32[11]==0x10)
  {
    explore2ndDir(dirEnt32,printList,targetFileName);
  sdRead1Blk(dirSecLoc+secDirIndex, false);
    for(int j=0;j<32;j++)
  {
    dirEnt32[j]=sdBuf[j+dirOffset32];
   }
  }

  
    dirOffset32+=32;
   } 
   secDirIndex++;
}while(true);

}

void SdcardSecClus::explore2ndDir(uint8_t dirEnt31[32],boolean printList,char targetFileName[12])
{
 char dirName[]="            ";
uint32_t dirClus=0;
uint32_t dirSecLoc=0;
uint16_t sec2ndDirIndex=0;
uint16_t dir2ndOffset32=0;
byte dirEnt33[32];
     for(int n=0;n<11;n++)
    {
    dirName[n]=dirEnt31[n];
    
    }
    dirClus=uint32_t((dirEnt31[26]))|uint32_t((dirEnt31[27]))<<8|uint32_t((dirEnt31[20]))<<16|uint32_t((dirEnt31[21]))<<24;
    dirSecLoc=((dirClus-2)*secPerClus)+firstDataSec;
    lastMaxDirSecLoc=max(lastMaxDirSecLoc,dirSecLoc);
    if(printList==true)
    {
    Serial.print(F("Second Directory:"));Serial.print(dirName);
    Serial.print(F(" Cluster:"));Serial.print(dirClus);
    Serial.print(F(" Sector:"));Serial.println(dirSecLoc);
    }
   do{
   dir2ndOffset32=0;
   sdRead1Blk(dirSecLoc+sec2ndDirIndex, false);//r1Val removed
  while(dir2ndOffset32<512)

  {
      if(sdBuf[dir2ndOffset32]==0x2e){dir2ndOffset32+=32;continue;}
 if(sdBuf[dir2ndOffset32]==0xe5){dir2ndOffset32+=32;continue;}
if(sdBuf[dir2ndOffset32]==0x00){return;}
  for(int j=0;j<32;j++)
  {
    dirEnt33[j]=sdBuf[j+dir2ndOffset32];
   }
       if(dirEnt33[11]==0x20)
  {
       exploreFiles(dirEnt33,printList,targetFileName);
   }
  
    dir2ndOffset32+=32;
   } 
   sec2ndDirIndex++;
}while(dirEnt33[0]!=0x00);
 }



void SdcardSecClus::cleanDataSecForClusVal(uint32_t cleanEmptyClusValDataSec,uint32_t ffEmptyClusLoc)
{
  write1BlkIndex=0;
  uint32_t cleanDataSec=0;
  if(cleanEmptyClusValDataSec==0xfffffff)cleanEmptyClusValDataSec=ffEmptyClusLoc;
  for(uint16_t h=0;h<512;h++)
  {
    sdBuf[h]='\0';
  }
  cleanDataSec=(cleanEmptyClusValDataSec-3)*secPerClus+firstDataSec;
  for(uint8_t y=0;y<secPerClus;y++)
  {
    sdWrite1Blk(cleanDataSec+y);
  }
 }

void SdcardSecClus::write32EntSec(uint32_t sector,uint16_t offsetSec,byte data32[32])
{
 uint8_t res1Write=0xff,writeToken=0xff,busyToken=0xff;
 uint16_t writeItr=0,maxWriteItr=7000;
  
sdRead1Blk(sector,false);
   sdCmd(24,sector,0x00);
  res1Write=readResp1();
  if(res1Write==0)
  {
 delay(2);
  SPI.transfer(0xfe);

 for(int j=0;j<512;j++)
 {
  if(j>=offsetSec&&j<offsetSec+32)
  {
   sdBuf[j]=data32[j-offsetSec];
   SPI.transfer( sdBuf[j]);
  }
  else
  {
  SPI.transfer( sdBuf[j]);
 }
 }
  while(++writeItr!=maxWriteItr)
   {
     writeToken=SPI.transfer(0xff);
     if(writeToken!=0xff)break;
   }
    if((writeToken&0x1f)==0x05)
    {
      Serial.println(F("Writing Data"));
    while(SPI.transfer(0xff)==0x00)
    {
      if(writeItr==maxWriteItr||SPI.transfer(0xff)!=0x00)
      {
        if(writeItr==maxWriteItr)
        {
          Serial.println(F("Maximum write iterations exceeded"));
        }
        break;
      }
    }
    }
          SPI.transfer(0xFF);
    digitalWrite(csPin,HIGH);
    SPI.transfer(0xFF);
    return res1Write;
}
}

uint32_t SdcardSecClus::nextEmptyFatClus(uint32_t startFatSec,uint32_t amtClusReqFile)
{
  uint32_t fatSecIndex=0;
uint32_t clus0[]={1,1,1,1};
  uint16_t itr4Index=0;
  uint32_t freeClus=0;
  uint32_t clus0Val=1;
uint32_t searchFreeClus=0;
uint32_t lastSearchFreeClus=0;
  
  do{
  sdRead1Blk(startFatSec+fatSecIndex,false);
  itr4Index=0;
while(itr4Index<512)
{
     
      for(uint8_t e=0;e<4;e++)
      {
      clus0[e]=sdBuf[e+itr4Index];
      }
   
    clus0Val=clus0[0]|clus0[1]<<8|clus0[2]<<16|clus0[3]<<24;
      if(clus0Val==0)
    {
            freeClus=fatSecIndex*128+((startFatSec-firstFatSec)*128)+(itr4Index/4);
         Serial.print(F("Next free Fat Cluster(HEX):"));Serial.println(freeClus,HEX);
    searchFreeClus=amtFreeClus(freeClus,amtClusReqFile);//check amt empty fat clus

    if(searchFreeClus==freeClus)
    {
      return freeClus;
    }
    else
    {
   while(searchFreeClus!=lastSearchFreeClus)
    {
      if(lastSearchFreeClus!=0)
      {
      searchFreeClus=lastSearchFreeClus;
      }
      lastSearchFreeClus=amtFreeClus(searchFreeClus,amtClusReqFile);
      
    }
     return searchFreeClus;
    }
    }
    itr4Index+=4;
  }
  fatSecIndex++;
  }while(true);
 }

uint32_t SdcardSecClus::amtFreeClus(uint32_t clusFree,uint32_t amtClusReqForFile)
{
uint16_t auxFatSecOffset=0;
uint32_t fatClus00Val=0;
uint16_t  emptyClusIndex=0;
uint32_t next0FatClusSec=0;
uint16_t fat0SecOffset=0;
uint32_t fat00SecIndex=0;
uint32_t fat00Clus[]={1,1,1,1};
uint32_t  elapsedClus00=0;
uint32_t  lastEmptyFatClusLoc=0;
uint32_t searchClusFree=0;

  next0FatClusSec=((clusFree*4)/512)+firstFatSec;
  fat0SecOffset=(clusFree*4)%512;
  
  do{
  sdRead1Blk(next0FatClusSec+fat00SecIndex,false);
  emptyClusIndex=0;
while(emptyClusIndex+fat0SecOffset<512)
{
  
      for(uint8_t s=0;s<4;s++)
      {
      fat00Clus[s]=sdBuf[s+emptyClusIndex+fat0SecOffset];
      }
       
      fatClus00Val=fat00Clus[0]|fat00Clus[1]<<8|fat00Clus[2]<<16|fat00Clus[3]<<24;
     
      //Serial.println(fatClus00Val,HEX);
if(fat00SecIndex>1)
{
  elapsedClus00=(((fat00SecIndex-1)*512)+emptyClusIndex+(512-auxFatSecOffset))/4;
}
if(fat00SecIndex==1)
{
  elapsedClus00=(emptyClusIndex+(512-auxFatSecOffset))/4;
}
 if(fat00SecIndex==0)
{
  elapsedClus00=emptyClusIndex/4+1;
 
}
      
    if((fatClus00Val!=0)||(elapsedClus00==amtClusReqForFile))
    {
      if(elapsedClus00==amtClusReqForFile)
      {
       Serial.println(F("file size ok"));
        return clusFree;
      }
      else
      {
        searchClusFree=elapsedClus00+clusFree;
        return searchClusFree;
      }
         }
        emptyClusIndex +=4;
   }
   
if(fat00SecIndex==0)  auxFatSecOffset=fat0SecOffset;
  fat0SecOffset=0;
  fat00SecIndex++;
  }while(true);
 }


void SdcardSecClus::rootDirFilesList()
{
listRootDirFiles(firstDataSec,true," ");
}


void SdcardSecClus::listRootDirFiles(uint32_t rootClusSec,boolean printList,char targetFileName[12])
{
  uint16_t secIndex=0;
   uint16_t offset32=0;

   for(uint8_t c=0;c<32;c++)
   {
    dirEnt32[c]=0;
   }
    if(printList==true)
    {
Serial.println(F("Root Directory Directories and Files"));
 Serial.println(F("***************************************"));
    }
 do{
  offset32=0;
 
   sdRead1Blk(rootClusSec+secIndex,false);//r1Val removed
  while(offset32<512)
  {
  if(sdBuf[offset32]==0xe5){ offset32+=32;continue;}
if(sdBuf[offset32]==0x00){goto endOfSec;}
  for(int j=0;j<32;j++)
  {
    dirEnt32[j]=sdBuf[j+offset32];
   }
  if(dirEnt32[11]==0x10)
  {
   exploreDir(dirEnt32,printList,targetFileName);
  sdRead1Blk(rootClusSec+secIndex,false);//r1Val removed
   for(int j=0;j<32;j++)
  {
    dirEnt32[j]=sdBuf[j+offset32];
   }
    }
  
   if(dirEnt32[11]==0x20)
  {
       if(printList==true)
       {
         Serial.println(F("***********************************"));
Serial.println(F("Stand Alone File Root Directory"));
       }
    exploreFiles(dirEnt32,printList,targetFileName);

     }
    offset32+=32;
   }
       secIndex++;
  }while(true);
  
endOfSec:

int lastSecOffset=(offset32%512);
 nextRtDirFreeEnt=(secIndex*512)+lastSecOffset;


  }

void SdcardSecClus::readFile(char searchFileName[12],boolean hexOrChar)
{

  uint16_t fatSecIndex=0;
 uint16_t indexFat=0;
boolean entry00=true;
  uint32_t fatClusVal[4];
  uint32_t fatClusValNum=0;
  uint32_t lastfatClusValLoc=0;
   uint32_t fatClusValLoc=0;
   uint32_t readSector=0;
 boolean found00=true;
   uint32_t targetFileFatSec=0;
   uint32_t targetFileFatSecOffset=0;
   
cleanTargetFileData();
 listRootDirFiles(firstDataSec,false,searchFileName);
 if(targetFileData[1]==0)//fileLocSec
 {
  Serial.println();
  Serial.println(F("----------------------------"));
  Serial.print(searchFileName);Serial.println(F(" NOT FOUND"));
  Serial.println(F("----------------------------"));
  return;
 }
 targetFileFatSec=targetFileData[2];//fileFatSecNum
 targetFileFatSecOffset=targetFileData[3];//fileFatSecOffset
Serial.print(F("Target File Found:"));Serial.println(searchFileName);
Serial.print(F("Target File Fat Sec Start:"));Serial.println(targetFileFatSec);
Serial.print(F("Target File Fat Sec Start Offset :"));Serial.println(targetFileFatSecOffset);
Serial.print(F("Target File Start FAT Cluster Location(DEC):"));Serial.println(targetFileData[0]);
Serial.print(F("Target File Start Sector Location:"));Serial.println(targetFileData[1]);
Serial.println(F("Reading File Please Wait"));
do{
sdRead1Blk(targetFileFatSec+fatSecIndex,false);

indexFat=0;
while((targetFileFatSecOffset+indexFat)<512)
{
for(int q=0;q<4;q++)
{
  fatClusVal[q]=sdBuf[targetFileFatSecOffset+(indexFat+q)];
}
lastfatClusValLoc=fatClusValLoc;
 fatClusValNum=fatClusVal[0]|fatClusVal[1]<<8|fatClusVal[2]<<16|fatClusVal[3]<<24;

 fatClusValLoc=fatClusValNum-1;
 
if(fatClusValNum==0xfffffff)
{
 // Serial.print("targetFileData[3]:");Serial.println(targetFileData[3]);
 
indexFat=(indexFat)+4;
if(fatSecIndex>1)
{
  indexFat=indexFat+((fatSecIndex-1)*512)+(512-targetFileData[3]);
}
else if(fatSecIndex==1)
{
  indexFat=indexFat+(512-targetFileData[3]);
}

 if(indexFat==4&fatSecIndex==0)
{
  lastfatClusValLoc=targetFileData[0]-1;
 
}
 readSector=(lastfatClusValLoc-1)*secPerClus+firstDataSec;
   
   for(uint8_t k=0;k<secPerClus;k++)
 {
  
sdRead1Blk(readSector+k,true);
  found00=printRead1Blk(hexOrChar);
 // Serial.print("found00:");Serial.println(found00);
if(found00==1)Serial.println();
if(found00==false)break;
}
//sdRead1Blk(targetFileFatSec+fatSecIndex,false);
Serial.println();
Serial.print(F("Target File End Cluster:")); Serial.println(lastfatClusValLoc+1); 
Serial.print(F("Target Flie Total Clusters:"));Serial.println(indexFat/4);
Serial.println(F("end of file "));
Serial.println();
return;
}
else
{
//Serial.print(F("Cluster Num:")); Serial.println(fatClusNumless1);
  readSector=(fatClusValLoc-2)*secPerClus+firstDataSec;
  if((abs(fatClusValLoc-lastfatClusValLoc))!=1&&lastfatClusValLoc!=0)
  {
    Serial.println();
    Serial.println(F("NON SEQUENTIONAL FAT ENTRY DETECTED,FAT ENTRIES FRAGMENTED-READ ERROR"));
    return;
  }
 for(uint8_t k=0;k<secPerClus;k++)
 {
sdRead1Blk(readSector+k,true);
entry00=true;
entry00=printRead1Blk(hexOrChar);
//Serial.print("entry00:");Serial.println(entry00);
if(entry00==1)Serial.println();
if(entry00==false)return;
}
sdRead1Blk(targetFileFatSec+fatSecIndex,false);
}
  indexFat+=4;
}
targetFileFatSecOffset=0;
fatSecIndex++;
 }while(true);
}

void SdcardSecClus::cleanTargetFileData()
{
  for(uint8_t p=0;p<4;p++)
  {
    targetFileData[p]=0;
    if(p<2)targetDirData[p]=0;
  }
}


boolean SdcardSecClus::printRead1Blk(boolean hexTex)//char output true;hex output false
{
  uint8_t itrStamp=0;
  uint16_t print00Index=0;
 

   for(uint16_t m=0;m<512;m++)
   {
 if(hexTex==true)
 {
      Serial.write(sdBuf[m]);
       if(sdBuf[m]=='\0')
       {
        if(print00Index==0)
        {
          itrStamp=m;
        }
        print00Index++;
        if(print00Index==32&&(m-itrStamp)==31)
        {
          print00Index=0;
          itrStamp=0;
          return false;
        }
       }
 }
 else
 {
      if(sdBuf[m]=='\0')
       {
          if(print00Index==0)
        {
          itrStamp=m;
        }
        print00Index++;
       
        
        print00Index++;
        if(print00Index==512&&(m-itrStamp)==511)
        {
          print00Index=0;
           itrStamp=0;
          return false;
        }
       }
  if(sdBuf[m]<16)Serial.print('0');
 
    Serial.print(sdBuf[m],HEX);
  if((m+1)%32==0)
  {
    Serial.println();
   linesPrinted++;
   
    if(linesPrinted==58000)//Serial monitor can hold only so much data(approx 1.8MB),copy data,clear monitor,grd waitPin
    {
      Serial.print(F("Copy and paste and clear output serial monitor gnd waitPin"));
      while(digitalRead(waitPin)==HIGH)
      {
               delay(500);
      }
      linesPrinted=0;
    }
   }
  }
    }
 }

void SdcardSecClus::createDirorFile( char fileDirName[12],char dirOrFile[3],uint32_t fileSize)
{
if(dirOrFile=="FIL")
{
makeFileorDir(fileDirName,dirOrFile,fileSize,0,0);
}
else
{
 makeFileorDir( fileDirName, dirOrFile,0,0,0);
}
}


void SdcardSecClus::makeFileorDir(char fileDirName[12],char dirOrFile[3],uint32_t fileSize,uint32_t fileToDirSec, uint16_t fileToDirSecOffset)
{
  uint32_t mask=0xff;
   uint32_t writeSector=0;
    uint16_t writeSecOffset=0;
   byte entry32[32];
   byte fatEntry[4];
   uint32_t secReqForFile=0;
uint32_t clusReqForFile=0;
uint32_t emptyClusVal=0;
uint32_t next00FatClusSec=0;
uint16_t  fat00SecOffset=0;
uint16_t  index00FatClusSec=0;
uint16_t writeFat00Index=0;
uint32_t totalWriteFat00Clus=0;
uint32_t nextFat00ClusIndex=0;
uint16_t auxFat00SecOffset=0;
uint32_t  emptyClusLocFF=0;
uint32_t emptyFatClusLoc=0;
write1BlkIndex=0;
uint32_t emptyFatClusLocSec=0;
targetFileFoundFlag=false;
 static uint32_t newEmptyFatClusLocSec=0;

for(uint8_t k=0;k<32;k++)
{
  entry32[k]=0;
}
cleanTargetFileData();
listRootDirFiles(firstDataSec,false,fileDirName);
if(targetDirFoundFlag==true&&dirOrFile=="DIR")
{
Serial.print(fileDirName);Serial.println(F(" Directory already exsits,choose another name"));
targetDirFoundFlag=false;
return;
}
if(targetFileFoundFlag==true&&dirOrFile=="FIL")
{
  Serial.print(fileDirName);Serial.println(F(" File already exsits,choose another name"));
  targetFileFoundFlag=false;
  return;
}

//Serial.print(F("nextRtDirFreeEnt="));Serial.println(nextRtDirFreeEnt);
Serial.println(F("Searching Next Empty FAT Cluster(First Search may take a while)"));
secReqForFile=(fileSize/512)+1;//Serial.println(secReqForFile);
clusReqForFile=(secReqForFile/secPerClus)+1;//Serial.println(clusReqForFile);
emptyFatClusLoc=nextEmptyFatClus((firstFatSec+newEmptyFatClusLocSec),clusReqForFile);
emptyFatClusLocSec=(emptyFatClusLoc)/128;
newEmptyFatClusLocSec=emptyFatClusLocSec;

   if(dirOrFile=="DIR")
   {
    entry32[11]=0x10;
      for(uint8_t n=28;n<32;n++)
  {
    entry32[n]=0x00;
  }
   }
       if(dirOrFile=="FIL")
   {
    entry32[11]=0x20;
   for(uint8_t n=0;n<4;n++)
 {
  entry32[28+n]=(fileSize&mask<<n*8)>>n*8;
 // Serial.println(entry32[28+n],HEX);
 }
    }
   
  for(uint8_t n=0;n<11;n++)
  {
    entry32[n]=fileDirName[n];
  }
  if(dirOrFile=="DIR")
  {
   for(uint8_t w=8;w<11;w++)
    {
        entry32[w]=' ';
    }
  }
  entry32[21]=(emptyFatClusLoc&mask<<24)>>24;
  entry32[20]=(emptyFatClusLoc&mask<<16)>>16;
  entry32[27]=(emptyFatClusLoc&mask<<8)>>8;
  entry32[26]=emptyFatClusLoc&mask;
  
  writeSector=nextRtDirFreeEnt/512+firstDataSec;
  writeSecOffset=nextRtDirFreeEnt%512; 
  if(fileToDirSec>0)
  {
    writeSector=fileToDirSec;
    writeSecOffset=fileToDirSecOffset;
  }
write32EntSec(writeSector,writeSecOffset,entry32);

//write FAT cluster value

 next00FatClusSec=((emptyFatClusLoc*4)/512)+firstFatSec;
   fat00SecOffset=(emptyFatClusLoc*4)%512;
   
  do{

 sdRead1Blk(next00FatClusSec+index00FatClusSec,false);
writeFat00Index=0;
while(writeFat00Index+fat00SecOffset<512)
{
  if(index00FatClusSec==0)
  {
   totalWriteFat00Clus=  writeFat00Index/4;
  }
  else
  {
  totalWriteFat00Clus=(index00FatClusSec-1)*128+(writeFat00Index/4)+((512-auxFat00SecOffset)/4);
  }
 if( totalWriteFat00Clus==clusReqForFile)
 {
  Serial.println();
  Serial.println(F("Fat clusters written"));
 return;
 }
 if(totalWriteFat00Clus==clusReqForFile-1)
 {
  emptyClusVal=0xfffffff;
  emptyClusLocFF=emptyFatClusLoc+nextFat00ClusIndex+1;
 }
 else
 {
  emptyClusVal=emptyFatClusLoc+nextFat00ClusIndex+1;
  nextFat00ClusIndex++;
 }
/* for(uint8_t s=3;s>-1;s--)
 {
  fatEntry[3-s]=(emptyClusVal&mask<<(s*8))>>(s*8);
 }*/
fatEntry[0]=(emptyClusVal&mask<<24)>>24;
fatEntry[1]=(emptyClusVal&mask<<16)>>16;
fatEntry[2]=(emptyClusVal&mask<<8)>>8;
fatEntry[3]=emptyClusVal&mask;
for(uint8_t y=0;y<4;y++)
{
   sdBuf[fat00SecOffset+writeFat00Index+y]=fatEntry[3-y];
  }

  sdWrite1Blk(next00FatClusSec+index00FatClusSec);

  cleanDataSecForClusVal(emptyClusVal,emptyClusLocFF);
 sdRead1Blk(next00FatClusSec+index00FatClusSec,false);
  writeFat00Index+=4;
}
index00FatClusSec++;
auxFat00SecOffset=fat00SecOffset;
if(index00FatClusSec>0)fat00SecOffset=0;

  }while(true);
}

void SdcardSecClus::fileMakeInDir(char targetDir[12],char dirFile[12],uint32_t dirFileSize)
{
  uint32_t targetDirClus=0;
  uint32_t targetDirSec=0;
uint16_t dirIndex32=0;
uint16_t dirItr=0;
byte dirData32[32];
uint32_t mask=0xff;
 uint8_t dirVal32[32];
 uint16_t  ent32Index=0;
 uint16_t next00DirEntry=0;
 uint16_t fileToDirSecIndex=0;
 write1BlkIndex=0;
 targetFileFoundFlag=false;
 targetDirFoundFlag=false;
 cleanTargetFileData();
  listRootDirFiles(firstDataSec,false,targetDir);
   targetDirClus=targetDirData[0];
  targetDirSec=targetDirData[1];
 
   if(targetDirClus==0)
 {
  Serial.println(F("-----------------------------"));
  Serial.print(targetDir);Serial.println(F(" NOT FOUND"));
  Serial.println(F("-----------------------------"));
  return;
 }
 do{
 sdRead1Blk(targetDirSec+fileToDirSecIndex,false);
 dirItr=0;
 while(dirItr<512)
 {
  ent32Index=0;
 for(uint8_t a=0;a<32;a++)
 {
  dirVal32[a]=sdBuf[dirItr+a];
   if(dirVal32[a]==0)
   {
   ent32Index++;
   }
  }
   if(ent32Index==32)
  {
       next00DirEntry=dirItr;
    
    goto offset00Found;
  }
 dirItr+=32;
 }
 fileToDirSecIndex++;
 
 }while(true);
 
 offset00Found:
 // Serial.print(F("Next Dir 00 entry:"));Serial.println(next00DirEntry);
 if(next00DirEntry==0)
 {
 for(uint8_t g=0;g<2;g++)
 {
 for (uint8_t v=0;v<32;v++)
 {
  dirData32[v]=0;
 }
 if(dirIndex32==0)
 {
 dirData32[0]=0x2e;
  dirData32[21]=(targetDirClus&mask<<24)>>24;
  dirData32[20]=(targetDirClus&mask<<16)>>16;
  dirData32[27]=(targetDirClus&mask<<8)>>8;
 dirData32[26]=targetDirClus&mask;
 }
 if(dirIndex32==32)
 {
  dirData32[0]=0x2e;
  dirData32[1]=0x2e;
  }
  
write32EntSec( targetDirSec+fileToDirSecIndex, dirIndex32, dirData32);
dirIndex32+=32;
 }
 }
 if(next00DirEntry==0)
 {
  next00DirEntry=next00DirEntry+64;
 }
  makeFileorDir( dirFile,"FIL", dirFileSize, targetDirSec,next00DirEntry);
}

void SdcardSecClus::writeToFile(char writeFileName[12],char dataWrite[],uint16_t fileWriteSize,boolean programMemOrData)
{
 static uint32_t writeFile1stSec=0;
  uint16_t writeFileItrIndex=0;
   uint8_t nextWrite00Line=0;
uint8_t line00Index=0;
uint16_t writeFileOffset=0;
uint8_t progmemRead=0;
 uint16_t writeDataIndex=0;
  uint16_t writeDataSecIndex=0;
write1BlkIndex=0;
static uint32_t writeFile00SecIndex=0;
uint8_t nameIndex=0;
static uint32_t targetFileSize=0;
static boolean fileFullFlag=false;
  
   for(uint8_t n=0;n<12;n++)
   {
    if(lastWriteFileName[n]==writeFileName[n])
    {
      nameIndex++;
      }
   }
   if(nameIndex!=12)
   {

   
writeFile00SecIndex=0;
fileFullFlag=false;
    for(uint8_t c=0;c<12;c++)
    {
    lastWriteFileName[c]=writeFileName[c];
    }
   cleanTargetFileData();
   listRootDirFiles(firstDataSec,false,writeFileName);
 
   if(targetFileFoundFlag!=true)
   {
    Serial.println(F("-----------------------------"));
    Serial.println(F("Write File Not Found"));
    Serial.println(F("-----------------------------"));
    return;
   }
 
      targetFileSize=targetFileData[4];
   Serial.print(F("targetFileSize:"));Serial.println(targetFileSize);
   writeFile1stSec=targetFileData[1];
   }
   
   do{
    sdRead1Blk((writeFile1stSec+writeFile00SecIndex),false);
    writeFileItrIndex=0;
    while(writeFileItrIndex<512)
    {
       nextWrite00Line=sdBuf[writeFileItrIndex];
 
      if(nextWrite00Line==0)//was 4
      {
        writeFileOffset=writeFileItrIndex;
             goto write00Found;
      }
      writeFileItrIndex++;//was+=4
     }
      writeFile00SecIndex++;
       }while(true);
   
   write00Found:
      
    if(((writeFile00SecIndex*512)+fileWriteSize+ writeFileItrIndex)>targetFileSize)
    {
     writeFile00SecIndex=0;
      Serial.println();
Serial.print(writeFileName); Serial.println(F(" File Full"));
 fileFullFlag=true;
 return fileFullFlag;
    }
 
   for(uint16_t s=0;s<fileWriteSize;s++)
   {
     if(programMemOrData==false)
  {
     progmemRead=pgm_read_byte(&dataWrite[s]);
  }
  else
  {
    progmemRead=dataWrite[s];
  }
  
   sdBuf[writeDataIndex+writeFileOffset]=progmemRead;
   writeDataIndex++;
  
   if(writeDataIndex+writeFileOffset>511||(s+1==fileWriteSize))
   {
      if(s+1==fileWriteSize)
    {
           for(uint16_t r=writeDataIndex+writeFileOffset;r<512;r++)
      {
       sdBuf[r]=0;
      }
    }
    
        sdWrite1Blk(writeFile1stSec+writeFile00SecIndex+writeDataSecIndex);
        writeDataSecIndex++;
       writeDataIndex=0;
  writeFileOffset=0;
   }
   }
   Serial.print(F("File Data Writen To:"));Serial.println(writeFileName);
Serial.println();

}

void SdcardSecClus::writeNumToFile(char writeFile[12],uint32_t numData)
{
  uint16_t strSize=0;
 String strNumData=String(numData);
strSize=strNumData.length()+1;
 char dataCharArray[strSize];
strNumData.toCharArray(dataCharArray,strSize+1);
  writeToFile(writeFile,dataCharArray,strSize,true);
}

void SdcardSecClus::writeStringToFile(char writeFile[12],String myString)
{
  uint16_t stringSize=0;
   stringSize=myString.length()+1;
char charArray[stringSize];
myString.toCharArray(charArray,stringSize);
  writeToFile(writeFile,charArray,stringSize,true);
 }

void SdcardSecClus::writeProgMemToFile(char writeFile[12],char progMemFile[],uint16_t progMemFileSize)
{
  writeToFile(writeFile,progMemFile,progMemFileSize,false);
}