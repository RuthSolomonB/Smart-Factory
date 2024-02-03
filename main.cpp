#include "DHT11.h"   // Include library
#include "DS1820.h"  // Include the thermometer library
#include "TextLCD.h" //Include LCD library
#include "ds1307.h"
#include "hcsr04.h" // Include library      /// ULTRASONIC SENSOR CODE
#include "mbed.h"   // Include mbed.h
#include "rtc.h"

HCSR04 usensor(p6, p7);                    // Trig at p21, Echo at p22
TextLCD lcd(p21, p22, p23, p24, p25, p26); // RS, E, D4-D7 LCD DISPLAY
Serial pc(USBTX, USBRX);                   // For serial
// DigitalOut buzzer(p30);

Serial device(p13, p14);
DigitalOut leds(p29);
DigitalOut leds1(p27);

DS1307 my1307(p9, p10);

DS1820 ds1820(p5); // DS1820 is connected to pin p5

float temp = 0;  // Temperature
int tstatus = 0; // Sensor status

float humid1 = 0; // Temperature
int temp_from_bt = 23;

AnalogIn ain(p20); // water level to p20

DigitalOut buzzer_water(p30); // buzzer to p30

DHT11 hsensor(p17); // DHT 11 sensor is connected to pin p16

DigitalOut motorPin(p11); // connect to the transistor base

void test_rw(int test) {
  if (test == 0) // test for pass and fail of the clock
    pc.printf("Last R/W operaion passed!\n\r");
  int b = 0;
 // else 
  //pc.printf("Last R/W operation failed!\n\r");
}

float a;
char degree = 223;
char state = 0;
int junk = 0;
int sec;
int min;
int hours;
int day;
int date;
int month;
int year;

void water_l() {
  a = ain * 100.0f; // find the water level
  // device.putc(a);
  //  pc.printf("%f\r\n ", a);
  if (a >= 4) {       // if water level is > 4
    buzzer_water = 1; // turn the buzzzer on
  } else {
    buzzer_water = 0;
  }
}

void bluetooth() {
  if (device.readable()) { // if the device sent a signal
    state = device.getc(); // get the characters
    switch (state) {
    case '1':
      leds = 1; // interior light is on
      break;
    case '2':
      leds = 0; // interior light is off
      break;
    case '4':
      leds1 = 1; // exterior light is on
      break;
    case '5':
      leds1 = 0; // exterior light is on
      break;
    case 'A':
      temp_from_bt = 23; // set thresholds temperature
      break;
    case 'B':
      temp_from_bt = 24;
      break;
    case 'C':
      temp_from_bt = 25;
      break;
    case 'D':
      temp_from_bt = 26;
      break;
    case 'E':
      temp_from_bt = 27;
      break;
    case 'F':
      temp_from_bt = 28;
      break;
    }
  }
}

void clock101() {
  lcd.cls(); // clear LCD for the new display
             // get the time and date from the device
  test_rw(my1307.gettime(&sec, &min, &hours, &day, &date, &month, &year));

  lcd.locate(0, 0);          // locate the cursor at the first line
  lcd.printf("%.2D", hours); // print time
  lcd.printf(":%.2D", min);
  lcd.printf(":%.2D\n\r", sec);
  if (hours == 17) {
    leds = 0;
    leds1 = 0;
  }

  lcd.locate(0, 1); // locate the cursor at the second line
  if (day == 1)     // give names to number of days
    lcd.printf("Sun", day);
  else if (day == 2)
    lcd.printf("Mon", day);
  else if (day == 3)
    lcd.printf("Tue", day);
  else if (day == 4)
    lcd.printf("Wed", day);
  else if (day == 5)
    lcd.printf("Thu", day);
  else if (day == 6)
    lcd.printf("Fri", day);
  else if (day == 7)
    lcd.printf("Sat", day);

  lcd.printf("/ %.2D", date); // display date
  lcd.printf("/ %.2D", month);
  lcd.printf("/ %.2D", year);
  junk = 0x39;                       // just a junk value do read
  test_rw(my1307.write(0x20, junk)); // this should write the

  junk = 0;                          // clear junk to show that
  test_rw(my1307.read(0x20, &junk)); // this should read register
}

void temp_after_stat(int ts) {
  switch (ts) {
  case 0: // no errors -> 'temp' contains temperature
    lcd.locate(0, 0);
    lcd.printf("Temp=:%2.1f%cC\n", temp, degree);

    if (temp >= temp_from_bt) {
      motorPin = 1; // turn motor/fan on
    } else
      motorPin = 0; // turn motor/fan off
    break;
  case 1: // no sensor present
          // pc.printf("No sensor present\n\r");
    break;
  case 2: // CRC error
          // pc.printf("CRC error\r\n");
    break;
  }
}

int main() {
  device.baud(9600); // Set the baud rate to 9600
  ds1820.begin();

  while (1) {
    // get the time
    test_rw(my1307.gettime(&sec, &min, &hours, &day, &date, &month, &year));

    int h_status; // status of DHT 11 sensor
    float humid = hsensor.readHumidity();
    h_status = hsensor.readData(); // Read the status of sensor

    for (int i = 1; i <= 20; i++) {
      water_l();                // check the water level
      bluetooth();              // check bluetooth device
      ds1820.startConversion(); // start conversion
      // tstatus = ds1820.read(temp); // read temperature

      if (i < 10 && i % 2 == 0) {
        clock101(); // access the clock function
      }
      if (i >= 10) {

        if (i == 10) { // temperature reading every 10 seconds
          lcd.cls();
          tstatus = ds1820.read(temp); // read temperature
          humid1 = hsensor.readHumidity();
          humid1 = 0;
        }
        temp_after_stat(tstatus);

        if (h_status != DHT11::OK) { // If not okay
                                     // pc.printf("Device not ready\n");
        } else {
          lcd.locate(0, 1); // locate cursor
          lcd.printf("Hum:%d%%", humid1);
          // pc.printf("Huidmidity: %d %%\r\n", hsensor.readHumidity());
        }
      }
      wait(0.5);
    }
  }
}
