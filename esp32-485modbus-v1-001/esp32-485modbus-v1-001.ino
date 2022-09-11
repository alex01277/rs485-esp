#ifdef ESP8266
 #include <ESP8266WiFi.h>
#else //ESP32
 #include <WiFi.h>
 #include <ESPAsyncWebServer.h>
 #include <SPIFFS.h>
#endif
//#include <ModbusRTU.h>
#include <ModbusIP_ESP8266.h>
#include <EEPROM.h>
//#include <EEPROM-Storage.h>
//#include <SoftwareSerial.h> //https://github.com/plerup/espsoftwareserial
#include <Crc16.h>
#define hREGN 0
//#define MODBUSRTU_DEBUG
Crc16 crc; 
//ModbusRTU mb;
ModbusIP mb;  //ModbusIP object
AsyncWebServer aSrv(80); // Create AsyncWebServer object on port 80
WiFiServer msrv(81);
//uint16_t au16data[254];
 uint16_t Interval =   15;  //30s
 boolean  dbg      =   0;   // debug print
 bool     clearRom =   0;   // clear EEPROM    
 bool     dispRom  =   0;   // display EEPROM    
 bool     s_reboot =   0;   //soft reboot
 uint8_t  ids      =  39;   // CC301 address
 uint8_t  madr     = 199;   // 485 Modbus Slave ID
 uint8_t  p_ids,p_madr;
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
    ModData52[]={ids, 3, 52,0,0,0,0,0};  //Ki Ku fl  disp

#define MAX485_RE_NEG  4 //D38 RS485 has a enable/disable pin to transmit or receive data. Arduino Digital Pin 2 = Rx/Tx 'Enable'; High to Transmit, Low to Receive
#define RX_PIN         13 //D7
#define TX_PIN         12 //D6 

long tlastMillis = 0;
uint8_t state;
unsigned char  pday=0,    pmonth=0,   pyear=0,    pmax = 0;   // 
unsigned char  a_d=1,a_m=2,a_y=3,a_mx=4,a_id=5,a_madr=6;
uint8_t        cday=0,    cmonth=0,   cyear=0,    chour = 0;  //    current date
uint8_t        ppday=0,   ppmonth=0,  ppyear=0,   ppmax = 0;  //aux value
bool           b_pday=0,  b_pmonth=0, b_pyear=0,  b_pmax = 0; //chek  change value
uint16_t       spedss   = 19200;     // serial port speed
long ts;
float u1,u2,u3;
String  str_u1,str_u2,str_u3;
char lineBuf[80];
int charCount = 0;
//SoftwareSerial RS485;
// RS485 Serial2;

void setup(){
   //if (dbg) 
    Serial.begin(spedss, SERIAL_8N1);  // first ot all
    WiFi.begin("2123n", "313313313");
    while (WiFi.status() != WL_CONNECTED) {delay(500); if (dbg) Serial.print("."); }
    //if (dbg) 
    {Serial.println("WiFi connected...IP address: ");
     Serial.println(WiFi.localIP());}

    if(!SPIFFS.begin()){  Serial.println("An Error has occurred while mounting SPIFFS");
                          return; }
    // Route for root / web page
  aSrv.on("/", HTTP_GET, []
    (AsyncWebServerRequest *request){request->send(SPIFFS, "/index.html");} );
    aSrv.on("/u1", HTTP_GET, [](AsyncWebServerRequest *request){
              request->send_P(200, "text/plain", read_u1().c_str());});
    aSrv.on("/u2", HTTP_GET, [](AsyncWebServerRequest *request){
              request->send_P(200, "text/plain", read_u2().c_str());});
    aSrv.on("/u3", HTTP_GET, [](AsyncWebServerRequest *request){
              request->send_P(200, "text/plain", read_u3().c_str());});
    aSrv.begin();   
  EEPROM.begin(120);
  msrv.begin();
   // clearEEPROM();
  if (dbg) displayEEPROM(); 
  //  EEPROM.write(a_madr,199)
  //  EEPROM.write(a_id,39)
  //  EEPROM READ INITAL Value -- if first time run write default value to EEPROM
  p_ids = EEPROM.read(a_id);  //  p_ids = eeprom_read_byte(&a_id); //read Id cc301
  if (p_ids==255) 
    { EEPROM.write(a_id,39);  EEPROM.commit();  
      p_ids=39;ids=p_ids;
      } //default value 199  eeprom clear
  
  p_madr= EEPROM.read(a_madr);   //eeprom_read_byte(&a_madr); //read Slave ID   
  if (p_madr==255)
     { EEPROM.write(a_madr,199);  EEPROM.commit();  // eeprom_update_byte(&a_madr,199);
       p_madr=199;madr=p_madr;} //default value 199  eeprom clear
  
  pinMode(MAX485_RE_NEG, OUTPUT);     // Init in receive mode
  digitalWrite(MAX485_RE_NEG, LOW);
  mb.server();  
//  mb.begin(&Serial, 0);
//  mb.slave(madr);
  mb.addHreg(hREGN,0,120);   
  
  //RS485.begin(9600, SWSERIAL_8N1,RX_PIN, TX_PIN, false, 128);
   Serial2.begin(9600, SERIAL_8N1, 16, 17);
  mb.Hreg(50, pday  );
  mb.Hreg(42, 1  );  
  mb.Hreg(44, 1  );  
  
  mb.Hreg(52, pmonth);
  mb.Hreg(54, pyear );  

  mb.Hreg(90, p_ids  );
  mb.Hreg(92, p_madr );
  mb.Hreg(93, pmax   );  
   
  mb.Hreg(84, dbg     );
  mb.Hreg(85, Interval);
  mb.Hreg(86, clearRom);
  mb.Hreg(87, s_reboot);
  mb.Hreg(88, dispRom );
 if (dbg) {   Serial.print(  "Begin...My Slave ID-");Serial.print(madr); 
              Serial.print(" ids - ") ; Serial.println(p_ids);  }
               if (dbg) displayEEPROM();
  ts = millis();
}

void loop() {             //slave.poll( au16data, 120 );
            WiFiClient client = msrv.available();
            uint32_t uu1 = mb.Hreg(17)<<16 | mb.Hreg(16); u1=*(float*)&uu1; str_u1=String(u1,1); 
            uint32_t uu2 = mb.Hreg(19)<<16 | mb.Hreg(18); u2=*(float*)&uu2; str_u2=String(u2,1);             
            uint32_t uu3 = mb.Hreg(21)<<16 | mb.Hreg(20); u3=*(float*)&uu3; str_u3=String(u3,1); 
            if (client) { memset(lineBuf, 0, sizeof(lineBuf));
                          charCount = 0;
                          boolean currentLineIsBlank = true;  // HTTP-запрос заканчивается пустой строкой
            while (client.connected()) {
             client.println("HTTP/1.1 200 OK");
             client.println("Content-Type: text/html");
             client.println("Connection: close");
             client.println();
             // формируем веб-страницу
             String webPage = "<!DOCTYPE HTML>";
             webPage += "<html><head><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"> </head>";
             webPage += "<h1>ModBus - Web Server</h1> <table border=\"2\"> <tr>";
             webPage += "<td> U1 </td> <td>"+ str_u1 +"</td> </tr> <tr>";
             webPage += "<td> U2 </td> <td>"+ str_u2 +"</td> </tr> <tr>";             
             webPage += "<td> U3 </td> <td>"+ str_u3 +"</td> </tr> </table>  </html>";
             client.println(webPage);
             break;
        }
      client.stop();
    }

  mb.task();
 // yield();
  if (millis() > ts + Interval*1000) 
  {  // ESP.wdtDisable();
    ts = millis();
     if (dbg) {Serial.print("get data from CC301 ");Serial.print(ids);Serial.print(madr);}
      //Previous = millis();
      GetDataCC301(ids);
     // ESP.wdtEnable(1);
     // lastMillis = currentMillis;

     ppday   =  mb.Hreg(50);
      if (ppday != pday) {  //change  pday acc value
        b_pday=true;  pday = ppday;
        EEPROM.write(a_d,pday);  EEPROM.commit();  
        if (dbg) displayEEPROM();}  //new value
    
      ppmonth   =  mb.Hreg(52);
      if (ppmonth != pmonth) {  //change  pmonth acc value
        b_pmonth=true;  pmonth = ppmonth;
        EEPROM.write(a_m,pmonth);  EEPROM.commit();  
        if (dbg) displayEEPROM();}  //new value

      ppyear   =  mb.Hreg(54);
      if (ppyear != pyear) {  //change  pyear acc value
        b_pyear=true;  pyear = ppyear;
        EEPROM.write(a_y,pyear);  EEPROM.commit();  
        if (dbg) displayEEPROM();}  //new value
    
    ppmax   =  mb.Hreg(93);
    if (ppmax != pmax) {  //change  pyear acc value
      b_pmax=true;  pmax = ppmax;
      EEPROM.write(a_mx,pmax);  EEPROM.commit();  
      if (dbg) displayEEPROM();}  //new value

   if (p_ids != ids) //change id Slave made
   {ids=mb.Hreg(90);p_ids = ids;  // save new value
    EEPROM.write(a_id,ids);    EEPROM.commit();    //write new value
    if (dbg) displayEEPROM();
   }
   if (p_madr != madr) //change maddr made
   {madr=mb.Hreg(92);p_madr = madr;  // save new value
    EEPROM.write(a_madr,madr);   EEPROM.commit();       //write new value
     mb.slave(madr);             // set new SlaveID  
    if (dbg)displayEEPROM();  
   }  
   dbg      =  mb.Hreg(84);   
   Interval =  mb.Hreg(85);   
   clearRom =  mb.Hreg(86);   
   s_reboot =  mb.Hreg(87);      

   ModData2[3]  = mb.Hreg(50)-cday;   //pday 
   ModData3[3]  = mb.Hreg(52)-cmonth; // pmonth
   ModData4[3]  = mb.Hreg(54)-cyear;  //pyear 
   
   ModData71[3] = -mb.Hreg(93); //pmax 
   ModData72[3] = -mb.Hreg(93); //pmax 
   ModData73[3] = -mb.Hreg(93); //pmax 
   ModData74[3] = -mb.Hreg(93); //pmax 
   
   p_ids    = mb.Hreg(90); // ids 
   p_madr   = mb.Hreg(92); // madr
   
   dbg      = mb.Hreg(84);   
   Interval = mb.Hreg(85);   
   clearRom = mb.Hreg(86);
   s_reboot = mb.Hreg(87);
   dispRom  = mb.Hreg(88);
   cyear    = mb.Hreg(46); //y
   cmonth   = mb.Hreg(47); // month
   cday     = mb.Hreg(48); // day 
  }
}
        
