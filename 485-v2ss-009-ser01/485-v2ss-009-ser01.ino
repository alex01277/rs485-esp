#include <ModbusRtu.h>
#include <avr/eeprom.h> 
//#include <EEPROM-Storage.h>
#include <Crc16.h>
#include <SoftwareSerial.h>
#define DIR 8 // пин управления прием/передача
  Crc16 crc; 
  SoftwareSerial RS485(7,6);// RX, TX
  uint16_t au16data[254];
//  uint16_t au16data1[17];  
  uint16_t Interval =   3;   //30s
  boolean  dbg      =   1;   // debug print
  bool     clearRom =   0;   // clear EEPROM    
  bool     dispRom  =   0;   // display EEPROM    
  bool     s_reboot =   0;   //soft reboot
  uint8_t  ids      = 199;   // CC301 address
  uint8_t  madr     = 199;   // 485 Modbus Slave ID
  uint8_t  p_ids,p_madr;
  int8_t   state   =   0;
  Modbus slave(madr,Serial,3); // this is slave @1 and RS-232 or USB-FTDI  
//  Modbus slave1(madr+1,Serial,3); // this is slave @1 and RS-232 or USB-FTDI    
  uint8_t // reqests
    ModData1[] ={ids, 4,  1,0,0,0,0,0},  //p
    ModData2[] ={ids, 4,  2,0,0,0,0,0},  //de day
    ModData3[] ={ids, 4,  3,0,0,0,0,0},  //de month 
    ModData4[] ={ids, 4,  4,0,0,0,0,0},  //de y
    ModData71[]={ids, 4,  7,0,0,1,0,0},  //p max+
    ModData72[]={ids, 4,  7,0,0,2,0,0},  //p max-    
    ModData73[]={ids, 4,  7,0,0,3,0,0},  //q max+    
    ModData74[]={ids, 4,  7,0,0,4,0,0},  //q max-        
    ModData8[] ={ids, 4,  8,0,0,0,0,0},  //p a
    ModData9[] ={ids, 4,  9,0,0,0,0,0},  //p r 
    ModData10[]={ids, 4, 10,0,0,0,0,0},  //u 
    ModData11[]={ids, 4, 11,0,0,0,0,0},  //i 
    ModData12[]={ids, 4, 12,0,0,0,0,0},  //k p
    ModData13[]={ids, 4, 13,0,0,0,0,0},  //f 
    ModData21[]={ids, 3, 21,0,0,0,0,0},  // slave ID     
    ModData25[]={ids, 3, 25,0,0,0,0,0},  //Ki int 
    ModData26[]={ids, 3, 26,0,0,0,0,0},  //Ku int 
    ModData32[]={ids, 3, 32,0,0,0,0,0},  //Current Date Time 
    ModData34[]={ids, 3, 34,0,0,0,0,0},  //Ki Ku int disp
    ModData42[]={ids, 4, 42,0,0,0,0,0},  // Pakk Day     
    ModData43[]={ids, 4, 43,0,0,0,0,0},  // Pakk Month         
    ModData44[]={ids, 4, 44,0,0,0,0,0},  // Pakk Year        
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
     unsigned char   pday=0,    pmonth=0,   pyear=0,    pmax = 0;   // 
    unsigned char a_d=2,a_m=4,a_y=6,a_mx=8,a_id=10,a_madr=12;
   uint8_t  cday=0,    cmonth=0,   cyear=0,    chour = 0;  //    current date
   uint8_t      ppday=0,   ppmonth=0,  ppyear=0,   ppmax = 0;  //aux value
   bool     b_pday=0,  b_pmonth=0, b_pyear=0,  b_pmax = 0; //chek  change value
   uint16_t spedss   = 19200;     // serial port speed
   uint32_t Previous = 0;
   unsigned long tempus;
   bool     ppp = 0;   
 void setup() {//  p_ids=ids; p_madr=madr;
  for (int i9=0;i9<126;i9++) au16data[i9]=0 ;
    eeprom_update_byte(&a_d,pday);  //inital value
    eeprom_update_byte(&a_m,pmonth);
    eeprom_update_byte(&a_y,pyear);
    eeprom_update_byte(&a_mx,pmax);
  
   Serial.begin(spedss); // baud-rate at 19200
  
    if (clearRom) {
     displayEEPROM(); Serial.println(); 
     clearEEPROM();   Serial.println();
     displayEEPROM(); Serial.println(); }

   if (dbg) {   
      printHeader("EEPROM Contents (" + String(EEPROM.length()) + " bytes)");
      displayEEPROM(); 
      Serial.println(); 
      }
//  EEPROM READ INITAL Value -- if first time run write default value to EEPROM
    p_ids = eeprom_read_byte(&a_id); //read Id cc301
    if (p_ids==255) 
     { eeprom_update_byte(&a_id,199);
       p_ids=199;ids=p_ids;} //default value 199  eeprom clear
    p_madr= eeprom_read_byte(&a_madr); //read Slave ID   
    if (p_madr==255)
     { eeprom_update_byte(&a_madr,199);
       p_madr=199;madr=p_madr;} //default value 199  eeprom clear
    slave.setID(p_madr);  // set new SlaveID

   RS485.begin(9600);
   slave.setTimeOut(100);
 //  tempus = slave.getTimeOut();
   slave.start();   //   slave1.start();   
   if (DIR)  pinMode(DIR, OUTPUT);
   if (dbg) { 
      Serial.print(  "Begin...My Slave ID-");Serial.print(slave.getID()); 
      Serial.print(" ids - ") ; Serial.println(p_ids); 
   //   Serial.print(" TimeOut - ") ;Serial.println(slave.getTimeOut());
       }
   au16data[50] = pday;
   au16data[52] = pmonth;
   au16data[54] = pyear;  
   
   au16data[90] = p_ids; // ids 
   au16data[92] = p_madr; // madr
   au16data[93] = pmax;   
   
   au16data[84] = dbg;   
   au16data[85] = Interval;   
   au16data[86] = clearRom;   
   au16data[87] = s_reboot;      
   au16data[87] = dispRom;  
   
   //for (int i8=0;i8<16;i8++) au16data1[i8]=i8;
 }

void loop() { //void  swing(int pdata, int ppdata, boolean b_pdata,byte nregs,byte nepr)
    //slave1.poll( au16data1, 16 );
    state = slave.poll( au16data, 250 );
    if (state > 4) { //Si es mayor a 4 = el pedido fué correcto
    //Previous = millis() + 500; //Tiempo actual + 50ms delay(1000);
   if (millis() - Previous >= Interval*1000)     
    { //reqest
      if (dbg) {Serial.print("get data from CC301 ");}
      Previous = millis();
      GetDataCC301(Interval,ids);
      
      ppday   =  au16data[50];
      if (ppday != pday) {  //change  pday acc value
        b_pday=true;  pday = ppday;
        eeprom_update_byte(&a_d,pday);
        if (dbg) displayEEPROM();}  //new value
    
      ppmonth   =  au16data[52];
      if (ppmonth != pmonth) {  //change  pmonth acc value
        b_pmonth=true;  pmonth = ppmonth;
        eeprom_update_byte(&a_m,pmonth);
        if (dbg) displayEEPROM();}  //new value

      ppyear   =  au16data[54];
      if (ppyear != pyear) {  //change  pyear acc value
        b_pyear=true;  pyear = ppyear;
        eeprom_update_byte(&a_y,pyear);
        if (dbg) displayEEPROM();}  //new value
    
    ppmax   =  au16data[93];
    if (ppmax != pmax) {  //change  pyear acc value
      b_pmax=true;  pmax = ppmax;
      eeprom_update_byte(&a_mx,pmax);
      if (dbg) displayEEPROM();}  //new value

   if (p_ids != ids) //change id Slave made
   {ids=au16data[90];p_ids = ids;  // save new value
    eeprom_update_byte(&a_id,ids);    //write new value
    if (dbg) displayEEPROM();
   }
   if (p_madr != madr) //change maddr made
   {madr=au16data[92];p_madr = madr;  // save new value
    eeprom_update_byte(&a_madr,madr);      //write new value
    slave.setID(p_madr);              // set new SlaveID  
    if (dbg)displayEEPROM();  
   }  
  
   dbg      =  au16data[84];   
   Interval =  au16data[85];   
   clearRom =  au16data[86];   
   s_reboot =  au16data[87];      

   ModData2[3]  = au16data[50]-cday;   //pday 
   ModData3[3]  = au16data[52]-cmonth; // pmonth
   ModData4[3]  = au16data[54]-cyear;  //pyear 
   
   ModData71[3] = -au16data[93]; //pmax 
   ModData72[3] = -au16data[93]; //pmax 
   ModData73[3] = -au16data[93]; //pmax 
   ModData74[3] = -au16data[93]; //pmax 
   
   p_ids    = au16data[90]; // ids 
   p_madr   = au16data[92]; // madr
   
   dbg      = au16data[84];   
   Interval = au16data[85];   
   clearRom = au16data[86];
   s_reboot = au16data[87];
   dispRom  = au16data[88];
   cyear    = au16data[46]; //y
   cmonth   = au16data[47]; // month
   cday     = au16data[48]; // day 

   if (clearRom) {
     displayEEPROM(); Serial.println(); 
     clearEEPROM();   Serial.println();
     displayEEPROM(); Serial.println(); 
     clearRom = 0;    au16data[86] = clearRom;   }

   if (dispRom) {
     displayEEPROM(); Serial.println(); 
     dispRom = 0;     au16data[88] = dispRom;    }

   if (s_reboot){
       s_reboot=0;
       au16data[87] = s_reboot;   
       softwareReset( 3000 );    }
    } // mills          
    } //state 
  } //loop
