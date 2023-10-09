#include "esp_camera.h"
#include "fd_forward.h"
#include "soc/soc.h"           
#include "soc/rtc_cntl_reg.h"  
#include "ESP32_FTPClient.h"
#include "FirebaseESP32.h"
#include <esp_now.h>

#define mySSID "SCA"
#define myPASSWORD "proyectoSCA"
#define FIREBASE_HOST "https://sca-esp-default-rtdb.europe-west1.firebasedatabase.app/"
#define FIREBASE_AUTH "EdGPgejszNOOoGzoQJMQhFTqul8QRoLLsdVXKtJM"
#define MODO_STANDBY 0
#define RECONOCIDO 1
#define INTRUSO 2


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


FirebaseData firebaseData;
String path = "/Rostros";

char* servidor_ftp = "192.168.47.73";
char* usuario_ftp = "AlbertoP";
char* contrasenia_ftp = "servidor12345";
char* path_ftp = "./Comparar";

bool resultado_rostro=false;
camera_fb_t * fb;
int valido;
int intentos=0;

bool activacion;
int idHuella;

ESP32_FTPClient ftp (servidor_ftp,usuario_ftp,contrasenia_ftp, 5000, 2);

uint8_t direccionMAC[] = {0xC8, 0xC9, 0xA3, 0xC7, 0x17, 0x98};



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



void OnRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&resul, incomingData, sizeof(resul));
  idHuella = resul.huella;
  activacion = resul.activar;
}

void OnSent(const uint8_t *mac_addr, esp_now_send_status_t status) {

}


void inicializacionCamara() { /* Inicializa todos los pines y parametros de la camara (resolucion, orientacion ..)*/

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
  config.frame_size = FRAMESIZE_VGA;  
  config.jpeg_quality = 10;
  config.fb_count = 2;

  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);

  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    delay(1000);
    ESP.restart();
  }

   sensor_t * s = esp_camera_sensor_get();
   s->set_vflip(s, 1);
}

mtmn_config_t mtmn_config = {0};


void setup() {
  Serial.begin(115200);
  
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(mySSID, myPASSWORD);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(5000);
    ESP.restart();
  }
  
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 


  inicializacionCamara();

  mtmn_config = mtmn_init_config();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  ftp.OpenConnection();
  ftp.ChangeWorkDir(path_ftp);

  if (esp_now_init() != ESP_OK) {
    return;
  }
  esp_now_register_send_cb(OnSent);

  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, direccionMAC, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
      
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    return;
  }
  esp_now_register_recv_cb(OnRecv);


}


bool tomarFoto() { /* Toma una foto y al sitio FTP para el reconocimiento facial y a Firebase para su registro. Esta funcion se activa cuando se recibe el mensaje de activacion por protocolo ESP-NOW*/

  delay(2000);
  while (!resultado_rostro) {

    delay(2000);
    fb = esp_camera_fb_get();
      
    String idN = (String) idHuella;
      
    ftp.InitFile("Tipo I");
       
    String nombreArchivo = "comparar_"+idN+".jpg";
    int str_len = nombreArchivo.length() + 1;

    char char_array[str_len];
    nombreArchivo.toCharArray(char_array, str_len);

    ftp.NewFile(char_array);
    ftp.WriteData( fb->buf, fb->len );
    ftp.CloseFile();

    esp_camera_fb_return(fb);
    
    Firebase.getInt(firebaseData, path + "/id"+idN+"/recog");
    valido = firebaseData.intData();

    while(valido == MODO_STANDBY){
      Firebase.getInt(firebaseData, path + "/id"+idN+"/recog");
      valido = firebaseData.intData();
    }
    
    if(valido == RECONOCIDO){
      resultado_rostro=true;  
    
    }
    else { 
      
      intentos++;
      if(intentos == 3){
        intentos=0;
        resultado_rostro=true;

      }

      
     }
      Firebase.setInt(firebaseData, path + "/id"+idN+"/recog",MODO_STANDBY);

  }
    

   return resultado_rostro;
  
}

void loop() {
 
  if(activacion){

    if(tomarFoto()){
      activacion = false;
      resultado_rostro = false;
      rostro.camara = false;
      rostro.coincide = valido;
     
      esp_err_t result = esp_now_send(direccionMAC, (uint8_t *) &rostro, sizeof(rostro));
      
    }
    
  }

}
