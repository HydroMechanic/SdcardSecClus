# SdcardSecClus
Read Write List Files Sd Card Sector Cluster information
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
  
LIBRARY USE FOR ARDUINO IDE.
 USE WITH SDCARD FORMATED WITH FAT32 ONLY,SKETCH WILL ONLY READ SEQUENTIAL FAT ENTRIES
 (NOT FRAGMENTED FAT ENTRIES).SKETCH WRITES ONLY SEQUENTIAL FAT ENTRIES.YOU CAN READ
 CHARACTER OR HEX DATA FROM SECTORS(TRUE OR FALSE).READS AND LISTS FILE FROM ONLY 2 DIRECTORY LEVELS.
 IF YOU DELETE DATA  FROM FILE IN WINDOWS,FILE WILL BE CORRUPTED(WINDOWS WRITES 0X20
  TO THE ENTIRE REMAINING FILE???)YOU MUST DELETE THE FILE AND LET THE SKETCH CREATE IT AGAIN(ALL 
  DATA LOST).

User Functions:

*SdcardSecClus()*
Constructor

*void sdBegin(uint16_t baudRate,uint8_t waitPin,uint8_t chipSelectPin)*
Initializes sdCard formated in FAT32,recommended baudrate:57600. Wait pin variable for pausing read function, ChipSeclectPin for SPI is normally D10 on uno nano.


*void rootDirFilesList()*
Will list directories and files created by windows or sketch with sector cluster information. Only 2 levels of directory readable.


*void readFile(char searchFileName[12],boolean hexOrChar)*
File name follows 8.3 convention 8 charactor name,3 charactor extention(no '.')file name must be 8 UPPERCASE charactors spaces ok
(eg. "MYFILE  TXT"). Read file output can be charactors or hexidecimal output(char output: true; hex output: false ) it seems
large file charactor output cannot be copied from Serial monitor,use hexdecimal output, copy serial monitor contents then use hex to ascii convertor like notepad++.Will only read  FAT sequential entries(non fragmented),error message should be displayed if non -sequential FAT entry detected.

*void createDirorFile( char fileDirName[12],char dirOrFile[3],uint32_t fileSize)*
Directory name must be 8 UPPERCASE charactors spaces ok(eg "MYDIR   ") . You can create a file in the root directory follow 
8.3 convention(see fileMakeInDir function below).Specify directory("DIR" or "FIL"). Last field used only if file is created (in bytes)otherwise fileSize will be 0 for directory.



*void fileMakeInDir(char targetDir[12],char dirFile[12],uint32_t dirFileSize)*
Specify directory name(previously created) 8 UPPPERCASE charactor directory name where new file will be created.
Specify File name follows 8.3 convention 8 UPPERCASE charactor name,3 UPPERCASE charactor extention(no '.') charactors spaces ok (eg. "MYFILE  TXT").  Directory file size is in bytes.


*void writeNumToFile(char writeFile[12],uint32_t numData);*
Specify 8.3 convention UPPERCASE file name and extention to write to. Specify integer number data.Message displayed when
file is full.

*void writeStringToFile(char writeFile[12],String myString)*
Specify 8.3 convention UPPERCASE file name and extention to write to.Specify string .Message displayed when
file is full.

*void writeProgMemToFile(char writeFile[12],char progMemFile[],uint16_t progMemFileSize)*
Writes Data from program memory(Flash) to a specifed file.Specify 8.3 convention UPPERCASE file name and extention to write to.
Use sizeof utility to determine file size.Message displayed when file is full.

