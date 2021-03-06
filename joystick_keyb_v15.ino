#include <Bounce2.h>
#include <Mouse.h>
#include <Keyboard.h>
#include <Keypad.h>

#include <Arduino.h>
#include <U8x8lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

U8X8_SSD1306_128X32_UNIVISION_SW_I2C u8x8(/* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather ESP8266/32u4 Boards + FeatherWing OLED

//JOYSTICK PINS
#define vertJoyPIN A2
#define horJoyPIN A3
#define buttonJoyPIN 4
//JOYSTICK SETUP
#define analogDeadBand 20
#define sensitivityScroll 18000//è il CONTRARIO della sensitivity (DEFAULT 15000)
#define sensitivityArrow 20000
#define singleScroll 1
//TIMER CLOCK
#define srr 60000 //refresh rate schermo
//KEYPAD MATRIX CHARACTERISTICS
#define ROWS 6
#define COLS 3

// Instantiate a Bounce object
Bounce debouncer = Bounce();

char keys[ROWS][COLS] = {
  {'0', '1', '2'},
  {'3', '4', '5'},
  {'6', '7', '8'},
  {'9', 'A', 'B'},
  {'C', 'D', 'E'},
  {'F', 'G', 'H'}
};

byte rowPins[ROWS] = {5 , 6 , 7 , 8 , 9 , 10}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {15 , 14 , 16}; //connect to the column pinouts of the keypad

int vertPin = 0;  // Analog output of vertical joystick pin
int selPin = 2;  // select button pin of joystick

int tmins = 00;
int thrs = 00;
int tds = 00;

String jState = "  Mouse Wheel";
String testm = "0";

bool joyConf = false;
bool screenUpdate = true;
bool clrScr = false;

int vertZero, horZero; // Stores the initial value of each axis, usually around 512
int vertRawValue, horRawValue;
int scrollPeriod;
int oledScrollPreiod;
unsigned long lastOledTime = 0;
unsigned long lastVertScroll = 0, lastHorScroll = 0;
int mouseClickFlag = 0;

//INIZIALIZE KEYPAD MATRIX

Keypad kpad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);
String msg;

void setup()
{
  pinMode(vertJoyPIN, INPUT);  // set button select pin as input
  pinMode(horJoyPIN, INPUT);  // set button select pin as input
  pinMode(buttonJoyPIN, INPUT_PULLUP);  // set button select pin as input

  // After setting up the button, setup the Bounce instance :
  debouncer.attach(buttonJoyPIN);
  debouncer.interval(5); // interval in ms

  delay(100);  // short delay to let outputs settle
  vertZero = analogRead(vertJoyPIN);  // get the initial values
  horZero = analogRead(horJoyPIN);  // get the initial values

  u8x8.begin();
  u8x8.setPowerSave(0);

  Serial.begin(9600);

  //RESET KEYBOARD
  Keyboard.begin();
  Keyboard.releaseAll();
}
int mi = 0;

void loop()
{
  // Update the Bounce instance :
  debouncer.update();
  //read raw values
  vertRawValue = analogRead(vertJoyPIN);
  horRawValue = analogRead(horJoyPIN);

  if (debouncer.rose())
  {
    joyConf = !joyConf;
    if (joyConf == 0)
    {
      jState = "  Mouse Wheel";
      screenUpdate = true;
    }
    if (joyConf == 1)
    {
      jState = "  Arrow mode ";
      screenUpdate = true;
    }
    Serial.println(joyConf);
  }

  //############################GESTIONE ASSE VERTICALE############################
  if (abs(vertRawValue - vertZero) > analogDeadBand)
  {
    //joystick horizz mosso in modo ponderato
    if (!joyConf)
    {
      scrollPeriod = abs(sensitivityScroll / (vertRawValue - vertZero));
      if ((millis() - lastVertScroll) >= scrollPeriod)                            //#
      {
        if ((vertRawValue - vertZero) > 0)                                        //#
        {
          Mouse.move(0, 0, -singleScroll);
        }
        else
        {
          Mouse.move(0, 0, +singleScroll);
        }
        lastVertScroll = millis();
      }
    }
    else
    {
      //frecce
      scrollPeriod = abs(sensitivityArrow / (vertRawValue - vertZero));
      if ((millis() - lastVertScroll) >= scrollPeriod)                            //#
      {
        if ((vertRawValue - vertZero) > 0)                                        //#
        {
          Keyboard.press(KEY_DOWN_ARROW);
          Keyboard.release(KEY_DOWN_ARROW);
        }
        else
        {
          Keyboard.press(KEY_UP_ARROW);
          Keyboard.release(KEY_UP_ARROW);
        }
        lastVertScroll = millis();
      }
    }

  }

  //##############################################################################

  //############################GESTIONE ASSE ORIZZONTALE#########################
  if (abs(horRawValue - horZero) > analogDeadBand)
  {
    //joystick horizz mosso in modo ponderato
    if (!joyConf)
    {
    }
    else
    {
      scrollPeriod = abs(sensitivityArrow / (horRawValue - horZero));
      if ((millis() - lastHorScroll) >= scrollPeriod)                            //#
      {
        if ((horRawValue - horZero) > 0)                                        //#
        {
          Keyboard.press(KEY_LEFT_ARROW);
          Keyboard.release(KEY_LEFT_ARROW);
        }
        else
        {
          Keyboard.press(KEY_RIGHT_ARROW);
          Keyboard.release(KEY_RIGHT_ARROW);
        }
        lastHorScroll = millis();
      }

    }
  }

  //##############################################################################
  //###########################KEYPAD GESTIONE ###################################

  if (kpad.getKeys())
  {
    for (int i = 0; i < LIST_MAX; i++) // Scan the whole key list.
    {
      if ( kpad.key[i].stateChanged )   // Only find keys that have changed state.
      {
        //###########################ALLOCAZIONE MACRO E TASTI#########################
        char tasto = (char)kpad.key[i].kchar;
        //Serial.println(tasto);

        if (tasto == '0')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              Keyboard.press(KEY_ESC);
              break;
            case RELEASED:
              Keyboard.release(KEY_ESC);
              break;
            case IDLE:
              break;
            case HOLD:
              break;
          }
        }

        if (tasto == '1')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              Keyboard.press(176);
              break;
            case RELEASED:
              Keyboard.release(176);
              break;
            case IDLE:
              break;
            case HOLD:
              break;
          }
        }
        if (tasto == '2')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              Keyboard.press(KEY_DELETE);
              break;
            case RELEASED:
              Keyboard.release(KEY_DELETE);
              break;
            case IDLE:
              break;
            case HOLD:
              break;
          }
        }
        if (tasto == '3')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              Mouse.press(MOUSE_MIDDLE);
              break;
            case RELEASED:
              Mouse.release(MOUSE_MIDDLE);
              break;
            case IDLE:
              break;
            case HOLD:
              break;
          }
        }

        if (tasto == '4')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              Keyboard.press(KEY_LEFT_ALT);
              Keyboard.press('a'); //A MISURA
              Keyboard.releaseAll();
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              break;
          }
        }
        if (tasto == '5')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              Keyboard.press(KEY_LEFT_ALT);
              Keyboard.press('q'); //Q AFFIANCA VERTICALMENTE
              Keyboard.releaseAll();
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              break;
          }
        }
        if (tasto == '6')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press('x'); //X TAGLIA
              Keyboard.releaseAll();
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              break;
          }
        }
        if (tasto == '7')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press('c'); //C COPIA
              Keyboard.releaseAll();
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              break;
          }
        }
        if (tasto == '8')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press('v'); //V INCOLLA
              Keyboard.releaseAll();
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              break;
          }
        }
        if (tasto == '9')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              Keyboard.press(KEY_LEFT_SHIFT);
              break;
            case RELEASED:
              Keyboard.release(KEY_LEFT_SHIFT);
              break;
            case IDLE:
              break;
            case HOLD:
              break;
          }
        }
        if (tasto == 'A')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press('z'); //Z AZIONE PRECEDENTE
              Keyboard.releaseAll();
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              break;
          }
        }
        if (tasto == 'B')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press('y'); //Y AZIONE SUCCESSIVA
              Keyboard.releaseAll();
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              break;
          }
        }
        if (tasto == 'C')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              Keyboard.press(KEY_RIGHT_CTRL);
              delay(50);
              Keyboard.press(KEY_TAB);  //CAMBIA TAB
              delay(50);
              Keyboard.releaseAll();
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              break;
          }
        }
        if (tasto == 'D')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press('s'); //S SALVA
              Keyboard.releaseAll();
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              break;
          }
        }
        if (tasto == 'E')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              Keyboard.press(KEY_LEFT_GUI);
              Keyboard.press(KEY_TAB); //AFFIANCA FINESTRE WINDOWS 10
              Keyboard.releaseAll();
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              break;
          }
        }
        if (tasto == 'F')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              Keyboard.press(KEY_LEFT_CTRL);
              break;
            case RELEASED:
              Keyboard.release(KEY_LEFT_CTRL);
              break;
            case IDLE:
              break;
            case HOLD:
              break;
          }
        }
        if (tasto == 'G')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              Keyboard.press(KEY_LEFT_ALT);
              Keyboard.press('w'); //W ACCOPPIA
              Keyboard.releaseAll();
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              break;
          }
        }
        if (tasto == 'H')
        {
          switch (kpad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press('q'); //Q RICOSTRUISCI TUTTO
              Keyboard.releaseAll();
              break;
            case IDLE:
              break;
            case HOLD:
              break;
            case RELEASED:
              break;
          }
        }
        //#######################################FINE IMPOSTAZIONE#################################
      }

    }
  }

  //###########################STAMPA A SCHERMO###################################

  if (screenUpdate)
  {
    joystatPrt();
  }

  screenUpdate = false;

  if ((millis() - lastOledTime) >= srr)
  {
    if (tmins >= 60)
    {
      thrs = thrs + 1;
      tmins = -1;
      u8x8.clear();
      joystatPrt();
    }

    if (thrs >= 24)
    {
      tds = tds + 1;
      thrs = 0;
      u8x8.clear();
      joystatPrt();
    }
    tmins = tmins + 1;
    testm = "  d" + (String)tds + ":" + "h" + (String)thrs + ":" + "m" + (String)tmins;
    uptimePrt();
    lastOledTime = millis();
  }
}


//####################### DEFINIZIONE FUNZIONI #############################


//####################### FUNZIONE STAMPA A SCHERMO STATO JOYSTICK ########################
void joystatPrt()
{
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0, 0, "JOY Mode:");
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setCursor(0, 1);
  u8x8.print(jState);

}

//####################### FUNZIONE STAMPA A SCHERMO UPTIME ########################
void uptimePrt()
{
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0, 2, "Uptime:");
  u8x8.setCursor(0, 3);
  u8x8.print(testm);
}
