#include "esp_camera.h"
#include "fd_forward.h"

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
  return true;
}

mtmn_config_t mtmn_config = {0};


void setup() {
  Serial.begin(115200);
 
  if (!initCamera()) {
 
    Serial.printf("Failed to initialize camera...");
    return;
  }
 
  mtmn_config = mtmn_init_config();
}
 
void loop() {
   
  camera_fb_t * fb;
  fb = esp_camera_fb_get();
 
  dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
  fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item);
 
  esp_camera_fb_return(fb);
 
  box_array_t *boxes = face_detect(image_matrix, &mtmn_config);
 
  if (boxes != NULL) {
   
    Serial.println("Rostro detectado");
 
    dl_lib_free(boxes->score);
    dl_lib_free(boxes->box);
    dl_lib_free(boxes->landmark);
    dl_lib_free(boxes);
  }
  else{

    Serial.println("Rostro NO detectado");
  }
 
  dl_matrix3du_free(image_matrix);
 
}

