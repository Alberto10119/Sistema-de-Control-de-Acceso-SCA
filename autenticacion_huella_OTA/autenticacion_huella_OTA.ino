//#define ESP32_RTOS  // Uncomment this line if you want to use the code with freertos only on the ESP32
// Has to be done before including "OTA.h"

#include "OTA.h"
#include <Adafruit_Fingerprint.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Keypad.h>
#include <LiquidCrystal.h>
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "BluetoothSerial.h"
#include <time.h>
#include <math.h>


#define mySSID "MiFibra-A045"
#define myPASSWORD "XrbL9knC"
#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // four columns

#define LEDV 5
#define LEDR 15
#define ACCESS_DELAY    3000 // Keep lock unlocked for 3 seconds 
#define RELAY_PIN 2
#define N 6 //password lenght

/**************************Teclado***************************/
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pin_rows[ROW_NUM]      = {32, 23, 22, 21}; // GIOP19, GIOP18, GIOP5, GIOP17 connect to the row pins
byte pin_column[COLUMN_NUM] = {19,18,4,0};   // GIOP16, GIOP4, GIOP0, GIOP2 connect to the column pins

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
String inicio= "Pulsa * para HUELLA y # para invitado";


/***************************PANTALLA****************/

int lcdColumns = 20;
int lcdRows = 4;


const int RS = 13, EN = 14, d4 = 27, d5 = 26,d6 = 25 , d7 = 33;
LiquidCrystal lcd(RS,EN,d4, d5, d6, d7);

/**************************BLUETOOTH***********************/

BluetoothSerial SerialBT;
const String deviceName = "Sistema de Control de Acceso";


String passwordF;// change your password here
String input_password;
String generarContrasenia();


/***************************WIFI***************************/

//const char* ssid     = "Prueba";   //your network SSID
//const char* password = "diegochupala";   //your network password

//const char* ssid     = "MiFibra-A045";   //your network SSID
//const char* password = "XrbL9knC";   //your network password

/*******************************HUELLA*********************/
HardwareSerial mySerial(1);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);


/******************************ESP-NOW******************/

uint8_t broadcastAddress[] = {0xC4, 0x4F, 0x33, 0x18, 0xAE, 0x2D};

typedef struct resultados_huella {
  int huella;
  bool activar;
};

resultados_huella resul;


typedef struct esp_eye {
  int coincide;
  bool camara;
  
};
esp_eye rostro;


void OnSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nSend message status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sent Successfully" : "Sent Failed");
}

void OnRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rostro, incomingData, sizeof(rostro));
  //Serial.print(rostro.coincide); 

}

int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -2;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -2;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  return finger.fingerID;
}
void scrollText(int row, String message, int delayTime, int lcdColumns) {
  for (int i=0; i < lcdColumns; i++) {
    message = " " + message;  
  } 
  message = message + " "; 
  for (int pos = 0; pos < message.length(); pos++) {
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);
  }
}

char g_key;

void scrollInitialText(int row, String message, int delayTime, int lcdColumns){
    //lcd.clear();
    for (int i=0; i < lcdColumns; i++) {
    message = " " + message;  
  } 
  message = message + " "; 
  for (int pos = 0; ((pos < message.length())&&(g_key != '#')&&(g_key != '*')); pos++) {
    g_key = keypad.getKey();
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);
  }
  
}

void scrollBluetoothText(int row, String message, int delayTime, int lcdColumns){
    
    for (int i=0; i < lcdColumns; i++) {
    message = " " + message;  
  } 
  message = message + " "; 
  for (int pos = 0; ((pos < message.length())&&(!SerialBT.hasClient())); pos++) { //cambiar la condicion de pos < message.length ¿inecesaria?
  
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);
  }
  SerialBT.println("Escriba # para solicitar contraseña");
  
}

void displayMessage(String msg){
  // set cursor to first column, first row
  lcd.setCursor(0, 0);
  // print scrolling message
  lcd.display();
  scrollText(1, msg, 250, lcdColumns);
  lcd.clear();
  delay(5);  
 
}

void displayStatic(String msg){
  
  lcd.clear();
  lcd.setCursor(0, 0);

  //lcd.display();
 lcd.print(msg);
  
}

String generarContrasenia() {

  int i = 0;
  int randomizer = 0;


  char numbers[] = "0123456789";
  char LETTER[] = "ABCD";
  char symbols[] = "*#";

  String password;

  randomizer = random(1, 4);
  for (i = 0; i < N; i++) {

    if (randomizer == 1) {
      password += numbers[random(0, 10)];


    }
    else if (randomizer == 2) {
      password += LETTER[random(0, 4)];


    }
    else {
      password += symbols[random(0, 2)];


    }
    randomizer = random(1, 4);
  }
  return password;
}


bool autenticacionMiembro(){

  rostro.camara = true;
  displayStatic("Coloque el dedo");
  
  resul.huella = getFingerprintIDez();
  while(resul.huella == -2){
    resul.huella = getFingerprintIDez();
  }
  if(resul.huella > 0){
  
    resul.activar = true;
    
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &resul, sizeof(resul));
   
  displayStatic("Mire a cámara");
  
  resul.activar = false;           

  while(rostro.camara){   
     
    //displayStatic("Esperando resultado");
   
    delay(2000);
    if(!rostro.camara){
      
      if(rostro.coincide == 1){
      
      return true;
    }

    else if(rostro.coincide == 2){
        
      //displayStatic("Acceso denegado");

        return false;
         
      
      }
    }
    
  }
  
 rostro.camara= true;

 }

 else if(resul.huella == -1){
     
  return false;
    
     
 }
   

}

unsigned long tiempo=0;

bool indentificacionInvitado(){
 int contador = 0;
 bool entrar=false;
 bool server=true;
  
  
  scrollBluetoothText(1, "Conectese al servidor blueetooth con su dispositivo movil", 250, lcdColumns);
  lcd.clear();
  while(server){
   //tiempo = millis();  
    
  if(SerialBT.available()) // Compruebe si recibimos algo de Bluetooth
  {
    int incoming = SerialBT.read(); // Lee lo que recibimos
    if (incoming == 35) { // # en ASCII  42 para *
      passwordF = generarContrasenia();
      SerialBT.println("La contraseña es " + passwordF);
      SerialBT.println("Escriba la contraseña en el teclado del sistema");
      //tiempo = 0;
      entrar=true;
      server=false;
    }

  }
 
  /*if(tiempo > 40000){
    server=false;
    tiempo=0;
    return false;
  }*/
  }
      lcd.clear();
      while(entrar){ 
      char key = keypad.getKey();  
        if (key) {
          lcd.print(key);
          input_password += key;
          contador++;
          if (contador == N) {
            if (passwordF == input_password) {
              
              passwordF = generarContrasenia();
            input_password = "";
            contador = 0;
            SerialBT.disconnect();// se desconecta de cualquier dispositivo? hacer pruebas con varios dispositivos
            entrar=false;

              return true;

              
            } else {

              
            input_password = "";
            contador = 0;
            SerialBT.disconnect();// se desconecta de cualquier dispositivo? hacer pruebas con varios dispositivos
            entrar=false;
            return false;
             
            }

          }
          
        }

    }
  
                                        
}

void acceso(bool decision){
  if(decision){
    displayStatic("Acceso permitido");
    digitalWrite(LEDV, HIGH);
    digitalWrite(RELAY_PIN, LOW);
    delay(ACCESS_DELAY);
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(LEDV, LOW);
  }
  else{
     displayStatic("Acceso denegado");
     digitalWrite(LEDR, HIGH);
     delay(ACCESS_DELAY);
     digitalWrite(LEDR, LOW);
    
  }
  lcd.clear(); 
}


void setup() {
  Serial.begin(115200);
  Serial.println("Booting");

  setupOTA("ESP32 SCA", mySSID, myPASSWORD);

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  pinMode(LEDV,OUTPUT);
  pinMode(LEDR,OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); 
  
  lcd.begin(20,4);
  SerialBT.begin(deviceName);
  
  //Serial.begin(9600);
  WiFi.mode(WIFI_AP_STA);

  WiFi.begin(mySSID, myPASSWORD);  

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  ///// Init ESP-NOW////////////////////////////////////////////////////////
  if (esp_now_init() != ESP_OK) {
    //Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnSent);


  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    //Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnRecv);
  

  
  
  delay(100);
  mySerial.begin(57600,SERIAL_8N1,16,17);

  delay(5);


}

void loop() {
  ArduinoOTA.handle();

  scrollInitialText(1, inicio, 250, lcdColumns);
  
    if(g_key == '*'){
         acceso(autenticacionMiembro());

    }
    else if(g_key == '#'){
      acceso(indentificacionInvitado());
  }
  g_key='N';
  

}
