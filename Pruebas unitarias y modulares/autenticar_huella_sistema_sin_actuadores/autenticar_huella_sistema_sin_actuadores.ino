#include <Adafruit_Fingerprint.h>
#include <esp_now.h>
#include <WiFi.h>
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include <time.h>


/***************************WIFI***************************/


const char* ssid     = "MiFibra-A045";   //your network SSID
const char* password = "XrbL9knC";   //your network password

/*******************************HUELLA*********************/
HardwareSerial mySerial(1);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

/*************************FIREBASE*********************/


/******************************ESP-NOW******************/

uint8_t broadcastAddress[] = {0xC4, 0x4F, 0x33, 0x18, 0xAE, 0x2D};

typedef struct resultados_huella {
  int huella;
  bool activar;
};

resultados_huella resul;

int g_huella;
bool g_activar;


typedef struct esp_eye {
  int coincide;
  bool camara;
  
};
esp_eye rostro;

int g_coincide;
bool g_camara;


void OnSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nSend message status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sent Successfully" : "Sent Failed");
}

void OnRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rostro, incomingData, sizeof(rostro));
  g_coincide = rostro.coincide;
  g_camara = rostro.camara;
  //Serial.print(rostro.coincide); 

}

int contador=0;

void autenticacionMiembro(){

  //rostro.camara = true;
  g_camara = true;
  
  Serial.println("Coloque el dedo");
  g_huella = getFingerprintIDez();
  while(g_huella == -2){
    g_huella = getFingerprintIDez();
    contador++;
    if(contador == 20){
    Serial.println("Huella no detectada");
    
    contador=0;
    }
  }
  Serial.println(g_huella);
  if(g_huella > 0){
  
    g_activar = true;
    resul.activar = g_activar;
    resul.huella = g_huella;
    
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &resul, sizeof(resul));
  if (result == ESP_OK){
  Serial.println("Mire a camara");
  
  g_activar = false;           

  while(g_camara){   
     
    Serial.println("Esperando a resultado de autenticación");
    delay(2000);
    if(!g_camara){
      
      if(g_coincide == 1){
      
         Serial.println("Autenticacion correcta");
    }

    else if(g_coincide == 2){

         Serial.println("Autenticacion incorrecta");
         
      
      }
    }
    
  }
  
 g_camara= true;
 }

 }
 
 else if(g_huella == -1){
     
     Serial.println("Huella no registrada");
     delay(5000); 
     
 }
   

}

  
void setup()
{
  
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  Serial.begin(9600);
  WiFi.mode(WIFI_AP_STA);

  WiFi.begin(ssid, password);  

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  ///// Init ESP-NOW////////////////////////////////////////////////////////
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
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
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnRecv);
  
  
  delay(100);
  mySerial.begin(57600,SERIAL_8N1,16,17);

  delay(5);

}
bool entra=true;
void loop()           
{
   //if(entra){
    autenticacionMiembro();
   // entra=false;

   //}
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
