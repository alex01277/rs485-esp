
void preTransmission()  {  digitalWrite(MAX485_RE_NEG, HIGH);} //Switch to transmit data
void postTransmission() {  digitalWrite(MAX485_RE_NEG, LOW);} //Switch to receive data}

void displayPaddedHexByte(byte value, bool showPrefix = true)
{ if (showPrefix) Serial.print("0x");
  if (value <= 0x0F) Serial.print("0");
  Serial.print(value, HEX);
}
void clearEEPROM() 
{ for (int i = 0; i < 13; i++) 
  {EEPROM.write(i, 0xFF);}
  EEPROM.commit();  }

void displayEEPROM()
{ for (int i = 0; i < 13; i++)
  { displayPaddedHexByte(EEPROM.read(i), true);
    Serial.print(" ");    //  if ((i + 1) % 32 == 0)  
    }
  Serial.println();
}
void printHeader(String description)
{ Serial.println("***");
  Serial.print  ("*** "); Serial.println(description);
  Serial.println("***");
}
void GetCRCm( byte *ModData,byte lln) 
   { unsigned short value = crc.Modbus(ModData,0,lln-2);  
     ModData[lln-2]=value>>8;ModData[lln-1]=value;}
     
void getDataM(byte NumParam, byte CountParam, byte *ModData, uint8_t lenM,uint8_t _ids)  { 
    uint8_t  serialBuffer[254];
    bool flag=0;  
    ModData[0] = _ids;
    GetCRCm(ModData,lenM);  
    if (dbg) {Serial.println();Serial.print(lenM);Serial.print("z");
       for (size_t im = 0; im < lenM; im++)
         {Serial.print(":"); Serial.print(ModData[im],HEX);}    }
    int incomingCharPos = 0;
    preTransmission(); 
    RS485.write(ModData,lenM); 
    postTransmission(); 
    delay(120);
    while (RS485.available() > 0) {
          byte response = RS485.read();
          if (response == _ids) flag=1;
          if (dbg) {Serial.print("|"); Serial.print(response, HEX);}
          serialBuffer[incomingCharPos] = response;
          if (flag) incomingCharPos++;     
          }
     if (dbg) {Serial.print(";R-");Serial.print(incomingCharPos);Serial.print("#");} 
     if (CountParam) {  //don't do if zero
        for (int jj=0;jj<CountParam;jj++){   //data processing 
          mb.Hreg(jj+NumParam,serialBuffer[5+jj*2]<<8 | serialBuffer[4+jj*2]);          
          if (dbg){Serial.print(jj+NumParam); 
                   Serial.print("-"); 
                   Serial.print(mb.Hreg(jj+NumParam), HEX); 
                   Serial.print(":");}
         }
     //  if (dbg)  Serial.println();
      }
     incomingCharPos=0;
 }
 void getDataD(byte NumParam, byte CountParam, byte *ModData, uint8_t lenM,uint8_t _ids)  { 
    uint8_t  serialBuffer[120]; 
    bool flag=0;  
    ModData[0] = _ids;
    GetCRCm(ModData,lenM);  
    if (dbg) {Serial.println();Serial.print(lenM);Serial.print("z");
       for (size_t im = 0; im < lenM-2; im++)
         {Serial.print(":"); Serial.print(ModData[im],HEX);}    }
    int incomingCharPos = 0;
   preTransmission(); 
    RS485.write(ModData,lenM); 
    postTransmission(); 
    delay(120);
    while (RS485.available() > 0) {
       byte response = RS485.read();
          if (response == _ids) flag=1;
          if (dbg) {Serial.print("|"); Serial.print(response, HEX);}
          serialBuffer[incomingCharPos] = response;
          if (flag) incomingCharPos++;   
       }
      if (dbg) {Serial.print(";R-");Serial.print(incomingCharPos);Serial.print("#");}        
     mb.Hreg(NumParam+0, serialBuffer[9]); // y
     mb.Hreg(NumParam+1, serialBuffer[8]); // month
     mb.Hreg(NumParam+2, serialBuffer[7]); // day 
     mb.Hreg(NumParam+3, serialBuffer[6]); // hr 
     if (dbg){
        for (int jj1=0;jj1<CountParam;jj1++){
             Serial.print(NumParam+jj1);    Serial.print("-"); 
             Serial.print(mb.Hreg(jj1+NumParam), HEX);   Serial.print(":");} 
                  }         
  } 
void getDataM7(byte NumParam, byte CountParam, byte *ModData, uint8_t lenM,uint8_t _ids)  { 
    uint8_t  serialBuffer[120]; 
    bool flag=0;   
    ModData[0] = _ids;
    GetCRCm(ModData,lenM);  
    if (dbg) {Serial.println();Serial.print(lenM);Serial.print("z");
       for (size_t im = 0; im < lenM-2; im++)
         {Serial.print(":"); Serial.print(ModData[im],HEX);}    }
    int incomingCharPos = 0;
    preTransmission();
    RS485.write(ModData,lenM); 
    postTransmission(); 
    delay(120);
    while (RS485.available() > 0) {
       byte response = RS485.read();
          if (response == _ids) flag=1;
          if (dbg) {Serial.print("|"); Serial.print(response, HEX);}
          serialBuffer[incomingCharPos] = response;
          if (flag) incomingCharPos++;   
       }
      if (dbg) {Serial.print(";R-");Serial.print(incomingCharPos);Serial.print("#");} 
      if (incomingCharPos==22){
         mb.Hreg(NumParam+1, serialBuffer[13]<<8 | serialBuffer[12]); 
         mb.Hreg(NumParam  , serialBuffer[11]<<8 | serialBuffer[10]); 
         mb.Hreg(NumParam+2, serialBuffer[9]); //y
         mb.Hreg(NumParam+3, serialBuffer[8]); // month
         mb.Hreg(NumParam+4, serialBuffer[7]); // day 
         mb.Hreg(NumParam+5, serialBuffer[6]); // hr 
         mb.Hreg(NumParam+6, serialBuffer[5]); // min  
        if (dbg){
            for (int jj1=0;jj1<CountParam;jj1++){
                Serial.print(NumParam+jj1);    Serial.print("-"); 
                Serial.print(mb.Hreg(jj1+NumParam), HEX);   Serial.print(":");} 
                }
         }         
  }
 
 void GetDataCC301(uint8_t ids)
{     getDataM(0 ,34, ModData46,sizeof(ModData46),ids);  //all       
   //   getDataM( 0, 8, ModData8, sizeof(ModData8) ,ids ); // p a 4        
   //   getDataM( 8, 8, ModData9, sizeof(ModData9) ,ids ); // p r 4        
   //   getDataM(16, 6, ModData10,sizeof(ModData10),ids ); // u   3        
   //   getDataM(22, 6, ModData11,sizeof(ModData11),ids ); // i   3         
   //   getDataM(28, 6, ModData12,sizeof(ModData12),ids ); // kp  3      
      getDataD(46, 4, ModData32,sizeof(ModData32),ids ); // date time
      getDataM(34, 8, ModData1, sizeof(ModData1) ,ids ); //e 
  //    getDataM(42, 2, ModData25, sizeof(ModData25) ,ids ); //Ki       
  //    getDataM(44, 2, ModData26, sizeof(ModData26) ,ids ); //Ku             
    if (b_pday)  {getDataM(60,  8, ModData2, sizeof(ModData2) ,ids ); b_pday = false; }//d m 
    if (b_pmonth){getDataM(68,  8, ModData3, sizeof(ModData3) ,ids ); b_pmonth= false;} //m m 
    if (b_pyear) {getDataM(76,  8, ModData4, sizeof(ModData4) ,ids ); b_pyear= false; } //� m 
    if (b_pmax)  {getDataM7(94, 7, ModData71,sizeof(ModData71),ids ); //pmax pa+
                  getDataM7(101,7, ModData72,sizeof(ModData72),ids ); //pmax pa-
                  getDataM7(108,7, ModData73,sizeof(ModData73),ids ); //pmax pr+     
                  getDataM7(115,7, ModData74,sizeof(ModData74),ids ); //pmax pr- 
                  b_pmax=false;}
    //  getDataM(84, 2, ModData13,sizeof(ModData13),ids ); //f???? 
 } 
