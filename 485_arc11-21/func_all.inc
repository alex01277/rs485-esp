/*   dataQ[3] = dataQLoad(ids, 3, 11,0,0,0); //i
  // dataQ[4] = dataQLoad(ids, 3, 12,0,0,0); //k 
  // dataQ[5] = dataQLoad(ids, 3, 46,0,0,0); //all
  // dataQ[6] = dataQLoad(ids, 4, 1,0,0,0); //k 
  // dataQ[11]= dataQLoad(ids, 3, 25,0,0,0); //Ki 
  // dataQ[12]= dataQLoad(ids, 3, 50,0,0,0); //Ki   
  // dataQ[13]= dataQLoad(ids, 3, 26,0,0,0); //Ki 
  // dataQ[14]= dataQLoad(ids, 3, 34,0,0,0); //Ki   
  // dataQ[15]= dataQLoad(ids, 3, 52,0,0,0); //Ki   
byte  ModData1[]={39,16,34,0,1, 0,0,0,1,0, 0,0,1,3,0, 1,3,0,0,3, 0,3,0,0};  //  base  24
byte  ModData2[]={39,16,34,0,0,0,2,0,1,0,0,0,1,3,0,1,3,0,0,3,0,3,0,0};  //  base mod   24
byte  ModData3[]={39,3,34,0,0,0,0,0}; //u
byte  ModData4[]={ids,31,0,0, 0x30,0x30,0x30,0x30, 0x30,0x30,0x30,0x30, 0,0};  //p  14
byte  ModData5[]={ids,32,0,0,0,0};  //pm   14
byte  ModData6[]={ids,37,0,0,0x30,0x30,0x30,0x30, 0x30,0x30,0x30,0x31, 0,0};  //pm   14
uint16_t GetCrc16Simple( uint8_t * data, uint8_t len )
{   uint8_t lo;
    uint16_t rdata;
    union
    {   uint16_t value;
        struct { uint8_t lo, hi; } bytes;
    } crc;
    crc.value = 0xFFFF;
    while ( len-- )   {
        lo = crc.bytes.lo;
        crc.bytes.lo = crc.bytes.hi;
        crc.bytes.hi = lo ^ *data++;
        uint8_t mask = 1;
        if ( crc.bytes.hi & mask ) crc.value ^= 0x0240;
        if ( crc.bytes.hi & ( mask << 1 ) ) crc.value ^= 0x0480;
        if ( crc.bytes.hi & ( mask << 2 ) ) crc.bytes.hi ^= 0x09;
        if ( crc.bytes.hi & ( mask << 3 ) ) crc.bytes.hi ^= 0x12;
        if ( crc.bytes.hi & ( mask << 4 ) ) crc.bytes.hi ^= 0x24;
        if ( crc.bytes.hi & ( mask << 5 ) ) crc.bytes.hi ^= 0x48;
        if ( crc.bytes.hi & ( mask << 6 ) ) crc.bytes.hi ^= 0x90;
        if ( crc.bytes.hi & ( mask << 7 ) ) crc.value ^= 0x2001; }
    rdata = crc.value;
    crc.value = (crc.value << 8) | (rdata >> 8 & 0xFF);
    return crc.value;
}

MyDataQ dataQLoad(byte adr, byte n_func, byte codP, byte d01, byte d02, byte d03)
{ MyDataQ dataQL;
  dataQL.adr    = adr;
  dataQL.n_func = n_func;
  dataQL.codP   = codP;
  dataQL.d01 = d01;
  dataQL.d02 = d02;
  dataQL.d03 = d03;
  dataQL.crc = GetCrc16Simple( (byte*)&dataQL, sizeof(dataQL) - 2 );
  return dataQL;
}
void SendStruct(const MyDataQ& toSend, Stream* s, byte len)
{ byte* b = (byte*) &toSend;
  for (size_t i = 0; i < len; i++)  s->write(*b++); 
}

//void SendStructP(const ModData2[]& toSend, Stream* s, byte len)
//{ byte* b = (byte*) &toSend;  for (size_t i = 0; i < len; i++)  s->write(*b++); }
void PrintStruct(const MyDataQ& dataQ)
{ Serial.print(" 0x"); Serial.print(dataQ.adr, HEX);
  Serial.print(" 0x"); Serial.print(dataQ.n_func, HEX);
  Serial.print(" 0x"); Serial.print(dataQ.codP, HEX);
  Serial.print(" 0x"); Serial.print(dataQ.d01, HEX);
  Serial.print(" 0x"); Serial.print(dataQ.d02, HEX);
  Serial.print(" 0x"); Serial.print(dataQ.d03, HEX);
  Serial.print(" 0x"); Serial.println(dataQ.crc, HEX);
}
void getData(byte NumParam,byte CountParam, MyDataQ dataQL) 
   {  int incomingCharPos = 0;
     if (DIR) digitalWrite(DIR, HIGH);
    //  RS485.write(dataQL,8); 
     SendStruct(dataQL, &RS485, 8);  if (dbg)  PrintStruct(dataQL);   //reqest
     if (DIR) digitalWrite(DIR, LOW);
     delay(50);
     while (RS485.available() > 0) {
       byte response = RS485.read();
       if (dbg) {Serial.print("|");  Serial.print(response, HEX);}
       serialBuffer[incomingCharPos] = response;
       incomingCharPos++;  }
      int inn= incomingCharPos/2;
      if (dbg) {Serial.print(":inc#:");Serial.println(incomingCharPos); }
   // if (dbg) {Serial.print("in:");  Serial.print(incomingCharPos); Serial.println(inn);}
       for (int jj=0;jj<CountParam;jj++){   //data processing 
          buffer11[0] = serialBuffer[4+jj*2 ]; 
          buffer11[1] = serialBuffer[5+jj*2 ];    
          au16data[jj+NumParam] = (uint16_t&)buffer11;
        //   if (dbg){Serial.print(jj+NumParam); Serial.print("-"); Serial.print(au16data[jj+NumParam], HEX); Serial.print(":");}
          }
        if (dbg)  Serial.println();
     incomingCharPos=0;
 }  
    union
    {   uint16_t value;
        struct { uint8_t lo, hi; } bytes;
    } crc_16;
     //   au16data[38]=slave.getInCnt();
      //   au16data[39]=slave.getOutCnt();
      //   au16data[40]=slave.getErrCnt(); 
 */
 