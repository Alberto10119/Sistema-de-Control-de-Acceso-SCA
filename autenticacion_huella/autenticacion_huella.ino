#include "autenticacion_huella.h"


#define mySSID "SCA"
#define myPASSWORD "proyectoSCA"

void OnSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  
}

void OnRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rostro, incomingData, sizeof(rostro));

}

int getFingerprintIDez() {
  uint8_t p = dedo.getImage();
  if (p != FINGERPRINT_OK)  return -2;

  p = dedo.image2Tz();
  if (p != FINGERPRINT_OK)  return -2;

  p = dedo.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  return dedo.fingerID;
}


void mostrarTextoDinamicoInicial(int fila, String mensaje, int tiempoDelay, int columnasLcd) {
  lcd.clear();
  for (int i = 0; i < columnasLcd; i++) {
    mensaje = " " + mensaje;
  }
  mensaje = mensaje + " ";
  for (int pos = 0; ((pos < mensaje.length()) && (g_boton != '#') && (g_boton != '*')); pos++) {
    g_boton = teclado.getKey();
    lcd.setCursor(0, fila);
    lcd.print(mensaje.substring(pos, pos + columnasLcd));
    delay(tiempoDelay);
  }

}

void mostrarTextoDinamicoBluetooth(int fila, String mensaje, int tiempoDelay, int columnasLcd) {

  for (int i = 0; i < columnasLcd; i++) {
    mensaje = " " + mensaje;
  }
  mensaje = mensaje + " ";
  for (int pos = 0; ((pos < mensaje.length()) && (!SerialBT.hasClient())); pos++) { 

    lcd.setCursor(0, fila);
    lcd.print(mensaje.substring(pos, pos + columnasLcd));
    delay(tiempoDelay);
  }
  SerialBT.println("Escriba # para solicitar contraseña");

}


void mostrarTextoEstatico(String mensaje, uint8_t columna, uint8_t fila) {

  lcd.clear();
  lcd.setCursor(columna, fila);
  lcd.print(mensaje);

}

String generarContrasenia() {

  int i = 0;
  int randomizador = 0;


  char numeros[] = "0123456789";
  char letras[] = "ABCD";
  char simbolos[] = "*#";

  String contrasenia;

  randomizador = random(1, 4);
  for (i = 0; i < N; i++) {

    if (randomizador == 1) {
      contrasenia += numeros[random(0, 10)];


    }
    else if (randomizador == 2) {
      contrasenia += letras[random(0, 4)];


    }
    else {
      contrasenia += simbolos[random(0, 2)];


    }
    randomizador = random(1, 4);
  }
  return contrasenia;
}


bool autenticacionMiembro() {

  rostro.camara = true;
  mostrarTextoEstatico("Coloque el dedo", 1, 1);

  resul.huella = getFingerprintIDez();
  tiempoActual = millis();
  tiempoDeseado = 20000;
  
  while (resul.huella == -2 && (millis() - tiempoActual) < tiempoDeseado) {
    resul.huella = getFingerprintIDez();
  }
  if (resul.huella > 0) {

    resul.activar = true;
    digitalWrite(LEDV, HIGH);

    esp_err_t result = esp_now_send(direccionMAC, (uint8_t *) &resul, sizeof(resul));
    

    resul.activar = false;
    digitalWrite(LEDV, LOW);
    while (rostro.camara == true) {
      mostrarTextoEstatico("Mire a la camara", 1, 1);


    }

    mostrarTextoEstatico("Esperando resultado..", 0, 1);
    delay(2000);
    if (rostro.coincide == RECONOCIDO) {

      return true;
    }

    else if (rostro.coincide == INTRUSO) {

      return false;

    }


  }

  else if (resul.huella == -1) {

    return false;


  }

  else {

      mostrarTextoEstatico("Tiempo agotado..", 2, 1);
      delay(3000);
      return false;
    
  }

}


int intentos = 0;

bool indentificacionInvitado() {
  int contador = 0;
  bool entrar = false;
  bool servidor = true;
  tiempoActual = millis();
  tiempoDeseado = 20000;

  mostrarTextoDinamicoBluetooth(1, "Conectese al servidor bluetooth con su dispositivo movil", 250, columnasLcd);
  lcd.clear();
  while (servidor && (millis() - tiempoActual) < tiempoDeseado) {
    mostrarTextoEstatico("Esperando..", 2, 1);
    if (SerialBT.available()) // Compruebe si recibimos algo de Bluetooth
    {
      int recibido = SerialBT.read(); // Lee lo que recibimos
      if (recibido == 35) { 
        contraseniaGenerada = generarContrasenia();
        SerialBT.println("La contraseña es " + contraseniaGenerada);
        SerialBT.println("Escriba la contraseña en el teclado del sistema");
        mostrarTextoEstatico("Contrasenia:", 1, 1);
        entrar = true;
        servidor = false;
      }

    }

  }

  if (!entrar){
      mostrarTextoEstatico("Tiempo agotado..", 2, 1);
      delay(3000);
      SerialBT.disconnect();
      servidor = false;
      return false;
  }
  
  while (entrar) {

    char boton = teclado.getKey();
    if (boton) {
      lcd.print(boton);
      contrasenia_entrada += boton;
      contador++;
      if (contador == N) {
        if (contraseniaGenerada == contrasenia_entrada) {

          contrasenia_entrada = "";
          contador = 0;
          intentos = 0;
          SerialBT.disconnect();
          entrar = false;

          return true;


        } else {

          intentos++;

          if (intentos == 3){
                    
          contrasenia_entrada = "";
          contador = 0;
          SerialBT.disconnect();
          entrar = false;
          intentos = 0;
          return false;
          
         }

         else{
              contrasenia_entrada = "";
              contador = 0;              
              mostrarTextoEstatico("Contrasenia erronea", 0, 1);
              delay(3000);
              mostrarTextoEstatico("Contrasenia:", 1, 1);
          
          
          }



        }

      }

    }

  }


}

void acceso(bool decision) {
  if (decision) {
    mostrarTextoEstatico("Acceso permitido", 1, 1);
    delay(TIEMPO_ACCESO);
    digitalWrite(LEDV, HIGH);
    digitalWrite(RELE_PIN, LOW);
    delay(TIEMPO_ACCESO);
    digitalWrite(RELE_PIN, HIGH);
    digitalWrite(LEDV, LOW);
    ESP.restart();
  }
  else {
    mostrarTextoEstatico("Acceso denegado", 1, 1);
    digitalWrite(LEDR, HIGH);
    delay(TIEMPO_ACCESO);
    digitalWrite(LEDR, LOW);

  }
  lcd.clear();
}


void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(mySSID, myPASSWORD);


  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(5000);
    ESP.restart();
  }

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  pinMode(LEDV, OUTPUT);
  pinMode(LEDR, OUTPUT);
  pinMode(RELE_PIN, OUTPUT);
  digitalWrite(RELE_PIN, HIGH);

  lcd.begin(columnasLcd, filasLcd);
  SerialBT.begin(nombreDispositivo);

  if (esp_now_init() != ESP_OK) {
    
    return;
  }
  esp_now_register_send_cb(OnSent);


  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, direccionMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    
    return;
  }
  esp_now_register_recv_cb(OnRecv);



  delay(100);
  mySerial.begin(57600, SERIAL_8N1, 16, 17);

  delay(5);


}

void loop() {


    mostrarTextoDinamicoInicial(1, inicio, 250, columnasLcd);
  
    if (g_boton == '*') {
      acceso(autenticacionMiembro());
  
    }
    else if (g_boton == '#') {
      acceso(indentificacionInvitado());
    }
    g_boton = 'N';


}
