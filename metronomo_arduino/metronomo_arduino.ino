/*Bruno Ribeiro Chaves -> Creator
 * @brief metronome_arduino.c: This is the main firwmare of the vitron metronome
 *
 * @note
 * Copyright(C) Bruno Tecnology
 * All rights reserved.
 *
 *  Authors:
 *  Bruno Ribeiro (https://github.com/brunorchaves)
 */

#define inputCLK 13
#define inputDT 12
//Pins***************
//Device          ESP
//Encoder-->      
//inputCLK        D13
//inputDT         D12
//Motor-->        D2
//Display--> 
//SCL             D22
//SDA             D21

//**Tip**--> every global variable must be preceeded by a "g"
//Motor Variables

int g_display = 0;
int g_motor = 5;
//int g_led = 3;//Depois ligar um transistor aqui
//Variáveis encoder
int previousStateCLK;
int currentStateCLK;
int counter = 1; 
//Timer
int timerDivider = 1000;
int g_milisecs = 0;
hw_timer_t *timer = NULL;
//portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer()
{
  g_milisecs ++;
  
}

void setup() 
{
  //Serial Monitor-----------------------------
  Serial.begin (9600);
  //Timer
  timer = timerBegin(0,80,true);//Timer 0, MWDT clock period = 12,5 s * TIMGn_Tx_WDT_CLK_PRESCALE -> 12.5ns*80 ->1u sec, countUP 
  timerAttachInterrupt(timer,&onTimer, true); //edge (not level) triggered
  timerAlarmWrite(timer, 1000, true);//1000*1us = 1s , autoreload true;
  timerAlarmEnable(timer);//enable
  //Vibrationg motor
  pinMode(g_motor, OUTPUT);
  //pinMode(g_led, OUTPUT);
  /// Encoder-----------------------------------
  pinMode (inputCLK,INPUT);
  pinMode (inputDT,INPUT);
  previousStateCLK = digitalRead(inputCLK);

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
       //encdir ="CCW";
       //digitalWrite(ledCW, LOW);
       //digitalWrite(ledCCW, HIGH);
       
     } 
     else 
     {
       // Encoder is rotating clockwise
       counter ++;
      // encdir ="CW";
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

  Serial.print(currentEncoder);
  Serial.print(" ");
  Serial.print(BPM);
  Serial.print(" ");
  Serial.println(counter);
}


