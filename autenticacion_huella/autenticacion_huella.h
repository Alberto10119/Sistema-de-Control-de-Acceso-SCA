#include <Adafruit_Fingerprint.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Keypad.h>
#include <LiquidCrystal.h>
#include "soc/soc.h"           
#include "soc/rtc_cntl_reg.h"  
#include "BluetoothSerial.h"
#include <time.h>
#include <math.h>


#define RECONOCIDO 1
#define INTRUSO 2
#define NUM_FILAS     4 
#define NUM_COLUMNAS  4 
#define LEDV 5
#define LEDR 15
#define TIEMPO_ACCESO 3   
#define RELE_PIN 2
#define N 6 




/****************************************************/
char botones[NUM_FILAS][NUM_COLUMNAS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pines_fila[NUM_FILAS]      = {32, 23, 22, 21}; 
byte pines_columna[NUM_COLUMNAS] = {19,18,4,0};   

Keypad teclado = Keypad( makeKeymap(botones), pines_fila, pines_columna, NUM_FILAS, NUM_COLUMNAS );
String inicio= "Pulsa * para huella y # para invitado";
char g_boton;


/***************************PANTALLA****************/

int columnasLcd = 20;
int filasLcd = 4;


const int RS = 13, EN = 14, d4 = 27, d5 = 26,d6 = 25 , d7 = 33;
LiquidCrystal lcd(RS,EN,d4, d5, d6, d7);

/**************************BLUETOOTH***********************/

BluetoothSerial SerialBT;
const String nombreDispositivo = "Sistema de Control de Acceso";

String contrasenia_entrada;
String contraseniaGenerada;
unsigned long tiempoActual;
unsigned long tiempoDeseado;

/*******************************HUELLA*********************/
HardwareSerial mySerial(1);
Adafruit_Fingerprint dedo = Adafruit_Fingerprint(&mySerial);


/******************************ESP-NOW******************/

uint8_t direccionMAC[] = {0xC4, 0x4F, 0x33, 0x18, 0xAE, 0x2D};

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


void OnSent(const uint8_t *mac_addr, esp_now_send_status_t status); /*Envia el mensaje al dispositivo ESP32/ESP-EYE*/

void OnRecv(const uint8_t * mac, const uint8_t *incomingData, int len); /*Recibe el mensaje del dispositivo ESP32/ESP-EYE*/

int getFingerprintIDez(); /*Busca si la huella que se ha detectado coincide con alguna registrada*/

void mostrarTextoDinamicoInicial(int fila, String mensaje, int tiempoDelay, int columnasLcd); /* Muestra el texto de inicio de manera dinamica por pantalla*/

void mostrarTextoDinamicoBluetooth(int fila, String mensaje, int tiempoDelay, int columnasLcd); /* Muestra el texto cuando se selecciona el modo invitado de manera dinamica por pantalla*/

void mostrarTextoEstatico(String mensaje,uint8_t columna, uint8_t fila); /* Muestra texto estatico por pantalla*/

String generarContrasenia(); /*Genera una contraseña aletoria teniendo en cuenta los caracteres del teclado del sistema */

bool autenticacionMiembro(); /* Proceso de autenticacion de un usuario registrado mediante el uso de huella dactilar y comunicacion con el ESP-EYE*/

bool indentificacionInvitado(); /* Proceso de identificacion de un usuario invitado, haciendo uso de la comunicacion bluetooth y uso contraseña de un solo uso*/

void acceso(bool decision); /*Funcion principal que se encarga de realizar las llamadas a las funciones autenticacionMiembro() y  indentificacionInvitado()*/
