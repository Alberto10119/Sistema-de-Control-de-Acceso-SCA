#include <Adafruit_Fingerprint.h>
#include <esp_now.h>
#include <WiFi.h>
HardwareSerial mySerial(1);


Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
#define LEDV 15
#define LEDR 5
#define ACCESS_DELAY    3000 // Keep lock unlocked for 3 seconds 

const char* ssid     = "MiFibra-A045";   //your network SSID
const char* password = "XrbL9knC";   //your network password

//const char* ssid     = "MiFibra-3747";   //your network SSID
//const char* password = "MaJptK39";   //your network password

uint8_t broadcastAddress[] = {0xC4, 0x4F, 0x33, 0x18, 0xAE, 0x2D};

typedef struct esp_eye {
  bool camara;
  uint16_t id;
};
// Create a struct_message called myData
esp_eye enrollData;

typedef struct sensor_huella {
  bool camara;
  int completo;
};
sensor_huella resRostro;


//bool resulFinal;

void OnSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nSend message status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sent Successfully" : "Sent Failed");
}
void OnRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&resRostro, incomingData, sizeof(resRostro));
  //resulFinal = resRostro.camara;
  //Serial.print("Res: ");
  //Serial.print(resRostro.completo);

}


void setup()
{

  //pinMode(LEDV,OUTPUT);
  //pinMode(LEDR,OUTPUT);
  Serial.begin(9600);
  WiFi.mode(WIFI_AP_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("There was an error initializing ESP-NOW");
    return;
  }

 else{
    Serial.println("Inicializado correctamente");
  
 }
  // We will register the callback function to respond to the event
  esp_now_register_send_cb(OnSent);
  
  // Register the slave
  esp_now_peer_info_t slaveInfo;
  memcpy(slaveInfo.peer_addr, broadcastAddress, 6);
  slaveInfo.channel = 0;  
  slaveInfo.encrypt = false;
  
  // Add slave        
  if (esp_now_add_peer(&slaveInfo) != ESP_OK){
    Serial.println("There was an error registering the slave");
    return;
  }
 else{
  //Serial.println("Placa registrada correctamente");
 }

  esp_now_register_recv_cb(OnRecv);

  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  delay(100);
  //Serial.println("\n\nAdafruit finger detect test");
  mySerial.begin(57600, SERIAL_8N1, 16, 17);

  // set the data rate for the sensor serial port
  //finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    //Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) {
      delay(1);
    }
  }


  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print(".");
  }
  else {
    //Serial.println("Waiting for valid finger...");
    //Serial.print("Sensor contains "); Serial.print(finger.templateCount-1); Serial.println(" templates");
  }

}

int huella;
int contador;
//uint8_t idH;
//bool camara;

void loop()               
{ 
  
  resRostro.camara=true;
  enrollData.camara = false;
  resRostro.completo=0;

  huella = getFingerprintIDez();
  if (huella == -2) {
    Serial.println("Huella no registrada");
    Serial.println("Situe el dedo a registrar"); 
    finger.getTemplateCount();  //huella 0 añadida sin querer
    enrollData.id = finger.templateCount;
    
    while(!enrollData.camara){
       enrollData.camara = getFingerprintEnroll();
       //enrollData.camara = true;
      
      if(enrollData.camara){
        
        //enrollData.id = finger.templateCount + 8;
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &enrollData, sizeof(enrollData));
   
          if (result == ESP_OK) {
            Serial.println("The message was sent sucessfully.");
          }
          else {
            Serial.println("There was an error sending the message.");
          }
          delay(2000);
              
     }

   }

    enrollData.camara=false;

   while(resRostro.camara){

    Serial.println("Esperando a resultado de registro de rostro");
    delay(2000);
    if(!resRostro.camara){
      
      if(resRostro.completo == 1){
      
         Serial.println("Registro completo realizado exitosamente");
    }

    else if(resRostro.completo == 2){

         Serial.println("Error al realizar registro");
         deleteFingerprint(enrollData.id);
      
      }
    }
    
    }
    //Serial.println(resRostro.completo);
    resRostro.camara=true;;
  }
    
  
  else if (huella == -1) {
    contador++;
    if(contador == 20){
    contador=0;
    Serial.println("Huella no detectada");
    }
  }
  else {
    Serial.println("El ID de huella es: ");
    Serial.println(huella);
  }
  delay(50);  //don't ned to run this at full speed.

}


int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -2;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}

uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Esperando dedo valido para registrar como #"); Serial.println(enrollData.id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Imagen tomada");
        break;
      case FINGERPRINT_NOFINGER:
        //Serial.println(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Retira el dedo");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  //Serial.print("ID "); Serial.println(enrollData.id);
  p = -1;
  Serial.println("Situa el mismo dedo otra vez");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        //Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        //Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  //Serial.print("Creando modelo para #");  Serial.println(enrollData.id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Hecho");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Las huellas no coinciden");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  //Serial.print("ID "); Serial.println(enrollData.id);
  p = finger.storeModel(enrollData.id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Huella almacenada");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}

uint8_t deleteFingerprint(uint8_t id) {
  uint8_t p = -1;

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
  }

  return p;
}
