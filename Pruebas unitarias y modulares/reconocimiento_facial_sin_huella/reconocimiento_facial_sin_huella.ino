
#include "esp_camera.h"
#include "fd_forward.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
//#include "driver/rtc_io.h"
#include <WiFi.h>
//#include <WiFiClient.h>   
#include "ESP32_FTPClient.h"
#include "FirebaseESP32.h"

#define FIREBASE_HOST "https://sca-esp-default-rtdb.europe-west1.firebasedatabase.app/"
#define FIREBASE_AUTH "EdGPgejszNOOoGzoQJMQhFTqul8QRoLLsdVXKtJM"



char* ftp_server = "192.168.1.82";
//char* ftp_server = "192.168.200.73";
char* ftp_user = "AlbertoP";
char* ftp_pass = "servidor12345";
char* ftp_path = "./Compare";


//const char* WIFI_SSID = "MiFibra-3747";
//const char* WIFI_PASS = "MaJptK39";

const char* WIFI_SSID = "MiFibra-A045";
const char* WIFI_PASS = "XrbL9knC";


ESP32_FTPClient ftp (ftp_server,ftp_user,ftp_pass, 5000, 2);

FirebaseData firebaseData;
String path = "/Rostros";


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



void initCamera() {

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
    delay(1000);
    ESP.restart();
  } 
}

mtmn_config_t mtmn_config = {0};

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
 
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  Serial.println("Connecting Wifi...");
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to WiFi..");
      
  }
  Serial.println("IP address: ");
      
  Serial.println(WiFi.localIP());

  initCamera();
  mtmn_config = mtmn_init_config();
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  ftp.OpenConnection();
  ftp.ChangeWorkDir(ftp_path);



}

bool face=false;
int idHuella = 1;
int valido;
int intentos=0;

camera_fb_t * fb;

bool takePhoto() { 
  
  while (!face) {
  
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
    }
   
    dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
    fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item);
    
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
         
      String nombreArchivo = "comparar_"+idN+".jpg";
      Serial.println("Subiendo " + nombreArchivo);
      int str_len = nombreArchivo.length() + 1;

      char char_array[str_len];
      nombreArchivo.toCharArray(char_array, str_len);

      ftp.NewFile(char_array);
      ftp.WriteData( fb->buf, fb->len );
      ftp.CloseFile();

      esp_camera_fb_return(fb);
     
      delay(200);
      
      Firebase.getInt(firebaseData, path + "/id"+idN+"/recog");
      valido = firebaseData.intData();

      while(valido == 0){
        Serial.println("Esperando a resultado");
        Firebase.getInt(firebaseData, path + "/id"+idN+"/recog");
        valido = firebaseData.intData();
      }
      
      if(valido == 1){
        
      Serial.println("El rostro coincide con el registrado");
      }
      else if (valido == 2){ 
        intentos++;
        if(intentos == 3){
          intentos=0;
          Serial.println("El rostro no coincide con NINGUNO de los registrados"); 
  
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
bool activacion=true;

void loop() {
  if(activacion){

   if(takePhoto()){
    activacion = false;
    //face = false;
    
  }
  delay(1000);
  }
}
