#include <ModbusRtu.h>
#include <avr/eeprom.h> 
#include <EEPROM-Storage.h>
#include <Crc16.h>
#include <SoftwareSerial.h>
#define DIR 8 // пин управления прием/передача
  Crc16 crc; 
  SoftwareSerial RS485(7,6);// RX, TX
  uint16_t au16data[127];
  uint8_t ids  = 199;
  uint8_t madr = 199;
  uint8_t p_ids,p_madr;
  Modbus slave(madr,Serial,3); // this is slave @1 and RS-232 or USB-FTDI  
  uint8_t // reqests
    ModData1[] ={ids, 4,  1,0,0,0,0,0},  //p
    ModData2[] ={ids, 4,  2,0,0,0,0,0},  //de day
    ModData3[] ={ids, 4,  3,0,0,0,0,0},  //de month 
    ModData4[] ={ids, 4,  4,0,0,0,0,0},  //de y
    ModData71[] ={ids, 4,  7,0,0,1,0,0},  //p max+
    ModData72[] ={ids, 4,  7,0,0,2,0,0},  //p max-    
    ModData73[] ={ids, 4,  7,0,0,3,0,0},  //q max+    
    ModData74[] ={ids, 4,  7,0,0,4,0,0},  //q max-        
    ModData8[] ={ids, 3,  8,0,0,0,0,0},  //p a
    ModData9[] ={ids, 3,  9,0,0,0,0,0},  //p r 
    ModData10[]={ids, 3, 10,0,0,0,0,0},  //u 
    ModData11[]={ids, 3, 11,0,0,0,0,0},  //i 
    ModData12[]={ids, 3, 12,0,0,0,0,0},  //k p
    ModData13[]={ids, 3, 13,0,0,0,0,0},  //f 
    ModData21[]={ids, 3, 21,0,0,0,0,0},  // slave ID     
    ModData25[]={ids, 3, 25,0,0,0,0,0},  //Ki int 
    ModData26[]={ids, 3, 26,0,0,0,0,0},  //Ku int 
    ModData34[]={ids, 3, 34,0,0,0,0,0},  //Ki Ku int disp
    ModData46[]={ids, 3, 46,0,0,0,0,0},  // all 
    ModData50[]={ids, 3, 50,0,0,0,0,0},  //Ki fl
    ModData51[]={ids, 3, 51,0,0,0,0,0},  //Ku fl
    ModData52[]={ids, 3, 52,0,0,0,0,0},  //Ki Ku fl  disp
    ModData116[]={ids,16,34,0,2,1,1,1,2,1,1,1,1,3,0,1,3,0,0,3,0,3,0,0}, //  base mod   24        
    ModData117[]={ids,16,34,0,2,1,1,1,2,1,1,1,0,0},                     //  base mod   24 1  
    ModData118[]={ids,16,34,0,0,0,0xB0,0x40,0,0,0xB0,0x40,1,3,0,1,3,0,0,3,0,3,0,0},       //  base mod   24           
    ModData121[]={ids,16,21,0,39,0,0},       //  base mod   24    
    ModData125[]={ids,16,34,0,0,0,0,1,0,0,2,0,0,0,1,3,0,01,3,0,0,3,0,3,0,0},       //  base mod   24   
    ModData131[]={ids,31,0,0, 0x30,0x30,0x30,0x30, 0x30,0x30,0x30,0x30, 0,0}, //  enable ch  14
    ModData132[]={ids,32,0,0,0,0},                                            //  disable change   14
    ModData137[]={ids,37,0,0,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x31,0,0};    //  pass mod   14
//////////////////////////////////////////////////////////////////////////////////  //byte     TypeRead =1;    //1 - seqentall; 0-all zusammen
   int      pday=0,  pmonth=0, pyear=0,  pmax = 0; //max
   int      dbg      = 1;         //debug 
   uint16_t Interval = 5000;     //10s
   uint16_t spedss   = 19200;     // serial port speed
   uint32_t Previous = 0;
      
 void setup() {
  Serial.begin(spedss); // baud-rate at 19200
  // if (dbg) { // clearEEPROM();
   //  printHeader("EEPROM Contents (" + String(EEPROM.length()) + " bytes)");
   //  displayEEPROM(); Serial.println(); Serial.println();}
//  EEPROM READ INITAL Value if first time run write default value to EEPROM
    p_ids = eeprom_read_byte(10); //read Id cc301
    if (p_ids==255) 
     { eeprom_update_byte(10,199);
       p_ids=199;ids=p_ids;} //default value 199  eeprom clear
    p_madr= eeprom_read_byte(12); //read Slave ID   
    if (p_madr==255)
     { eeprom_update_byte(12,199);
       p_madr=199;madr=p_madr;} //default value 199  eeprom clear
   slave.setID(p_madr);  // set new SlaveID
  
   RS485.begin(9600);
   slave.start();
   if (dbg) { 
      Serial.print("Begin...Slave ID-");Serial.print(slave.getID()); 
      Serial.print(" ids-");Serial.println(p_ids);
      //Serial.println(p_madr); Serial.println(madr);    
      }
   if (DIR)  pinMode(DIR, OUTPUT);
   au16data[50] = pday;
   au16data[52] = pmonth;
   au16data[54] = pyear;  
   au16data[93] = pmax;
   au16data[90] = p_ids; // ids 
   au16data[92] = p_madr; // madr
 }

void loop() {
   if (p_ids != ids) //change id Slave made
   {ids=au16data[90];p_ids = ids;  // save new value
    eeprom_update_byte(10,ids);    //write new value
   }
   if (p_madr != madr) //change maddr made
   {madr=au16data[92];p_madr = madr;  // save new value
    eeprom_update_byte(12,madr);      //write new value
    slave.setID(p_madr);              // set new SlaveID    
   }
   slave.poll( au16data, 127 );
   if (millis() - Previous > Interval)     
    {   //reqest
      if (dbg) Serial.println("get...");
      Previous = millis();
      GetDataCC301(Interval,ids);
    }  // mills   
   ModData2[3]  = -au16data[50]; //pday 
   ModData3[3]  = -au16data[52]; // pmonth
   ModData4[3]  = -au16data[54]; //pyear 
   
   ModData71[3] = -au16data[93]; //pmax 
   ModData72[3] = -au16data[93]; //pmax 
   ModData73[3] = -au16data[93]; //pmax 
   ModData74[3] = -au16data[93]; //pmax 
   
   p_ids  =  au16data[90]; // ids 
   p_madr =  au16data[92]; // madr
   pmax   = -au16data[93]; // pmax  
 }
