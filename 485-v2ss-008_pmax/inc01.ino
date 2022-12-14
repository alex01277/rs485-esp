  void GetCRCm( byte *ModData,byte lln) 
   { unsigned short value = crc.Modbus(ModData,0,lln-2);  
     ModData[lln-2]=value>>8;ModData[lln-1]=value;}

  void getDataM(byte NumParam, byte CountParam, byte *ModData, uint8_t lenM,uint8_t _ids)  { 
    uint8_t  serialBuffer[120];  
    ModData[0] = _ids;
    GetCRCm(ModData,lenM);  
    if (dbg) {Serial.print(lenM);Serial.print("z");
       for (size_t im = 0; im < lenM-2; im++)
         {Serial.print(":"); Serial.print(ModData[im],HEX);}    }
    int incomingCharPos = 0;
    if (DIR) digitalWrite(DIR, HIGH);
    RS485.write(ModData,lenM); 
    if (DIR) digitalWrite(DIR, LOW);
    delay(100);
    while (RS485.available() > 0) {
       byte response = RS485.read();
     if (dbg) {Serial.print("|"); Serial.print(response, HEX);}
       serialBuffer[incomingCharPos] = response;
       incomingCharPos++;  
       }
      if (dbg) {Serial.println();Serial.print(";R-");Serial.println(incomingCharPos);} 
      int inn= incomingCharPos/2;
      if ( ModData[2] = 7 ){   //pmax for month imm=ModData[5]*7;
           au16data[NumParam]   = serialBuffer[13]<<8 | serialBuffer[12]; 
           au16data[NumParam+1] = serialBuffer[11]<<8 | serialBuffer[10]; 
           au16data[NumParam+2] = serialBuffer[9]; //y
           au16data[NumParam+3] = serialBuffer[8]; // month
           au16data[NumParam+4] = serialBuffer[7]; // day 
           au16data[NumParam+5] = serialBuffer[6]; // hr 
           au16data[NumParam+6] = serialBuffer[5]; // min  

          if (dbg){
              for (int jj1=0;jj1<CountParam;jj1++){
                   Serial.print(NumParam+jj1); 
                   Serial.print("-"); 
                   Serial.print(au16data[NumParam+jj1], HEX); 
                   Serial.print(":");} 
                  }         
          }
      else {  //4 or 3 float value
      if (CountParam) {  //don't do if zero
        for (int jj=0;jj<CountParam;jj++){   //data processing 
          au16data[jj+NumParam] = serialBuffer[5+jj*2]<<8 | serialBuffer[4+jj*2];          
          if (dbg){Serial.print(jj+NumParam); 
                   Serial.print("-"); 
                   Serial.print(au16data[jj+NumParam], HEX); 
                   Serial.print(":");}
         }
       if (dbg)  Serial.println();
      }
     } 
   incomingCharPos=0;
 }
void GetDataCC301(uint16_t interval,uint8_t ids)
{   //getDataM(0 ,34, ModData46,sizeof(ModData46),ids); //all       
  /*    getDataM( 0 ,8, ModData8, sizeof(ModData8),ids ); // p a 4        
      getDataM( 8 ,8, ModData9, sizeof(ModData9),ids ); // p r 4        
      getDataM( 16,6, ModData10,sizeof(ModData10),ids ); // u   3        
      getDataM( 22,6, ModData11,sizeof(ModData11),ids ); // i   3         
      getDataM( 28,6, ModData12,sizeof(ModData12),ids ); // kp  3         
      getDataM(34, 8, ModData1, sizeof(ModData1),ids ); //e
      getDataM(60, 8, ModData2, sizeof(ModData2),ids ); //d m 
      getDataM(68, 8, ModData3, sizeof(ModData3),ids ); //m m 
      getDataM(76, 8, ModData4, sizeof(ModData4),ids ); //?? m */
      getDataM( 94, 7, ModData71, sizeof(ModData71),ids ); //pmax pa+
      getDataM(101, 7, ModData72, sizeof(ModData72),ids ); //pmax pa-
      getDataM(108, 7, ModData73, sizeof(ModData73),ids ); //pmax pr+     
      getDataM(115, 7, ModData74, sizeof(ModData74),ids ); //pmax pr-     
   //   getDataM(86, 2, ModData13,sizeof(ModData13),ids ); //f      
  /*  if (mmm) {
      getDataM(0,  0, ModData25,sizeof(ModData25),ids);   // ki 
      getDataM(0,  0, ModData131,sizeof(ModData131),ids); // enable  change
      getDataM(0,  0, ModData125,sizeof(ModData125),ids);   // ki 
      getDataM(0,  0, ModData118,sizeof(ModData118),ids); // 118   f
      getDataM(0,  0, ModData52,sizeof(ModData52),ids);   // ki ku  f    
      getDataM(0,  0, ModData121,sizeof(ModData121),ids); // 121   
      getDataM(0,  0, ModData131,sizeof(ModData131),ids); // enable  change
      getDataM(0,  0, ModData117,sizeof(ModData117),ids); //   change ki ku
      delay(500);          
      getDataM(0,  0, ModData34,sizeof(ModData34),ids);   // ki ku
       mmm=0;}  
      getDataM(0,  0, ModData132,sizeof(ModData132),ids);   // disable change      
      getDataM(0,  0, ModData137,sizeof(ModData137),ids);   // pass mod      */
 } 

 //=========================
 void displayPaddedHexByte(byte value, bool showPrefix = true)
{ if (showPrefix) Serial.print("0x");
  if (value <= 0x0F) Serial.print("0");
  Serial.print(value, HEX);
}
void clearEEPROM() { for (int i = 0; i < EEPROM.length(); i++) EEPROM.update(i, 0xFF);}
void displayEEPROM()
{ for (int i = 0; i < EEPROM.length(); i++)
  { displayPaddedHexByte(EEPROM[i], false);
    Serial.print(" ");
    if ((i + 1) % 32 == 0)  Serial.println();
  }
}
void printHeader(String description)
{ Serial.println("***");
  Serial.print("*** "); Serial.println(description);
  Serial.println("***");
}
  
