
//-----------------------------------------------------------------
//                              INCLUDES
//-----------------------------------------------------------------
#include <IRremote.h>
#include <dht.h>
#include <ELClient.h>
#include <ELClientMqtt.h>
#include <Wire.h>
#include <BH1750.h>

//-----------------------------------------------------------------------
//      PIN 
//----------------------------------------------------------------------
const int PIN_LIGHT=0;
const int PIN_PIR=12; 
const int PIN_DHT=13; 
const int PIN_RECV=8;
const int PIN_SOUND=6;
const int PIN_SOUNDA= A0; 
const int PIN_RADAR=2;
//const int PIN_9 
//const int PIN_10 
//const int PIN_11 
//const int PIN_12
int hexcode=0;
//--------------------------------------------------------------------
//                            Real time pic 2 -  sensor array
//--------------------------------------------------------------------
// all the data is saveed in this array 
const int num =9;
int RT_value[num];

char send_array[12];
const int value_ptr_light=0;
const int value_ptr_movement=1;
const int value_ptr_temp=2;
const int value_ptr_humi=3;
const int value_ptr_sound=4;
const int value_ptr_soundA=5;
const int value_ptr_radar=6;
//-----------------------------------------------------------------
//                              "temp"s
//-----------------------------------------------------------------
const char Topic="1";

//-----------------------------------------------------------------
//                               VARS
//-----------------------------------------------------------------
//IR vars
String readString;
String code;
unsigned long value; // The code value if not raw
//unsigned int rawCodes[RAWBUF]; // The durations if raw
//int codeLen; // The length of the code
//the time we give the sensor to calibrate (10-60 secs according to the datasheet)
int calibrationTime = 30;        
//the time when the sensor outputs a low impulse
long unsigned int lowIn;         
//the amount of milliseconds the sensor has to be low 
//before we assume all motion has stopped
long unsigned int pause = 5000;  

boolean lockLow = true;
boolean takeLowTime; 
//radar
bool state;
bool lastState;
// Initialize the IO

//Creats a DHT object
dht DHT; 
String Ir_code;

IRsend irsend;
IRrecv irrecv(PIN_RECV);
decode_results results;
char letter = Serial.read();

BH1750 sensor(LOW);
//---------------------------------mqtt intlize------------------------------------------
ELClient esp(&Serial, &Serial);
ELClientMqtt mqtt(&esp);
//---------------------------------------------------------
// FUNCTIONS 
//---------------------------------------------------------

//--------------------------------mqtt funcshens-------------------------------
void wifiCb(void* response) {
  ELClientResponse *res = (ELClientResponse*)response;
  if (res->argc() == 1) {
    uint8_t status;
    res->popArg(&status, 1);

    if(status == STATION_GOT_IP) {
      Serial.println("WIFI CONNECTED");
    } else {
      Serial.print("WIFI NOT READY: ");
      Serial.println(status);
    }
  }
}

bool connected;

// Callback when MQTT is connected
//---------------------------------------------------------------------------------------mqtt subscribe!!!!!!!--------------------------------------------------------------------------
void mqttConnected(void* response) {
  Serial.println("MQTT connected!");
  mqtt.subscribe("send_ir");
  //mqtt.subscribe("/esp-link/1");
 // mqtt.subscribe("/rumah/pompa/#");
  
  connected = true;
}

// Callback when MQTT is disconnected
void mqttDisconnected(void* response) {
  Serial.println("MQTT disconnected");
  connected = false;
}

// Callback when an MQTT message arrives for one of our subscriptions
void mqttData(void* response) {
  ELClientResponse *res = (ELClientResponse *)response;

  Serial.print("Received:");
  String Topic = res->popString();
//  Serial.println("temp");
  Serial.println(Topic);
  if(Topic=="send_ir")
  {
    Ir_code=res->popString();
    send_ir(Ir_code);
    
  }
  else
  {
  Serial.print("data=");
  String data = res->popString();
  Serial.println(data);
  }
}

void mqttPublished(void* response) {
  Serial.println("MQTT published");
}

//-------------------------------------------------------------------------------------------------------------
//--------------------------------CHECKS THE IR DO NOT TOUCH!!!!---------------------------------------------------------------
decode_results input;
void checkbutton (){
  send_array[0]=1;
char sendir[32];
 if (irrecv.decode(&results)){
         
       
           Serial.println(results.value,HEX);
           unsigned long numin=irparams.recvpin;
           Serial.println("---"+numin);
    // value.toCharArray(sendir,32);
       itoa(results.value,sendir,32);
//       for(int i=0;i<16;i++)
//       {
//        Serial.print(sendir[i]);
//       }
       Serial.println();
       mqtt.publish("IR_recive",sendir);
           switch (results.decode_type){
            case NEC:{ Serial.println("NEC");
            int a=101;
            itoa(a,send_array,10);
            mqtt.publish("temp",send_array); break ;}
            case SONY:{ Serial.println("SONY");
            int a=102;
            itoa(a,send_array,10);
            mqtt.publish("temp",send_array); break ;}
            case RC5: {Serial.println("RC5");
            int a=103;
            itoa(a,send_array,10);
            mqtt.publish("temp",send_array); break ;}
            case RC6: {Serial.println("RC6");
             int a=104;
            itoa(a,send_array,10);
            mqtt.publish("temp",send_array); break ;}
          default:
            case UNKNOWN: {Serial.println("UNKNOWN");
            int a=105;
            itoa(a,send_array,10);
            mqtt.publish("temp",send_array); break ;}
           }
        irrecv.resume();
         } 
}

//----------------------------------------RADAR----------------------------------------------
void RADAR(int snum)

{ int a=8;
  pinMode(PIN_RADAR, INPUT);
  send_array[0]=2;

    // read the state of the sensor and see if it 
    // matches the last known state
    if((state = digitalRead(PIN_RADAR)) != RT_value[snum])
    {
        
        RT_value[snum] = state;
        a=a+(RT_value[snum]*10);
        itoa(a,send_array,10);
        mqtt.publish("temp",send_array); 
        // sensor indicates active when it pulls
        // the input to 0.
        Serial.println("polled - " + String(state ? "IDLE" : "ACTIVE"));
    }
    yield();
    delay(500);
}


//-----------------------------------SENDIR---------------------------------------------

void send_ir(String code)
{
  mqtt.publish("send_ir1","started");    
// char c[32];
// code.toCharArray(c, 32);
 //value = code.toInt();
value = strtoul (code,NULL,32);
 Serial.println(code); //prints string to serial port out
//value = strtoul( c,0, 32); 
//irsend.sendNEC( c, 32);
//irsend.sendNEC(value, 32);
irsend.sendRaw(code, 32,38); //send 0x0 code (8 bits)



      Serial.println(value,16);
      Serial.println(value,32);
      Serial.println(c);
      Serial.println(code);
     irrecv.enableIRIn();
     mqtt.publish("send_ir1","done");
     Serial.println("---------------------------------------------------------");
}
//---------------------------------------PIR--------------------------------------------
void PIR(int pin,int snum)
{
  send_array[0]=2;
  int a=2;
  int RT_pir=digitalRead(pin);

  if(RT_pir!=RT_value[snum])
    { 
     RT_value[snum]=RT_pir;
    Serial.println("RT_pir");
    Serial.println(RT_pir);
    a=a+(RT_value[snum]*10);
    itoa(a,send_array,10);
    Serial.println(send_array);
    mqtt.publish("temp",send_array);
    }
}
//-------------------------------------------------------------------TEMP------------------------------------------------------------------------
void temp (int pin,int snum) 
{
    int a;
    int readData = DHT.read11(pin); // Reads the data from the sensor
    int RT_temp = DHT.temperature; // Gets the values of the temperature
    int RT_humi= DHT.humidity; // Gets the values of the humidity
    uint8_t change=RT_temp-RT_value[snum];
    change=abs(change);
   if (change>1)
    {    
    Serial.print("temp " );
    Serial.println(RT_temp);
    RT_value[snum]=RT_temp;
    a=3;
    a=a+(RT_temp*10);
    itoa(a,send_array,10);
    Serial.println(send_array);
    mqtt.publish("temp",send_array);
    }
    change=RT_humi-RT_value[snum+1];
    change=abs(change);
   if (change>2)
   {
      Serial.print("humi ");
      Serial.println(RT_humi);
      RT_value[snum+1]=RT_humi;
      a=4;
      a=a+(RT_humi*10);
      itoa(a,send_array,10);   
      Serial.println(send_array);
      mqtt.publish("temp",send_array);
    
    }
}
//-----------------------------------------------------------------LIght--------------------------------------------------------------------------------
void light(int pin,int snum)
{
  int a=5;
   int RT_light = analogRead(pin);  
  uint16_t lux;
  uint8_t change=RT_light-RT_value[snum];
 change=abs(change); 
 if(change>5)
 {
  RT_value[snum]=RT_light;
  // Wait for completion (blocking busy-wait delay)
  if (sensor.isConversionCompleted()) {
    // Read light
    lux = sensor.read();

    // Print light
    Serial.print(F("Light: "));
    Serial.print(lux / 2);
 
      a=a+((lux / 2)*10);
      itoa(a,send_array,10);   
      Serial.println(send_array);
      mqtt.publish("temp",send_array);
      delay(50);
}
}  
}
//----------------------------------------------------------------MIC------------------------------------------------------------------------------------
void voice(int pin1,int pin2,int snum)
{
  int a;
 int  RT_soundA = analogRead (pin1) ;
 int  RT_soundD = digitalRead(pin2);
 uint8_t change=RT_soundD - RT_value[snum];
 change=abs(change);
 if (change>2) // If we hear a sound
 {
  RT_value[snum]=RT_soundD;
  Serial.print("sound");
  Serial.println((RT_soundA));
  Serial.println((RT_soundD));
  a=6;
  a=a+(RT_soundA*10);
  itoa(a,send_array,10);
  Serial.println(send_array);
  mqtt.publish("temp",send_array);
  a=7 ;
  a=a+(RT_value[snum]*10);
  itoa(a,send_array,10);
  Serial.println(send_array);
  mqtt.publish("temp",send_array);                                                         //need to change 

  }
 }
//-----------------------------------------------------------
//                        SETUP
//-----------------------------------------------------------



void setup(){
  Serial.begin(115200);
  ////////////////////////////////////////////----------------------------light sensor////////////////////////////////////
  // Initialize I2C bus
  Wire.begin();
  // Initialize sensor in continues mode, high 0.5 lx resolution
  sensor.begin(ModeContinuous, ResolutionHigh);
  // Start conversion
  sensor.startConversion();



  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  Serial.println("EL-Client starting!");

  // Sync-up with esp-link, this is required at the start of any sketch and initializes the
  // callbacks to the wifi status change callback. The callback gets called with the initial
  // status right after Sync() below completes.
  esp.wifiCb.attach(wifiCb); // wifi status change callback, optional (delete if not desired)
  bool ok;
  do {
    ok = esp.Sync();      // sync up with esp-link, blocks for up to 2 seconds
    if (!ok) Serial.println("EL-Client sync failed!");
  } while(!ok);
  Serial.println("EL-Client synced!");
  
  mqtt.connectedCb.attach(mqttConnected);
  mqtt.disconnectedCb.attach(mqttDisconnected);
  mqtt.publishedCb.attach(mqttPublished);
  mqtt.dataCb.attach(mqttData);
  mqtt.setup();
  //Serial.println("ARDUINO: setup mqtt lwt");
  //mqtt.lwt("/lwt", "offline", 0, 0); //or mqtt.lwt("/lwt", "offline");
  Serial.println("EL-MQTT ready");
  
  
  irrecv.enableIRIn();
  irrecv.blink13(true);
  pinMode(PIN_PIR, INPUT);
  digitalWrite(PIN_PIR, LOW);
   pinMode(PIN_RADAR, INPUT);
//    give the sensor some time to calibrate
  Serial.println( "calibrating sensor" );
    for(int i = 0; i == calibrationTime; i++){
      Serial.print(".");
      delay(100);
      }
    Serial.println( "done");
    Serial.println("SENSOR ACTIVE");
    delay(50);
    for(int i=0;i<num;i++)
    {
    RT_value[i]=0;
    }


    
//------------------WIFI------------------------------------------------------------------------------------------------------------------------------
                      //enter real data
    mqtt.subscribe ("temp", 1);

}
static int count;
static uint32_t last;

//--------------------------------------------------------------
//                             LOOP 
//--------------------------------------------------------------
void loop()
{
   esp.Process();


 void mqttData();

 // ----------------IR loop---------------------------
/* if (Serial.available() > 0)                                                                 //someway to get IR string
{
    code = Serial.readString();
    send_ir(code);
 }/*==*/
//----------------sensors loop----------------------
  PIR(PIN_PIR,value_ptr_movement);

  temp(PIN_DHT,value_ptr_temp);

  light(PIN_LIGHT,value_ptr_light);

  voice(PIN_SOUNDA,PIN_SOUND,value_ptr_sound);

  checkbutton();

  RADAR (value_ptr_radar);

  mqtt.publish("keep_alive","pirymide_alive"); 
  
}
  
 

