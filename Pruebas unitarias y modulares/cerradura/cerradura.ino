#define RELAY_PIN      5
#define ACCESS_DELAY    3000 // Keep lock unlocked for 3 seconds 


void setup(){

  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
 
  digitalWrite(RELAY_PIN, HIGH); 
}

void loop()
{ 
   delay(9000);
   digitalWrite(RELAY_PIN, LOW);
   delay(ACCESS_DELAY);
   digitalWrite(RELAY_PIN, HIGH);   
              //Add some delay before next scan.
}
