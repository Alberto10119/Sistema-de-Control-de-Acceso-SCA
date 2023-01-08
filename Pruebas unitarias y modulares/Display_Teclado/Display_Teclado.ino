#include <LiquidCrystal.h>
#include <Keypad.h>

#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // four columns

// set the LCD number of columns and rows
int lcdColumns = 20;
int lcdRows = 4;


const int RS = 13, EN = 14, d4 = 27, d5 = 26,d6 = 25 , d7 = 33;
LiquidCrystal lcd(RS,EN,d4, d5, d6, d7);
//Teclado

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pin_rows[ROW_NUM]      = {32, 23, 22, 21}; // GIOP19, GIOP18, GIOP5, GIOP17 connect to the row pins
byte pin_column[COLUMN_NUM] = {19, 18, 4, 0};   // GIOP16, GIOP4, GIOP0, GIOP2 connect to the column pins

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
String pass;
int i =0;
String inicio= "Pulsa # y escribe";

void scrollText(int row, String message, int delayTime, int lcdColumns) {
  for (int i=0; i < lcdColumns; i++) {
    message = " " + message;  
  } 
  message = message + " "; 
  for (int pos = 0; pos < message.length(); pos++) {
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);
  }
}




void setup(){
  Serial.begin(9600);
  lcd.begin(20,4);
  
                     
 
}

bool esc= true;

void loop(){
  // set cursor to first column, first row

  while(esc){
  lcd.setCursor(0,1);
  lcd.print(inicio);
  char key = keypad.getKey();
  if(key){
    if(key == '#'){
      esc=false;
      lcd.clear();
    }
  }
 }
 char key = keypad.getKey();
  if(key){
    if(key == '*'){
      lcd.clear();
      i=0;
    }else{
  pass+=key;
  lcd.setCursor(i,1);
  lcd.print(key);
  i++;  
 }
}
}
  
  
