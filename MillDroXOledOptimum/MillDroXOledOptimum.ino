
/******************************************************************
/*                                                                *
/*                            MillDRO - X                         *
/*                                                                *
/*          original May 10, 2015; updated 23nov 2025             *
/*                        W.D. Berggren                           *
/*    debouncing trick copied from CuriousScientist               *
/*            Scale factor for Optimum lathe added                *      
/*            30jan26: removed big font 18pt                      *
/*              File: MillDroXOledOptimum                         *
/******************************************************************

// iGaging Scale werkt ook met HBM scale
// Red    3.3V
// White  Clock -> Scale W Pin 2 via 10k and 20k ohm for 5V to 3,3V
// Green  Data  <- Scale G Pin 3
// Black  Gnd

// Display OLED SH1106 128x64
// VDD    R pin 5 V
// GND    B GND
// SCK    G pin A5
// SDA    O pin A4

// Zero switch 
// COM    B Gnd
// NO     Y Pin 9    

/******************************************************************/
// Adafruit oled library: 2 items SH110X and GFX
#include <Adafruit_GFX.h>
#include <Fonts/FreeSans12pt7b.h>
//#include <Fonts/FreeSans18pt7b.h>
#include <Adafruit_SH110X.h>
#define i2c_Address 0x3c  //initialize with the I2C addr 0x3C Typically eBay OLED's
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/******************************************************************/

#define SCF 99.157 // Scale factor tested to be adjusted if required met 100mm eindmaat gemeten
#define DLY 100    // Refresh period (100 ms)
#define SCC 2      // Scale clock (Pin 2)
#define SCD 3      // Scale data  (Pin 3)
#define SWZ 9      // Zero switch (Pin 9)

byte DroIni;                    // DRO init state
long DroCnt;                    // DRO raw counter
long DroOfs;                    // DRO offset (zero)
float DroMea;                   // DRO measurement
float DroKft;                   // DRO scale factor was float
unsigned long buttonTimer = 0;  //debouncing

/******************************************************************/

void setup() {
  display.clearDisplay();
  Serial.begin(9600);
  delay(50);                         // wait for the OLED to power up
  display.begin(i2c_Address, true);  // Address 0x3C default
  display.clearDisplay();

  IniSwz();  // Init Zero switch
  IniScl();  // Init Scale
  IniClc();  // Init Calc
}

void loop() {

  ThrSwz();  // Test Zero switch
  ThrScl();  // Inquire Scale
  ThrClc();  // Calc measure
  //ThrDsp();                   // Display on LED
  showtext();  // Display measurement on Oled
  delay(DLY);
}

/******************************************************************/

void IniClc(void)  // Init Calc
{
  DroIni = 1;
  DroCnt = 0;
  DroOfs = 0;
  DroMea = 0;
  DroKft = SCF;
}

void ThrClc(void)  // Calc measure
{
  if (DroIni) {
    DroIni = 0;
    DroOfs = DroCnt;
  }
  DroMea = FltLng((float)(DroCnt - DroOfs) * DroKft);
}

/******************************************************************/

void showtext(void) {  
//  DroMea=-315.333; (for testing)    // with this block I replaced
  float x = DroMea / 10000;           // the 7 segments display
  display.setTextSize(1);             // with Oled display
  display.setFont(&FreeSans12pt7b);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(5,20);
  display.print("Optimum");
  display.setCursor(20,50);
  //display.setFont(&FreeSans18pt7b);
  display.print(x, 2);  // 2 decimals
  display.display();
  delay(1);
  display.clearDisplay();
}  

/******************************************************************/

void IniSwz(void)  // Init Zero switch
{
  pinMode(SWZ, INPUT_PULLUP);  // original design pin 9 to gnd = reset
 //pinMode(SWZ,INPUT);         // later design debounced by adding NE555 chip
}


void ThrSwz(void)  // Test Zero switch
{
  //if(digitalRead(SWZ))        // if high == switch pressed (with NE555)
  if (!digitalRead(SWZ)) {      // original design pin 9 to gnd; SWZ == 0 = reset

    if (millis() - buttonTimer > 300) { //software debounce
      DroOfs = DroCnt;
      buttonTimer = millis();
    }

  }  

}

/******************************************************************/

void IniScl(void)  // Init Scale
{
  digitalWrite(SCC, LOW);
  pinMode(SCC, OUTPUT);
  pinMode(SCD, INPUT);
}


void ThrScl(void)  // Inquire Scale
{
  int i;
  long v;

  v = 0;
  for (i = 0; i < 20; i++) {
    ClkScl();
    if (BitScl())
      v |= 0x80000000;
    v >>= 1;
    v &= 0x7FFFFFFF;
  }
  v >>= 11;
  ClkScl();
  if (BitScl())
    v |= 0xFFF00000;
  DroCnt = v;
}


void ClkScl(void)  // Clock Scale
{
  digitalWrite(SCC, HIGH);
  delayMicroseconds(100);
  digitalWrite(SCC, LOW);
  delayMicroseconds(25);
}


int BitScl(void)  // Get Scale bit
{
  int i;
  int h, l;

  h = l = 0;
  for (i = 0; i < 5; i++) {
    if (digitalRead(SCD))
      h++;
    else
      l++;
    delayMicroseconds(SCD);
  }
  if (h > l)
    return (1);
  return (0);
}

/******************************************************************/

long FltLng(float v) {
  return ((v < 0) ? (long)(v - 0.5) : (long)(v + 0.5));
}
