#include <Adafruit_Fingerprint.h>

HardwareSerial mySerial(1);


Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);



void setup()
{
   
  Serial.begin(9600);
  delay(100);
  Serial.println("\n\nReconocimiento de huella dactilar");
  mySerial.begin(57600,SERIAL_8N1,16,17);

  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Sensor de huella detectado");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  finger.getParameters();

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    Serial.println("Esperando un dedo valido...");
      
  }
}

void loop()                     /
{
  
getFingerprintID();
delay(50);            

}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image tomada");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("Dedo no detectado");
      delay(200);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Imagen convertida");
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


  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Encontrada una coincidencia!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("No se encuentra ninguna coincidencia");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }


  Serial.print("Encontrado ID #"); Serial.print(finger.fingerID);
  Serial.print("\n"); 


  return finger.fingerID;
}

