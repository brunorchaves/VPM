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
#include "SSD1306.h"
#include <AiEsp32RotaryEncoder.h> //Source: https://github.com/igorantolic/ai-esp32-rotary-encoder
//Pins***************
//Device          ESP
//Encoder-->      
//inputCLK        D13
//inputDT         D12
//Motor-->        D2
//Display--> 
//SCL             D22
//SDA             D21

/*
connecting Rotary encoder
CLK (A pin) - to any microcontroler intput pin with interrupt -> in this example pin 32
DT (B pin) - to any microcontroler intput pin with interrupt -> in this example pin 21
SW (button pin) - to any microcontroler intput pin -> in this example pin 25
VCC - to microcontroler VCC (then set ROTARY_ENCODER_VCC_PIN -1) or in this example pin 25
GND - to microcontroler GND
*/
#define ROTARY_ENCODER_A_PIN 32
#define ROTARY_ENCODER_B_PIN 34
#define ROTARY_ENCODER_BUTTON_PIN 25
#define ROTARY_ENCODER_VCC_PIN 27 /*put -1 of Rotary encoder Vcc is connected directly to 3,3V; else you can use declared output pin for powering rotary encoder */
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN);
int16_t g_encoderValue = 0;
int16_t g_encoderRead = 120;
static int BPM = 120;
void rotary_onButtonClick() 
{
  //rotaryEncoder.reset();
  //rotaryEncoder.disable();
  g_encoderValue = rotaryEncoder.readEncoder();
  BPM = g_encoderValue;
}
void rotary_loop() 
{
  //first lets handle rotary encoder button click
  if (rotaryEncoder.currentButtonState() == BUT_RELEASED) {
    //we can process it here or call separate function like:
    rotary_onButtonClick();
  }

  //lets see if anything changed
  int16_t encoderDelta = rotaryEncoder.encoderChanged();
  // g_encoderRead = rotaryEncoder.readEncoder();
  //optionally we can ignore whenever there is no change
  if (encoderDelta == 0) return;
  
  //for some cases we only want to know if value is increased or decreased (typically for menu items)
  //if (encoderDelta>0) Serial.print("+"); //Uncomment 4 debug
  //if (encoderDelta<0) Serial.print("-"); //Uncomment 4 debug

  //for other cases we want to know what is current value. Additionally often we only want if something changed
  //example: when using rotary encoder to set termostat temperature, or sound volume etc
  
  //if value is changed compared to our last read
  if (encoderDelta!=0) {
    //now we need current value
    int16_t encoderValue = rotaryEncoder.readEncoder();
    //process new value. Here is simple output.
    Serial.print("Value: ");
    Serial.print(encoderValue);
    Serial.println(" - Click to select");
    
  }
  
}


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
int timerDivider = 100;
int g_milisecs = 0;
hw_timer_t *timer = NULL;
//portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

//Display------------------------------------------------
//objeto controlador do display de led
/*
  0x3c  :   é um identificador único para comunicação do display
  pino 5 e 4 são os de comunicação (SDA, SDC)   
*/
#define SDA_PIN 21                      // GPIO21 -> SDA
#define SCL_PIN 22                      // GPIO22 -> SCL
#define SSD_ADDRESS 0x3c                // Endereço do display no bus i2c
SSD1306  screen(SSD_ADDRESS, SDA_PIN, SCL_PIN);
 
//utilizado para fazer o contador de porcentagem
int contador;

void IRAM_ATTR onTimer()
{
  g_milisecs ++;
}

void setup() 
{
  //Serial Monitor-----------------------------
  Serial.begin (9600);
  //Timer--------------------------------------
  timer = timerBegin(0,80,true);//Timer 0, MWDT clock period = 12,5 s * TIMGn_Tx_WDT_CLK_PRESCALE -> 12.5ns*80 ->1u sec, countUP 
  timerAttachInterrupt(timer,&onTimer, true); //edge (not level) triggered
  timerAlarmWrite(timer, 1000, true);//1000*1us = 1s , autoreload true;
  timerAlarmEnable(timer);//enable
  //Vibrationg motor
  pinMode(g_motor, OUTPUT);
  //pinMode(g_led, OUTPUT);
  /// Encoder-----------------------------------
    //we must initialize rorary encoder 
  rotaryEncoder.begin();
  rotaryEncoder.setup([]{rotaryEncoder.readEncoder_ISR();});
  //optionally we can set boundaries and if values should cycle or not
  rotaryEncoder.setBoundaries(40, 240, false); //minValue, maxValue, cycle values (when max go to min and vice versa)

  //Display-------------------------------------
  // Inicializa o objeto que controlará o que será exibido na tela
  screen.init();
  //gira o display 180º (deixa de ponta cabeça)
 // display.flipScreenVertically();
  //configura a fonte de escrita "ArialMT_Plain_10"
  screen.setFont(ArialMT_Plain_10);

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

void drawProgressBar() 
{
  
  //Serial.print(">> ");
  //Serial.println(contador);
   g_encoderRead = rotaryEncoder.readEncoder();
  // desenha a progress bar
  /*
   * drawProgressBar(x, y, width, height, value);
    parametros (p):
      p1: x       --> coordenada X no plano cartesiano
      p2: y       --> coordenada Y no plano cartesiano
      p3: width   --> comprimento da barra de progresso
      p4: height  --> altura da barra de progresso
      p5: value   --> valor que a barra de progresso deve assumir
     
  */
  //screen.drawProgressBar(10, 32, 100, 10, BPM);
 
  // configura o alinhamento do texto que será escrito
  //nesse caso alinharemos o texto ao centro
  screen.setTextAlignment(TEXT_ALIGN_CENTER);
  //escreve o texto de porcentagem
  /*
   * drawString(x,y,text);
    parametros (p):
     p1: x      --> coordenada X no plano cartesiano
     p2: y      --> coordenada Y no plano cartesiano
     p3: string --> texto que será exibido
  */
  screen.drawString(64, 15, "BPM");
  screen.drawString(64, 30, String(g_encoderRead));
 
  //se o contador está em zero, escreve a string "valor mínimo"
  // if(contador == 0){
  //   screen.drawString(64, 45, "Valor mínimo");
  // }
  //se o contador está em 100, escreve a string "valor máximo"
  // else if(contador == 100){
  //   screen.drawString(64, 45, "Valor máximo");
  // }
}
enum
{
	State_getBPM = 0,
	State_ChangeBPM,
};
void loop() {
  
  static int state = 0;
  static int currentEncoder = 1;
  static int previousEncoder = 1;
  static int counter = 0;
  static int valor_encoder = 1;
  static bool move = false;
   //in loop call your custom function which will process rotary encoder values
  rotary_loop();
  //currentEncoder = encoder();
  
  counter++;
  // switch(state)
  // {
  //   case State_getBPM:
  //     if(currentEncoder != previousEncoder)
  //     {
  //       move = true;
  //       previousEncoder = currentEncoder;
  //       counter = 0;
  //     }
  //     if(counter >= 15 && move == true)
  //     {
  //       valor_encoder = currentEncoder;
  //       move = false;
  //       state = State_ChangeBPM;
  //     }
  //     break;
  //   case State_ChangeBPM:
  //     BPM = valor_encoder;
  //     //previousEncoder = currentEncoder;
  //     state = State_getBPM;
  //     break;
  //   default:
  //     break;
  // }
  if(--timerDivider == 0)
  {//limpa todo o display, apaga o contúdo da tela
    timerDivider = 100;
    screen.clear();
    
    //  ++counter;
    //  counter > 100 ? counter = 0 : counter = counter;  
  
    //desenha a progress bar
    drawProgressBar();
  
    //exibe na tela o que foi configurado até então.
    screen.display();
  }
  machineMotorcontrol(BPM);

  Serial.print(g_encoderRead);
  Serial.print(" ");
  Serial.print(BPM);
  Serial.print(" ");
  Serial.println(counter);
  if (millis()>20000) 
    rotaryEncoder.enable ();
}
