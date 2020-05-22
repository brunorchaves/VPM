/*Bruno Ribeiro Chaves -> Creater
 * @brief metronome_arduino.c: This is the main firwmare of the vitron metronome
 *
 * @note
 * Copyright(C) Bruno Tecnology
 * All rights reserved.
 *
 *  Authors:
 *  Bruno Ribeiro (https://github.com/brunorchaves)
 */

#include "TimerOne.h"
#include "U8glib.h"
#define inputCLK 4
#define inputDT 5

//**Tip**--> every global variable must be preceeded by a "g"
//Variáveis motor
int g_milisecs = 0;
int g_display = 0;
int g_motor = 7;
//int g_led = 3;//Depois ligar um transistor aqui
//Variáveis encoder
int previousStateCLK;
int currentStateCLK;
String encdir ="";
int counter = 1; 
//
//Display
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);

void setup() {
  //Serial Monitor-----------------------------
   Serial.begin (9600);
  // put your setup code here, to run once:
  Timer1.initialize(1000); // Initialize the Timer1 and configures it to a period of 0,001 secs
  Timer1.attachInterrupt(contaMilisegundos); // Configura the callback() function to be called in every interrupt of the timer
  //Vibrationg motor
  pinMode(g_motor, OUTPUT);
  //pinMode(g_led, OUTPUT);
  /// Encoder-----------------------------------
  pinMode (inputCLK,INPUT);
  pinMode (inputDT,INPUT);

  previousStateCLK = digitalRead(inputCLK);
  //Display---------------------------------------
  if(u8g.getMode() == U8G_MODE_R3G3B2)
  {
    u8g.setColorIndex(255);
  }
  else if(u8g.getMode() == U8G_MODE_GRAY2BIT)
  {
    u8g.setColorIndex(3);
  }
  else if(u8g.getMode() == U8G_MODE_BW)
  {
    u8g.setColorIndex(1);
  }
   else if(u8g.getMode() == U8G_MODE_HICOLOR)
  {
    u8g.setHiColorByRGB(255,255,255);
  }

}
void contaMilisegundos()
{
  g_milisecs++;
  g_display++;
}
enum
{
	State_BPMtoMilis = 0,
	State_timeON,
	State_timeOFF,
};
void machineMotorcontrol(int BPM)
{
  static float period_milis=0;
  static float time_on=0;
  static int state =0;

  switch(state)
  {
    case State_BPMtoMilis:
      period_milis = 60000/BPM;//One BPM perido in seconds
      g_milisecs = 0;//Resets the timer
      time_on = period_milis/2;
      state = State_timeON;
      break;
    case State_timeON:
      if(g_milisecs <= time_on)
      {
        digitalWrite(g_motor,HIGH);
        //digitalWrite(g_led,HIGH);
      }
      else
      {
        state = State_timeOFF;
      }
      break;
    case State_timeOFF:
      if(g_milisecs <= period_milis)
      {
        digitalWrite(g_motor,LOW);
        //digitalWrite(g_led,LOW);
      }
      else
      {
        state = State_BPMtoMilis;
      }
      break;
    default:
      break;
  }
} 
int encoder()
{
  int valor_encoder = 0;
  // Read the current state of inputCLK
   currentStateCLK = digitalRead(inputCLK);
    
   // If the previous and the current state of the inputCLK are different then a pulse has occured
   if (currentStateCLK != previousStateCLK)
   { 
      // If the inputDT state is different than the inputCLK state then 
     // the encoder is rotating counterclockwise
     if (digitalRead(inputDT) != currentStateCLK) 
     { 
       counter --;
       encdir ="CCW";
       //digitalWrite(ledCW, LOW);
       //digitalWrite(ledCCW, HIGH);
       
     } 
     else 
     {
       // Encoder is rotating clockwise
       counter ++;
       encdir ="CW";
       //digitalWrite(ledCW, HIGH);
       //digitalWrite(ledCCW, LOW);
     }
    if(counter >=240)
    {
      counter = 240;
    }
    else if(counter <=1)
    {
      counter = 1;
    }
    //  Serial.print("Direction: ");
    //  Serial.print(encdir);
    //  Serial.print(" -- Value: ");
    //  Serial.println(counter);
   } 
   
   // Update previousStateCLK with the current state
  previousStateCLK = currentStateCLK; 

  return counter;
}
void draw(int BPM)
{

  do 
  {
    u8g.setFont(u8g_font_unifont);
    //u8g.drawStr(0,22,"BPM: ");
    char buf[9];
    sprintf (buf, "%d", BPM);
    u8g.drawStr(18, 3, buf);
    ///u8g.drawStr(1,22,BPM);
  } while( u8g.nextPage() );

}
enum
{
	State_getBPM = 0,
	State_ChangeBPM,
};
void loop() {
  static int BPM = 60;
  static int state = 0;
  static int currentEncoder = 1;
  static int previousEncoder = 1;
  static int counter = 0;
  static int valor_encoder = 1;
  static bool move = false;
  
  currentEncoder = encoder();
  
  counter++;
  switch(state)
  {
    case State_getBPM:
      if(currentEncoder != previousEncoder)
      {
        move = true;
        previousEncoder = currentEncoder;
        counter = 0;
      }
      if(counter >= 15 && move == true)
      {
        valor_encoder = currentEncoder;
        move = false;
        state = State_ChangeBPM;
      }
      break;
    case State_ChangeBPM:
      BPM = valor_encoder;
      //previousEncoder = currentEncoder;
      state = State_getBPM;
      break;
    default:
      break;
  }
 
  machineMotorcontrol(BPM);

  // u8g.firstPage();
  // do
  // {
     draw(BPM);
  // }while(u8g.nextPage());

  Serial.print(currentEncoder);
  Serial.print(" ");
  Serial.print(BPM);
  Serial.print(" ");
  Serial.println(counter);
}
