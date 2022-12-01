/*
* RTC -> ARDUINO
* SDA == A4
* SCL == A5
* VCC == 5V
* GND == GND
*
* LCD -> ARDUINO
* SDA == A4
* SCL == A5
* VCC == 5V
* GND == GND
*
* SDCARD -> ARDUINO
* CS     == D10
* MOSI   == D11
* MISO   == D12
* SCK    == D13
*
* ARDUINO -> WEMOS
* D7      == RX
* D8      == TX
* 3.3V    == 3.3V
* GND     == GND
*
* BUTTON -> ARDUINO
* D2 & D3 Pullup internal
*
* DHT22 -> ARDUINO
* NC    == D4
* DATA  == 10K ke VCC
* VCC   == 5V
* GND   == GND
*
* DHT22 -> ARDUINO
* NC    == D5
* DATA  == 10K ke VCC
* VCC   == 5V
* GND   == GND
*/


#include <SoftwareSerial.h>
#include <SPI.h> // // Sertakan perpustakaan SPI (diperlukan untuk kartu SD)
#include <SD.h> // Sertakan kode perpustakaan SD
//#include <LiquidCrystal.h> // Include LCD library code
#include <LiquidCrystal_I2C.h> // Sertakan kode perpustakaan I2C LCD
#include <Wire.h> // Sertakan kode perpustakaan LCD (needed for I2C protocol devices)
#include <DHT.h>

#define B1 2 // Tombol B1 terhubung ke pin Arduino digitsl 2
#define B2 3  // Tombol B1 terhubung ke pin Arduino 3
#define DHTPIN1 4 // A3 // Pin data DHT22 terhubung ke pin Arduino digital 4
#define DHTPIN2 5  

#define tx 7
#define rx 8
 
#define DHTTYPE DHT22 // menggunakan sensor DHT22
DHT dht1(DHTPIN1, DHTTYPE); // inisialisasi perpustakaan DHT 
DHT dht2(DHTPIN2, DHTTYPE); // inisialisasi perpustakaan DHT 

LiquidCrystal_I2C lcd(0x27, 20, 4); // Tetapkan alamat LCD ke 0x27 untuk tampilan 20 karakter dan 4 baris
SoftwareSerial mySerial(rx, tx); // RX, TX
 
File dataLog;
boolean sd_ok = 0;
char temperature1[] = " 00.0";
char temperature2[] = " 00.0";
char humidity1[]    = " 00.0";
char humidity2[]    = " 00.0";
char times[]        = "  :  :  ";
char Calendar[]     = "  /  /20  ";
byte i, second, minute, hour, date, month, year, previous_second;
int Temp1, RH1, Temp11;
int Temp2, RH2, Temp21;

unsigned long previoustp = 30000;
unsigned long previoustp2 = 0;
unsigned long previoustp3 = 0;
unsigned long nani        = 30000; // jeda waktu antara pembacaan sensor, dalam ms
unsigned long nani2 = 600000; // jeda waktu antara penulisan kartu SD, dalam ms

byte a0, a1, a2, a3;
byte b0, b1, b2, b3, b4;
byte c0, c1, c2, c3;
byte d0, d1, d2, d3, d4;
int ar, br, cr, dr;

 
void setup()
{
 // Buka komunikasi serial dan tunggu port terbuka:
 Serial.begin(9600);
 mySerial.begin(9600);
 Serial.print("Initializing SD card...");
 if (!SD.begin())
 Serial.println("initialization failed!");
 else 
 {
  Serial.println("initialization done.");
  sd_ok = 1;
 }
 pinMode(B1, INPUT_PULLUP);
 pinMode(B2, INPUT_PULLUP);
                          
 Wire.begin(); // Join dalam i2c bus
 lcd.begin(16,2); // inisialisasi LCD
// lcd.init(); // inisialisasi LCD (untuk perpustakaan lain)
 lcd.clear();
 lcd.backlight();
 dht1.begin();
 dht2.begin();
 lcd.setCursor(6, 1);
 lcd.print("Zona 1");
 lcd.setCursor(14, 1);
 lcd.print("Zona 2");  
 lcd.setCursor(0, 2);
 lcd.print("Temp:");
 lcd.setCursor(10, 2);
 lcd.write(223); // Print degree symbol ( °)
 lcd.setCursor(11, 2);  
 lcd.write('C');
 lcd.setCursor(0, 3);   
 lcd.print("RH:        %       %");
 lcd.setCursor(18, 2);  
 lcd.write(223); // Print degree symbol ( °)
 lcd.setCursor(19, 2);  
 lcd.write('C');
  
 Serial.println("   DATE    |   TIME   | TEMP1 [°C] | HUMI1 [%] | TEMP2 [°C] | HUMI2 [%] ");
  
 if(sd_ok) 
 {                           
  // Jika inisialisasi kartu SD OK
  dataLog = SD.open("Logger.txt", FILE_WRITE); // Buka file Logger.txt
  if(dataLog) 
  {                                
   // jika file terbuka dengan benar, tulislah:
   dataLog.println("   DATE    |   TIME   | TEMP1 [°C] | HUMI1 [%] | TEMP2 [°C] | HUMI2 [%] ");
   dataLog.close(); // tutup file
  }
 }
  
 // lcd.noBacklight();
 DS3231_read();
 DS3231_display();
 measurement();
 previoustp = millis();
} // akhiri pengaturan
 
void DS3231_display()
{
 // konversi BCD ke decimal
 second = (second >> 4) * 10 + (second & 0x0F);
 minute = (minute >> 4) * 10 + (minute & 0x0F);
 hour   = (hour >> 4)   * 10 + (hour & 0x0F);
 date   = (date >> 4)   * 10 + (date & 0x0F);
 month  = (month >> 4)  * 10 + (month & 0x0F);
 year   = (year >> 4)   * 10 + (year & 0x0F);
 // akhiri konversi
 times[7]     = second % 10 + 48;
 times[6]     = second / 10 + 48;
 times[4]      = minute % 10 + 48;
 times[3]      = minute / 10 + 48;
 times[1]      = hour   % 10 + 48;
 times[0]      = hour   / 10 + 48;
 Calendar[9] = year   % 10 + 48;
 Calendar[8] = year   / 10 + 48;
 Calendar[4]  = month  % 10 + 48;
 Calendar[3]  = month  / 10 + 48;
 Calendar[1]  = date   % 10 + 48;
 Calendar[0]  = date   / 10 + 48; // Menampilkan waktu
 lcd.setCursor(10, 0);
 lcd.print(Calendar); // Menampilkan kalender
 lcd.setCursor(0, 0);
 lcd.print(times);
 lcd.setCursor(1, 1);
}

void blink_parameter()
{
 byte j = 0;
 while(j < 10 && digitalRead(B1) && digitalRead(B2))
 {
  j++;
  delay(25);
 }
}

byte edit(byte x, byte y, byte parameter)
{
 char text[3];
 while(!digitalRead(B1)); // Tunggu sampai tombol (pin=8) dilepas
 while(true)
 {
  while(!digitalRead(B2)) 
  {
   // Jika tombol (pin=9) ditekan
   parameter++;
   if(i == 0 && parameter > 23) // Jika jam > 23 ==> jam = 0
   parameter = 0;
   if(i == 1 && parameter > 59) // Jika menit > 59 ==> menit = 0
   parameter = 0;
   if(i == 2 && parameter > 31) // Jika tanggal > 31 ==> tanggal = 1
   parameter = 1;
   if(i == 3 && parameter > 12) // Jika bulan > 12 ==> bulan = 1
   parameter = 1;
   if(i == 4 && parameter > 99) // Jika tahun > 99 ==> tahun = 0
   parameter = 0;
   sprintf(text,"%02u", parameter);
   lcd.setCursor(x, y);
   lcd.print(text);
   delay(200); // tunggu 200ms
  }
  lcd.setCursor(x, y);
  lcd.print("  ");                           // Menampilkan dua spasi
  blink_parameter();
  sprintf(text,"%02u", parameter);
  lcd.setCursor(x, y);
  lcd.print(text);
  blink_parameter();

  if(!digitalRead(B1))
  {
   // Jika tombol (pin #8) ditekan
   i++; // Penambahan 'i' untuk parameter selanjutnya
   return parameter; // Mengembalikan nilai parameter dan keluar
  }
 }
}
 
void loop()
{
 if (!digitalRead(B1) or (!digitalRead(B2)))
 {
  lcd.backlight();
  previoustp3 = millis();
 }
 if (millis() - previoustp3 > 5*nani)
 {
  lcd.noBacklight();
 }
  
 if(!digitalRead(B1))
 {                           
  // Jika tombol (pin #8) ditekan
  i = 0;
  hour   = edit(0, 0, hour);
  minute = edit(3, 0, minute);
  date   = edit(10, 0, date);
  month  = edit(13, 0, month);
  year   = edit(18, 0, year);
  // Konversi desimal ke BCD
  minute = ((minute / 10) << 4) + (minute % 10);
  hour = ((hour / 10) << 4) + (hour % 10);
  date = ((date / 10) << 4) + (date % 10);
  month = ((month / 10) << 4) + (month % 10);
  year = ((year / 10) << 4) + (year % 10);
  // Akhiri konversi
  // Tulis data ke DS3231 RTC
  Wire.beginTransmission(0x68); // Mulai protokol I2C dengan alamat DS3231
  Wire.write(0); // Kirim alamat pendaftaran
  Wire.write(0); // Setel ulang sesi dan mulai osilator
  Wire.write(minute); // Menulis menit
  Wire.write(hour); // Menulis jam
  Wire.write(1); // Menulis hari (tidak digunakan)
  Wire.write(date); // Menulis tanggal
  Wire.write(month); // Menulis bulan
  Wire.write(year); // Menulis tahun
  Wire.endTransmission(); // Hentikan transmisi dan lepaskan bus I2C
  delay(200); // tunggu 200ms
 }
 
 DS3231_read(); // Baca tahun dari register 6
 DS3231_display(); // Waktu & kalender diaplay
  
 if(previous_second != second)
 {
  previous_second = second;

  if (millis() - previoustp > nani)
  {
   measurement();
   previoustp = millis();
  }
    
  if (millis() - previoustp2 > nani2)
  {
   sendUpdate();
   previoustp2 = millis();
  }
  if (millis() - previoustp2 > 500)
  lcd.setCursor(0, 1);
  lcd.print(" ");
 }
 delay(50); // tunggu 50ms
} // akhiri loop

void measurement()
{
 RH1 = dht1.readHumidity() * 10; // Baca kelembapan x 10
 Temp1 = dht1.readTemperature() * 10; // Baca suhu dalam derajat Celcius x 10
 RH2 = dht2.readHumidity() * 10; // Baca kelembapan x 10
 Temp2 = dht2.readTemperature() * 10; // Baca suhu dalam derajat Celcius x 10
    
/*
 RH1 = random(0,+1000);  // untuk tes
 Temp1 = random(-300,+400);  // untuk tes
 RH2 = random(0,+1000);  // untuk tes
 Temp2 = random(-300,+400);  // untuk tes
 previoustp = millis();
}
*/
 
 if(Temp1 < 0)
 {
  temperature1[0] = '-'; // Jika suhu < 0 beri tanda minus
  Temp11 = abs(Temp1); // Nilai absolut dari 'Temp'
 }
 else
 {
  temperature1[0] = ' '; // jika tidak (suhu > 0) beri spasi
  Temp11 = abs(Temp1); 
 }
 if (abs(Temp1) >= 100)  
 temperature1[1] = (Temp11 / 100) % 10  + 48;
 else
 {
  temperature1[0] = ' '; 
  if(Temp2 < 0)
  temperature1[1] = '-'; 
  else
  temperature1[1] = ' '; 
 }

 temperature1[2] = (Temp11 / 10)  % 10  + 48;
 temperature1[4] = Temp11 % 10 + 48;

 if(RH1 >= 1000)
 humidity1[0] = '1'; // If Jika kelembapan >= 100,0% masukkan '1' dari ratusan
 else
 humidity1[0] = ' '; // jika tidak (kelembaban < 100) beri spasi
 if(RH1 >= 100)
 humidity1[1] = (RH1 / 100) % 10 + 48;
 else
 humidity1[1] = ' ';      
 humidity1[2] = (RH1 / 10) % 10 + 48;
 humidity1[4] = RH1 % 10 + 48;

 lcd.setCursor(5, 2);
 lcd.print(temperature1);
 lcd.setCursor(5, 3);
 lcd.print(humidity1);
    
 if(Temp2 < 0)
 {
  temperature2[0] = '-'; // Jika suhu < 0 beri tanda minus
  Temp21 = abs(Temp2); // Nilai absolut dari 'Temp'
 }
 else
 {
  temperature2[0] = ' '; // jika tidak (suhu > 0) beri spasi
  Temp21 = Temp2;
 }

 if (abs(Temp2) >= 100)  
 temperature2[1]   = (Temp21 / 100) % 10 + 48;
 else
 {
  temperature2[0] = ' '; 
  if(Temp2 < 0)
  temperature2[1] = '-'; 
  else
  temperature2[1] = ' '; 
 }
 temperature2[2]   = (Temp21 / 10) % 10 + 48;
 temperature2[4] = Temp21 % 10 + 48;
 if(RH2 >= 1000)
 humidity2[0] = '1'; // If Jika kelembapan >= 100,0% masukkan '1' dari ratusan
 else
 humidity2[0] = ' '; // jika tidak (kelembaban <100) beri ruang
 if(RH2 >= 100)
 humidity2[1] = (RH2 / 100) % 10 + 48;    
 else
 humidity2[1] = ' ';    
 humidity2[2] = (RH2 / 10) % 10 + 48;
 humidity2[4] =  RH2 % 10 + 48;
 
 lcd.setCursor(13, 2);
 lcd.print(temperature2);
 lcd.setCursor(13, 3);
 lcd.print(humidity2);
 
 // Kirim data ke monitor serial Arduino IDE
 Serial.print(Calendar);
 Serial.print(" | ");
 Serial.print(times);
 Serial.print(" |   ");
 Serial.print(temperature1);
 Serial.print("    |  ");
 Serial.print(humidity1);
 Serial.print("    |   ");
 Serial.print(temperature2);
 Serial.print("    |  ");
 Serial.println(humidity2);
}

void sendUpdate()
{
 lcd.setCursor(0, 1);
 lcd.print("x");

 if(sd_ok)  
 {                            
  // Jika inisialisasi kartu SD OK
  dataLog = SD.open("Logger.txt", FILE_WRITE); // // Buka file Logger.txt
  if(dataLog)
  {                                   
   // jika file terbuka dengan baik, tulislah:
   dataLog.print(Calendar);
   dataLog.print(" | ");
   dataLog.print(times);
   dataLog.print(" |   ");
   dataLog.print(temperature1);
   dataLog.print("    | ");
   dataLog.print(humidity1);
   dataLog.print("   |   ");
   dataLog.print(temperature2);
   dataLog.print("    | ");
   dataLog.println(humidity2);        
   dataLog.close(); // Tutup file
  }
 }


 if (Temp1 < 0) a0 = 1;
 else
 a0 = 0; 
 a1 = Temp11/100;
 ar = Temp11%100;
 a2 = ar/10;
 a3 = ar%10;
 b1 = RH1/1000;
 br = RH1%1000;
 b2 = br/100;
 br = br%100;
 b3 = br/10;
 b4 = br%10;

 if (Temp2 < 0) c0 = 1;
 else
 c0 = 0; 
 c1 = Temp21/100;
 cr = Temp21%100;
 c2 = cr/10;
 c3 = cr%10;
 d1 = RH2/1000;
 dr = RH2%1000;
 d2 = dr/100;
 dr = dr%100;
 d3 = dr/10;
 d4 = dr%10;

 // KIRIM DATA KE RECIVER....
 mySerial.print('*'); // Memulai karakter
 mySerial.print(a0); 
 mySerial.print(a1); 
 mySerial.print(a2); 
 mySerial.print(a3); 
 mySerial.print(b1); 
 mySerial.print(b2); 
 mySerial.print(b3); 
 mySerial.print(b4);
 mySerial.print(c0); 
 mySerial.print(c1); 
 mySerial.print(c2); 
 mySerial.print(c3); 
 mySerial.print(d1); 
 mySerial.print(d2); 
 mySerial.print(d3); 
 mySerial.print(d4);
 mySerial.print('#'); // Mengakhiri karakter
}

void DS3231_read()
{
 Wire.beginTransmission(0x68); // Mulai protokol I2C dengan alamat DS3231
 Wire.write(0); // Kirim alamat pendaftaran
 Wire.endTransmission(false); // memulai kembali I2C
 Wire.requestFrom(0x68, 7); // Minta 7 byte dari DS3231 dan lepaskan bus I2C di akhir pembacaan
 second = Wire.read(); // Baca detik dari register 0
 minute = Wire.read(); // Baca menit dari register 1
 hour   = Wire.read(); // Baca jam dari register 2
 Wire.read(); // Baca hari dari register 3 (tidak digunakan)
 date   = Wire.read(); // Baca tanggal dari register 4
 month  = Wire.read(); // Baca bulan dari register 5
 year   = Wire.read();     
}
