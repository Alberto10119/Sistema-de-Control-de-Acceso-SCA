#include <LiquidCrystal.h>

// set the LCD number of columns and rows
int lcdColumns = 20;
int lcdRows = 4;


const int RS = 13, EN = 14, d4 = 27, d5 = 26 ,d6 = 25 , d7 = 33;
LiquidCrystal lcd(RS,EN,d4, d5, d6, d7);


void setup() {
  // set up the LCD's number of columns and rows:
  Serial.begin(9600);
  lcd.begin(20, 4);
  // Print a message to the LCD.
  lcd.print("Hola mundo");
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  
  delay(5000);
  // print the number of seconds since reset:
  //lcd.print(millis() / 1000);
}
