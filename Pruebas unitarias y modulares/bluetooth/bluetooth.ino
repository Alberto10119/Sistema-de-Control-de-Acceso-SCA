#include "BluetoothSerial.h"
#include <Keypad.h>
#include <time.h>
#include <math.h>


#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // three columns
#define N 6 //password lenght

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};


byte pin_rows[ROW_NUM]      = {32, 23, 22, 21}; // GIOP19, GIOP18, GIOP5, GIOP17 connect to the row pins
byte pin_column[COLUMN_NUM] = {19,18,4,0};   // GIOP16, GIOP4, GIOP0, GIOP2 connect to the column pins

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

BluetoothSerial SerialBT;
const String deviceName = "Sistema de Control de Acceso";


String passwordF;// change your password here
String input_password;
String generarContrasenia();
int contador = 0;

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

void setup() {
  Serial.begin(9600);
  SerialBT.begin(deviceName);
  Serial.println(deviceName + " listo para emparejar..");

}

bool entrar = false;
int cont=0;
int input;
void loop() {

  if (SerialBT.available()) // Compruebe si recibimos algo de Bluetooth
  {
    int incoming = SerialBT.read(); // Lee lo que recibimos //contador por cada incoming? Solo valido si es 1 y #

    if (incoming == 35) { // # en ASCII  42 para *
      passwordF = generarContrasenia();
      SerialBT.println("La contraseña es " + passwordF);
      entrar = true;

    }
  }
  /*int i=SerialBT.available();
  Serial.println(i);*/
  while (entrar) {                                     // Envía el mensaje de texto a través de BT Serial
    char key = keypad.getKey();  // POSIBLES CALLBACK?
    // MODIFICAR PARA QUE EN CASO DE QUE HAYA UN USUARIO CONECRTADO, YA NO SE PUEDA CONECTAR NADIE MAS HASTA QUE EL USUARIO ABANDONDE LA CONEXIÓN. waitForConnect?
    if (key) {
      Serial.println(key);
      input_password += key;
      contador++;
      if (contador == N) {
        if (passwordF == input_password) {
          Serial.println("ACCESO PERMITIDO");
          passwordF = generarContrasenia();


        } else {
          Serial.println("ACCESO DENEGADO");


        }
        input_password = "";
        contador = 0;
        entrar = false;
        SerialBT.disconnect();
          

        

      }

    }
  }

}
