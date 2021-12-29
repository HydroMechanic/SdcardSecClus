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

   WARNING!:DONOT USE THIS SKETCH WITH SENSITIVE DATA. DATA MAY BE LOST,CORRUPTED,INCOMPLETE.MAY DAMAGE
  YOUR SDCARD. USE AT OWN RISK.
  
 USE WITH SDCARD FORMATED WITH FAT32 ONLY,SKETCH WILL ONLY READ SEQUENTIAL FAT ENTRIES
 (NOT FRAGMENTED FAT ENTRIES).SKETCH WRITES ONLY SEQUENTIAL FAT ENTRIES.YOU CAN READ
 CHARACTER OR HEX DATA FROM SECTORS(TRUE OR FALSE).READS AND LISTS FILE FROM ONLY 2 DIRECTORY LEVELS.
 IF YOU DELETE DATA  FROM FILE IN WINDOWS,FILE WILL BE CORRUPTED(WINDOWS WRITES 0X20
  TO THE ENTIRE REMAINING FILE???)YOU MUST DELETE THE FILE AND LET THE SKETCH CREATE IT AGAIN(ALL 
  DATA LOST).
 
*/

#include <SdcardSecClus.h>
SdcardSecClus mySd;
const char ardu[] PROGMEM={"const int dataPin=2; const int loadPin=3; const int clockPin=4; const int shutDnRg=0xc; const int decodeModeRg=0x9; const int intensityRg=0xa; const int scanLimitRg=0xb; byte digitArray[]={0,0,0,0,0,0,0,0}; byte newArray[8]; long  iniVal=123456; int stlgth=8;  void setup() {   Serial.begin(9600);   pinMode(dataPin,OUTPUT);   pinMode(loadPin,OUTPUT);   pinMode(clockPin,OUTPUT);   digitalWrite(loadPin,HIGH);   startMax();    for(int y=0;y<8;y++)   {     digitArray[y]=' ';   } digitOut(digitArray); byte digitArray[]={0,0,0,0,0,0,0,0}; digitOut(digitArray); delay(2000);  } void loop() {   while((iniVal--)>0)   {    //Serial.println(iniVal); String  stringIniVal=String(iniVal);     stlgth=stringIniVal.length();    //Serial.println(stlgth);     for(int x=0;x<stlgth;x++)     {     char num=stringIniVal.charAt(x);    digitArray[x]=num;      }     digitOut(digitArray);     //delay(100);     }   }  void writeData(byte msb,byte lsb) {   digitalWrite(loadPin,LOW);   shiftOut(dataPin,clockPin,MSBFIRST,(msb));   shiftOut(dataPin,clockPin,MSBFIRST,(lsb));   digitalWrite(loadPin,HIGH); } void startMax() {     writeData(shutDnRg,0x1);   writeData(decodeModeRg,0xff);   writeData(scanLimitRg,0x7);   writeData(intensityRg,0xa); }  void digitOut(byte *arrayDigit) {  // Serial.println(stlgth);      for(byte x=0;x<stlgth;x++)   {          writeData(stlgth-x,arrayDigit[x]);      if(stlgth<8)     {   writeData(stlgth+1,B0);     }   } } "};
const char myMess[] PROGMEM={"Hello joe how are you today are you feeling OK.I think I woke up too early today and are kind of feeling tired. Sounds like it is going to rain during pumpkinfest.Fire works delayed to saturday to bad!!"};
uint8_t waitPin=2;
uint8_t loopIndex=0;
uint8_t csPin=10;
long timeStamp=0;

void setup()
{
 mySd.sdBegin(57600,waitPin,csPin);
mySd.rootDirFilesList();
mySd.createDirorFile("JOEDIR1 ","DIR",0);//DIRECTORIES HAVE NO SIZE
mySd.createDirorFile("ZELLODIR","DIR",0);
mySd.createDirorFile("DIRFILE TXT","FIL",25000);//YOU CAN CREATE FILE IN ROOT DIR,FILE SIZE REQUIRED
mySd.createDirorFile("YELLODIR","DIR",0);
mySd.fileMakeInDir("ZELLODIR","AFILE   TXT",10000);
mySd.fileMakeInDir("ZELLODIR","BFILE   TXT",10000);
mySd.fileMakeInDir("YELLODIR","CFILE   TXT",60000);
mySd.fileMakeInDir("JOEDIR1 ","DFILE   TXT",100000);
  mySd.writeProgMemToFile("CFILE   TXT" ,ardu,sizeof(ardu)-1 );
mySd.writeProgMemToFile("DFILE   TXT" ,myMess,sizeof(myMess)-1 );
mySd.readFile("AFILE   TXT",true);//char output: true; hex output: false
mySd.readFile("BFILE   TXT",true);//char output: true; hex output: false
mySd.readFile("CFILE   TXT",true);//char output: true; hex output: false
mySd.readFile("DFILE   TXT",true);//char output: true; hex output: false

}

void loop()
{
 
  if(millis()-timeStamp>200)
  {

    timeStamp=millis();
 mySd.writeNumToFile("DIRFILE TXT",timeStamp);
  mySd.writeStringToFile("DIRFILE TXT",",");
  mySd.writeNumToFile("AFILE   TXT",timeStamp);
   mySd.writeStringToFile("AFILE   TXT",",");
 mySd.writeStringToFile("AFILE   TXT","hello joe,");
     if(loopIndex==20)
  {
while(digitalRead(waitPin)==1)
{
  delay(10);
}
loopIndex=0;

  }
 
     loopIndex++;
  }
}
