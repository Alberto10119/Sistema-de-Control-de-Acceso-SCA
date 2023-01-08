#include <WiFi.h>
#include <base64.h>

#include "FirebaseESP32.h"
#include "ESP32_FTPClient.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Base64.h"
#include "fd_forward.h"
#include <esp_now.h>
#include "esp_camera.h"

#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    4
#define SIOD_GPIO_NUM    18
#define SIOC_GPIO_NUM    23

#define Y9_GPIO_NUM      36
#define Y8_GPIO_NUM      37
#define Y7_GPIO_NUM      38
#define Y6_GPIO_NUM      39
#define Y5_GPIO_NUM      35
#define Y4_GPIO_NUM      14
#define Y3_GPIO_NUM      13
#define Y2_GPIO_NUM      34
#define VSYNC_GPIO_NUM   5
#define HREF_GPIO_NUM    27
#define PCLK_GPIO_NUM    25



const char* ssid     = "MiFibra-A045";   //your network SSID
const char* password = "XrbL9knC";   //your network password


//const char* ssid     = "MiFibra-3747";   //your network SSID
//const char* password = "MaJptK39";   //your network password

// Credenciales Proyecto Firebase
#define FIREBASE_HOST "https://sca-esp-default-rtdb.europe-west1.firebasedatabase.app/"
#define FIREBASE_AUTH "EdGPgejszNOOoGzoQJMQhFTqul8QRoLLsdVXKtJM"

//MAC del esp32  que no entra en la pcb
uint8_t broadcastAddress[] = {0xC8, 0xC9, 0xA3, 0xC8, 0xFF, 0xC4};

// Firebase Data object
FirebaseData firebaseData;

// Si deseamos una ruta especifica
String path = "/Rostros";

char* ftp_server = "192.168.1.82";
//char* ftp_server = "192.168.200.73";
char* ftp_user = "AlbertoP";
char* ftp_pass = "servidor12345";
char* ftp_path = ".";


ESP32_FTPClient ftp (ftp_server, ftp_user, ftp_pass, 5000, 2);

uint16_t idHuella;
bool activar;

int resCompleto;

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




void OnRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&enrollData, incomingData, sizeof(enrollData));

  Serial.print("ID: ");
  Serial.println(enrollData.id);
  Serial.print("Camara: ");
  Serial.println(enrollData.camara);
  idHuella = enrollData.id;
  activar = enrollData.camara;
}

void OnSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nSend message status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sent Successfully" : "Sent Failed");
}


bool face = false;

bool initCamera(){
  
  camera_config_t config;
  
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 34;
  config.pin_d1 = 13;
  config.pin_d2 = 14;
  config.pin_d3 = 35;
  config.pin_d4 = 39;
  config.pin_d5 = 38;
  config.pin_d6 = 37;
  config.pin_d7 = 36;
  config.pin_xclk = 4;
  config.pin_pclk = 25;
  config.pin_vsync = 5;
  config.pin_href = 27;
  config.pin_sscb_sda = 18;
  config.pin_sscb_scl = 23;
  config.pin_pwdn = -1;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA;  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
  config.jpeg_quality = 10;
  config.fb_count = 2;

  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return false;
  }

  sensor_t * s = esp_camera_sensor_get();
  s->set_vflip(s, 1);
  
  return true;
}


mtmn_config_t mtmn_config = {0};
int detections = 0;

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  Serial.begin(115200);
  //delay(10);
  
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
    Serial.println("Placa registrada correctamente");
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

  Serial.println("");
  Serial.println("STAIP address: ");
  Serial.println(WiFi.localIP());
    
  Serial.println("");

  
  if (!initCamera()) {
 
    Serial.printf("Failed to initialize camera...");
    return;
  }
 
  mtmn_config = mtmn_init_config();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  ftp.OpenConnection();
  ftp.ChangeWorkDir(ftp_path);

  
}



//bool act=true;
int intentos = 0;
void loop() {
  if (activar) {
    
    if (enrollFace()) {
      activar = false;
      face = false;

      resRostro.camara = false;
      resRostro.completo = resCompleto;
      //delay(500);
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &resRostro, sizeof(resRostro));
     if (result == ESP_OK) {
          Serial.println("The message was sent sucessfully.");
        }
        else {
          Serial.println("There was an error sending the message.");
        }
   }     
   
   delay(500);
  }
  
}
int enrolled;
camera_fb_t * fb;

bool enrollFace() {

  while (!face) {
    

    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
    }
   
    dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
    uint32_t res= fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item);
    
    box_array_t *net_boxes = face_detect(image_matrix, &mtmn_config);

    if (net_boxes){
       face = true;
      dl_lib_free(net_boxes->box);
      dl_lib_free(net_boxes->landmark);
      dl_lib_free(net_boxes);
      
    }else{
        Serial.println("Face Not Detected");
        esp_camera_fb_return(fb);
      }
   
    if (face) {
      
      String idN = (String) idHuella;
      
      ftp.InitFile("Type I");
         
      String nombreArchivo = "id"+idN+".jpg";
      Serial.println("Subiendo " + nombreArchivo);
      int str_len = nombreArchivo.length() + 1;

      char char_array[str_len];
      nombreArchivo.toCharArray(char_array, str_len);

      ftp.NewFile(char_array);
      ftp.WriteData( fb->buf, fb->len );
      ftp.CloseFile();

      Firebase.setInt(firebaseData, path + "/id"+idN+"/id", idHuella);
      Firebase.setInt(firebaseData, path + "/id"+idN+"/recog", 4);

   
      String imageFile = base64::encode(fb->buf,fb->len);

      //Serial.println(imageFile);
      esp_camera_fb_return(fb);
      
      
      //delay(20000);
      
      Firebase.getInt(firebaseData, path + "/id"+idN+"/recog");
      enrolled = firebaseData.intData();

      while(enrolled == 4){
        Serial.println("Esperando a registro de rostro aceptable");
        Firebase.getInt(firebaseData, path + "/id"+idN+"/recog");
        enrolled = firebaseData.intData();
      }
      
      
      if(enrolled == 0){

      Firebase.setString(firebaseData, path + "/id"+idN+"/Image2", imageFile);
      resCompleto = 1;
      Serial.println("Rostro registrado con exito");
      }
      else { 
        
        intentos++;
        if(intentos == 3){
          intentos=0;
          resCompleto = 2;
        }
        
        else{
             face=false;  
             Serial.println("Vuelva a mirar a camara");
         
        }
        
       }     
    }
    
    dl_matrix3du_free(image_matrix);
    delay(200);
  }
   
   return face;
}
