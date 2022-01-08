#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>  
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include<EEPROM.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>

#include <SoftwareSerial.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial espSerial(48, 49);

const byte rows = 4; 
const byte columns = 4; 
char hexaKeys[rows][columns] = {
{'1','2','3','A'},
{'4','5','6','B'},
{'7','8','9','C'},
{'*','0','#','D'}
};

byte row_pins[rows] = {29,28,27,26};
byte column_pins[columns] = {25,24,23,22};   
Keypad keypad_key = Keypad( makeKeymap(hexaKeys), row_pins, column_pins, rows, columns);
int cursorColumn = 0;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
#define ECHOPIN 4
#define TRIGPIN 5
#define DOUT 3 
#define CLK  2 

const int PulseWire = 0;               
int Threshold = 550;                                         
PulseSensorPlayground pulseSensor; 
int H2,HT,H1;
HX711 scale(DOUT, CLK);
int suhu, myBPM, berat;

float calibration_factor = 115000; 
char data[40];
int s0 = 0;
int b0 = 0;
int t0 = 0;
int d0 = 0;

void setup(){
  lcd.init(); // initialize the lcd
  lcd.backlight();
  
  pinMode(ECHOPIN,INPUT);
  pinMode(TRIGPIN,OUTPUT);
  delay(1000);
  HT = 100;
  
  mlx.begin();
  scale.set_scale();
  scale.tare(); 
  long zero_factor = scale.read_average();
  pulseSensor.analogInput(PulseWire);         
  pulseSensor.setThreshold(Threshold);   
}

void tinggi_badan(){
  digitalWrite(TRIGPIN,LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPIN,HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN,LOW);
  int distance = pulseIn(ECHOPIN,HIGH);
  distance = distance/58;
  H2=HT-distance;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Tinggi: ");
  lcd.print(H2);
  lcd.print("CM");
  sprintf(data, "%d,%d,%d,%d", s0, H2, b0, d0);
  espSerial.println(data);
  delay(1000);
}

void pulse_heart(){
  myBPM = pulseSensor.getBeatsPerMinute();  

  if (pulseSensor.sawStartOfBeat()) {   
    lcd.clear();         
    lcd.setCursor(0,0);
    lcd.print("Detak: ");
    lcd.print(myBPM);               
  }
  sprintf(data, "%d,%d,%d,%d", s0, t0, b0, myBPM);
  espSerial.println(data);
  delay(1000);
}

void suhu_badan(){
  suhu = mlx.readObjectTempC();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Suhu: ");
  lcd.print(suhu);
  lcd.print("C");
  sprintf(data, "%d,%d,%d,%d", suhu, t0, b0, d0);
  espSerial.println(data);
  delay(3000);
}

void berat_badan(){
  scale.set_scale(calibration_factor); //sesuaikan hasil pembacaan dengan nilai kalibrasi
  berat = scale.get_units();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Berat: ");
  lcd.print(berat);
  lcd.print("Kg");
  delay(2000);

  if(Serial.available())
  {
    char temp = Serial.read();
    if(temp == '+' || temp == 'a')
      calibration_factor += 10;
    else if(temp == '-' || temp == 'z')
      calibration_factor -= 10;
  }
  sprintf(data, "%d,%d,%d,%d", s0, t0, berat, d0);
  espSerial.println(data);
  delay(1000);
}

void loop(){
  static char userinput[16];
  static int count = 0;
  char number;
  char key = keypad_key.getKey();
  
  if (key != NO_KEY)
  {
    lcd.print(key);
  }

  switch (key)
  {
    case NO_KEY:
      // nothing to do
      break;

    case '*':
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Masukkan NIK :");
      // clear the current user input
      lcd.setCursor(0, 1);
      memset(userinput, 0, sizeof(userinput));
      number=(userinput, 0, sizeof(userinput));
      // reset the counter
      count = 0;
      break;

    case '#':           //Store number to memory
      EEPROM.write(0, number);
      for (int i = 0; i <= count; i++) {
        lcd.setCursor(i, 2);
        number = userinput[i];
      }
      lcd.clear();
      lcd.print("NIK Tersimpan");
      break;
    
    case 'A':
      suhu_badan();
    break;
    
    case 'B':
      tinggi_badan();
    break;

    case 'C':
      berat_badan();
    break;
    
    case 'D':
      pulse_heart();
    break;
    
    default:
      // if not 10 characters yet
      if (count <=16)
      {
        // add key to userinput array and increment counter
        userinput[count++] = key;
      }
      break;
  }
}
