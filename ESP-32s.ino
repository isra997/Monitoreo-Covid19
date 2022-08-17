#include <Wire.h>
#include <U8x8lib.h>
#include <Keypad.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "MAX30105.h"           //MAX3010x library
#include "heartRate.h"          //Heart rate calculating algorithm
#include "spo2_algorithm.h"
///conexion
const char* ssid = "UTA-WiFi";
//const char* ssid = "NETLIFE-DANIEL";
//const char* pass= "1850184613Dsanchez";
int c=0;

////////////
// Change this depending on where you put your piezo buzzer
const int TONE_OUTPUT_PIN = 25;

// The ESP32 has 16 channels which can generate 16 independent waveforms
// We'll just choose PWM channel 0 here
const int TONE_PWM_CHANNEL = 0; 


int val=0;
int aler=0;
int ced=0;
////////
const char* mqtt_server = "20.231.26.227";
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
char saturacion[20];
char temperatura[20];
char cedula[20];
char temperatura2[20];
char alerta[10];
///
int a=0;
#define MAX_BRIGHTNESS 255

#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // four columns

int freq = 2000;
int channel = 0;
int resolution = 8;


char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pin_rows[ROW_NUM]      = {2,0,4,16}; // GIOP19, GIOP18, GIOP5, GIOP17 connect to the row pins
byte pin_column[COLUMN_NUM] = {17,5,18,19};   // GIOP16, GIOP4, GIOP0, GIOP2 connect to the column pins

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

//////////////////////////////////////////////////

///
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
uint16_t irBuffer[100]; //infrared LED sensor data
uint16_t redBuffer[100]; //red LED sensor data
#else
uint32_t irBuffer[100]; //infrared LED sensor data
uint32_t redBuffer[100]; //red LED sensor data
#endif
//variables spo2
int32_t tam;
int32_t spo2;
int32_t sat;
int8_t bool_spo2; 
int32_t heartRate;
int8_t bool_hr;
//variables para MQTT
char str_hr[10];
char str_spo2[10];
char str_temp[10];
char str_sys[10];
char str_dia[10];
char str_ecg[10];
float heartRate_suma=0, heartRate_media = 0;
int valor = 0;
int k=0, j=0;
///logos
static const unsigned char PROGMEM logo_sat[] =
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 
  0x03, 0x30, 0x00, 0x00, 0x06, 0x18, 0x00, 0x00, 0x04, 0x18, 0x00, 0x00, 0x0c, 0x0c, 0x00, 0x00, 
  0x18, 0x04, 0x00, 0x00, 0x18, 0x06, 0x00, 0x00, 0x30, 0x03, 0x00, 0x00, 0x20, 0x03, 0x00, 0x00, 
  0x60, 0x01, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x40, 0x0f, 0x80, 0x00, 0x40, 0x19, 0xc0, 0x00, 
  0x40, 0x30, 0xc0, 0x00, 0x40, 0x20, 0x60, 0x00, 0x60, 0x20, 0x6e, 0x00, 0x60, 0x20, 0x69, 0x00, 
  0x30, 0x20, 0x62, 0x00, 0x1c, 0x30, 0xc6, 0x00, 0x0f, 0x98, 0xc8, 0x00, 0x03, 0xcf, 0x8f, 0x00, 
  0x00, 0x00, 0x00, 0x00 };

static const unsigned char PROGMEM logo_temp[] =
{ 0x00, 0x3c, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0xc3, 0x00, 0x00, 0x00, 0x81, 0xf0, 0x00, 
  0x00, 0x81, 0xe0, 0x00, 0x01, 0x81, 0x80, 0x00, 0x00, 0x9d, 0xe0, 0x00, 0x00, 0x9d, 0xf0, 0x00, 
  0x00, 0x9d, 0x80, 0x00, 0x00, 0x9d, 0x80, 0x00, 0x00, 0x9d, 0xf0, 0x00, 0x00, 0x9d, 0xa0, 0x00, 
  0x00, 0x9d, 0x80, 0x00, 0x01, 0x9d, 0xc0, 0x00, 0x03, 0x1c, 0xc0, 0x00, 0x03, 0x1c, 0x60, 0x00, 
  0x06, 0x3e, 0x60, 0x00, 0x06, 0x7e, 0x20, 0x00, 0x06, 0x7e, 0x20, 0x00, 0x06, 0x3e, 0x60, 0x00, 
  0x02, 0x1c, 0x60, 0x00, 0x03, 0x00, 0xc0, 0x00, 0x01, 0x81, 0xc0, 0x00, 0x00, 0xff, 0x80, 0x00, 
  0x00, 0x3e, 0x00, 0x00};

static const unsigned char PROGMEM logo_adver[] =
{ 0x00, 0x00, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x03, 0xc0, 0x00, 
  0x00, 0x07, 0xe0, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x1c, 0x38, 0x00, 
  0x00, 0x1c, 0x38, 0x00, 0x00, 0x3c, 0x3c, 0x00, 0x00, 0x7c, 0x3e, 0x00, 0x00, 0x7c, 0x3e, 0x00, 
  0x00, 0xfc, 0x3f, 0x00, 0x00, 0xfc, 0x3f, 0x00, 0x01, 0xfc, 0x3f, 0x80, 0x03, 0xfc, 0x3f, 0xc0, 
  0x03, 0xfc, 0x3f, 0xc0, 0x07, 0xfc, 0x3f, 0xe0, 0x07, 0xfc, 0x3f, 0xe0, 0x0f, 0xfc, 0x3f, 0xf0, 
  0x1f, 0xff, 0xff, 0xf8, 0x1f, 0xfe, 0x7f, 0xf8, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 
  0x7f, 0xfe, 0x7f, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xfe};
  

//Oxigenación
const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;
long irValue;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
//Dlecaración de OLED conectado a i2c
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
 
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
MAX30105 particleSensor;
 

////conexion WIfI
void setup_wifi() {
 
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
 //WiFi.begin(ssid,pass);
  WiFi.begin(ssid);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
////////


void setup() 
{
   
      Serial.begin(115200);
      ///////////////////
      ledcSetup(channel, freq, resolution);
      ledcAttachPin(15, channel);  
      //pinMode(13,OUTPUT);
      //sensor Temp
      mlx.begin();  
      
      //max30102
      // Initialize sensor
      if (!particleSensor.begin(Wire, 0x57)) 
      {
      Serial.println("MAX30105 was not found. Please check wiring/power. ");
      while (1);
      }
      Serial.println("Place your index finger on the sensor with steady pressure.");
     ////
      //indicado según la hoja del fabricante
       byte ledBrightness = 60; //Opciones: 0=Off to 255=50mA
       byte sampleAverage = 4; //Opciones: 1, 2, 4, 8, 16, 32
       byte ledMode = 2; //Opciones: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
       byte sampleRate = 100; //Opciones: 50, 100, 200, 400, 800, 1000, 1600, 3200
       int pulseWidth = 411; //Opciones: 69, 118, 215, 411
       int adcRange = 4096; //Opciones: 2048, 4096, 8192, 16384
      /////
        particleSensor.setup(); //Configure sensor with default settings
        particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
        particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
        particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
      //Pantall OLED
     
      if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
        }
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  delay(2000);
  display.clearDisplay();

  ledcAttachPin(TONE_OUTPUT_PIN, TONE_PWM_CHANNEL);
  //////////
}

/////////////////////////

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
 
  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
 
}
 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("datos/saturacion", "hello world");
      // ... and resubscribe
      //client.subscribe("datos/saturacion");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
////////////////////////
//////////Guardar el numero///////
long getKeypadIntegerMulti()
{
  
  int value = 0;                                // the number accumulator
  String borrar="";
  long keyvalue;                                     // the key pressed at current moment
  int isnum;
  Serial.println("Enter the digits,press any non-digit to end ");
  Serial.print("You have typed: ");
    display.clearDisplay();
    display.setTextSize(2);                    
    display.setTextColor(WHITE);             
    display.setCursor(0,0); 
    display.print("Ingrese el");
    display.setCursor(0,30); 
    display.print("# cedula");  
    display.display();

  do
  {
    
    keyvalue = keypad.getKey();                          // input the key
    isnum = (keyvalue >= '0' && keyvalue <= '9' || keyvalue == 'A');         // is it a digit?
    if (isnum)
    {

      if(keyvalue >= '0' && keyvalue <= '9'){
        Serial.print(keyvalue - '0');
        value = value * 10 + keyvalue - '0';               // accumulate the input number
        borrar=value;
      /////display/////
           
           display.clearDisplay();
           display.setTextSize(2);                    
           display.setTextColor(WHITE);             
           display.setCursor(0,0); 
           
           display.print(value); 
           display.display();                
      /////display/////
        
        }else{
          int longitud=borrar.length();
          borrar = borrar.substring(0, longitud-1);
          Serial.println(borrar);
           value=borrar.toInt();
           display.clearDisplay();
           display.setTextSize(2);                    
           display.setTextColor(WHITE);             
           display.setCursor(0,0); 
           
           display.print(value); 
           display.display();
        }
      
    }
    
  } while (isnum || !keyvalue);                          // until not a digit or while no key pressed
  //
  
  Serial.println(" "); 
  Serial.print("Returning from funtion: "); 
  Serial.println(value);
  ///////////
  int d1=0;
  while(d1<=5){
  display.clearDisplay();
  display.setTextSize(2);                    
  display.setTextColor(WHITE);             
  display.setCursor(0,0); 
  display.println("Tu ID es:"); 
  display.setCursor(0,38); 
  display.print(value); 
  display.display();
  delay(375);
  d1++;
  }
  ///////////
  return value;
ledcWrite(TONE_PWM_CHANNEL, 0);
}


//////////////////////////////////////////

 
void loop() 
{
  ledcWrite(TONE_PWM_CHANNEL, 0);
  tam = 100;

     for (byte i = 0 ; i < tam ; i++)
     {
     while (particleSensor.available() == false)
     particleSensor.check(); 
    
     redBuffer[i] = particleSensor.getRed();
     irBuffer[i] = particleSensor.getIR();
     particleSensor.nextSample();
     Serial.print(F("red="));
     Serial.print(redBuffer[i], DEC);
     Serial.print(F(", ir="));
     Serial.println(irBuffer[i], DEC);
     }
     maxim_heart_rate_and_oxygen_saturation(irBuffer, tam, redBuffer, &spo2, &bool_spo2, &heartRate, &bool_hr);
     while (1)
     {
     for (byte i = 25; i < 100; i++)
     {
     redBuffer[i - 25] = redBuffer[i];
     irBuffer[i - 25] = irBuffer[i];
     }
    
     for (byte i = 75; i < 100; i++)
     {
     while (particleSensor.available() == false)
     particleSensor.check();
     redBuffer[i] = particleSensor.getRed();
     irBuffer[i] = particleSensor.getIR();
     particleSensor.nextSample();
     heartRate_suma+=heartRate;
     k++;
     heartRate_media = heartRate_suma/k;
     maxim_heart_rate_and_oxygen_saturation(irBuffer, tam, redBuffer, &spo2, &bool_spo2, &heartRate, &bool_hr);
     ///////////////////////
    
      /////////Condicion para detectar si se esta sensando
      if(redBuffer[i]<5000){
        ///OLED
           display.clearDisplay();
           val= getKeypadIntegerMulti(); 
           Serial.println("Value is");
           Serial.println(val);
          
        }else{
          
          ////////Recolección de datos/////////////
           float tempc=mlx.readObjectTempC();
         
          
           
         c=0;
          while (c<=5){
            //////////////////////
           display.clearDisplay();
           display.setTextSize(2);                    
           display.setTextColor(WHITE);             
           display.setCursor(0,0);                
           display.println("Sensando"); 
           display.setCursor(0,30);
           display.println("...");  
           display.display();
           /////////////////////
           delay(500);
            c++;
            }
              /////tono de medida////
              ledcWriteNote(TONE_PWM_CHANNEL, NOTE_D, 5);
              delay(350);
              ledcWrite(TONE_PWM_CHANNEL, 0);
              delay(200);
              ledcWriteNote(TONE_PWM_CHANNEL, NOTE_D, 5);
              delay(350);  

              ledcWrite(TONE_PWM_CHANNEL, 0);
              delay(200);
              
              float satur=sat;
              
               ////////alerta///////////
            if(sat <92 || tempc >=38){
                 aler=1;
              }
             if(sat <92 && tempc >=38){
                 aler=2;
                }
            if(sat >=92 && tempc <=37){
                 aler=0; 
                }
               ////conversion
            dtostrf(val,10,0,cedula);
            dtostrf(sat,4,2,saturacion);
            dtostrf(tempc,4,2,temperatura);
            dtostrf(aler,4,2,alerta);
           
             ////// Envio de datos por MQTT /////////
            if (!client.connected()) {
             reconnect();
              }
            client.loop();
            long now = millis();
            if (now - lastMsg > 2000) {
              lastMsg = now;

              client.publish("datos/saturacion", saturacion);
              client.publish("datos/temperatura", temperatura);
              client.publish("datos/alerta", alerta);
              client.publish("datos/cedula", cedula);
            }  
             ////   
              
              int d=0; 
              while(d<=5){
               ///PANTALLA OLED
              display.clearDisplay();   //Clear the display    
              display.setCursor(0,0); 
              display.drawBitmap(0, 0, logo_sat, 25, 25, WHITE);    //Draw the second picture (bigger heart)
              display.setTextSize(2);                                //And still displays the average BPM
              display.setTextColor(WHITE);             
              display.setCursor(30,10);                
              display.println("S:");             
              display.setCursor(50,10); 
              display.println(saturacion);               
              display.setCursor(112,10);                
              display.println("%");
            
              display.drawBitmap(0, 32, logo_temp, 25, 25, WHITE);    //Draw the second picture (bigger heart)
              display.setTextSize(2);                                //And still displays the average BPM
              display.setTextColor(WHITE);             
              display.setCursor(30,42);                
              display.println("T:");             
              display.setCursor(50,42);                
              display.println(temperatura);
              display.display();
              delay(375);
              d++;
              }
 
           
            
            if(sat <92 || tempc >=38){
                 ledcWriteNote(TONE_PWM_CHANNEL, NOTE_D, 5);
                  delay(300);
                  ledcWrite(TONE_PWM_CHANNEL, 0);
                  delay(200);
                  ledcWriteNote(TONE_PWM_CHANNEL, NOTE_D, 5);
                  delay(300);
                  ledcWrite(TONE_PWM_CHANNEL, 0);
                  delay(200);
                  ledcWriteNote(TONE_PWM_CHANNEL, NOTE_D, 5);
                  delay(300);
                  ledcWrite(TONE_PWM_CHANNEL, 0);
                  delay(200);
                  ledcWriteNote(TONE_PWM_CHANNEL, NOTE_D, 5);
                  delay(300);
                  ledcWrite(TONE_PWM_CHANNEL, 0);
                  delay(200);
                  ledcWriteNote(TONE_PWM_CHANNEL, NOTE_D, 5);
                  delay(300);
                  ledcWrite(TONE_PWM_CHANNEL, 0);
                  delay(200);
                  ledcWriteNote(TONE_PWM_CHANNEL, NOTE_D, 5);
                  delay(300);
                  ledcWrite(TONE_PWM_CHANNEL, 0);
                  delay(200);
                   int f=0;
                  while(f<=5){
               ///PANTALLA OLED
                  display.clearDisplay();  
                  display.drawBitmap(47, 0, logo_adver, 32, 27, WHITE);    //Draw the second picture (bigger heart)
                  display.setTextSize(2);                                //And still displays the average BPM
                  display.setTextColor(WHITE);             
                  display.setCursor(0,35);                
                  display.println("ALERTA!!!!");             
                  display.display();
                  delay(375);
                   f++;
                   }
              }
             if(sat <92 && tempc >=38){
                 ledcWriteNote(TONE_PWM_CHANNEL, NOTE_D, 5);
                  delay(300);
                  ledcWrite(TONE_PWM_CHANNEL, 0);
                  delay(200);
                  ledcWriteNote(TONE_PWM_CHANNEL, NOTE_D, 5);
                  delay(300);
                  ledcWrite(TONE_PWM_CHANNEL, 0);
                  delay(200);
                  ledcWriteNote(TONE_PWM_CHANNEL, NOTE_D, 5);
                  delay(300);
                  ledcWrite(TONE_PWM_CHANNEL, 0);
                  delay(200);
                  ledcWriteNote(TONE_PWM_CHANNEL, NOTE_D, 5);
                  delay(300);
                  ledcWrite(TONE_PWM_CHANNEL, 0);
                  delay(200);
                  ledcWriteNote(TONE_PWM_CHANNEL, NOTE_D, 5);
                  delay(300);
                  ledcWrite(TONE_PWM_CHANNEL, 0);
                  delay(200);
                  ledcWriteNote(TONE_PWM_CHANNEL, NOTE_D, 5);
                  delay(300);
                  ledcWrite(TONE_PWM_CHANNEL, 0);
                  delay(200);
                  
                 int f=0;
                  while(f<=5){
               ///PANTALLA OLED
                  display.clearDisplay();   //Clear the display    
                  display.setCursor(0,0); 
                  display.setTextSize(4);                                       
                  display.println("ALERTA!!!!!");               
                  display.display();
                  delay(375);
                   f++;
                   } 
                }
         ///////////
       
        //SERIAL
   
            Serial.print("T Corporal = "); 
            Serial.println(temperatura);
            Serial.print("SpO2=");
            Serial.println(saturacion);
            Serial.print("Alerta=");
            Serial.println(alerta);
            Serial.print("Cedula=");
            Serial.println(cedula);
            
      //BPM y Oxigenación
        }
    
      display.display(); 
      //display.clearDisplay();
      delay(1000);
     }
    }
}
