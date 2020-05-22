#include "TimerOne.h"
#include "U8glib.h"
#define inputCLK 4
#define inputDT 5
//Display
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);

//Variáveis motor
int g_milisecs = 0;
int g_motor = 7;
//int g_led = 3;//Depois ligar um transistor aqui
//Variáveis encoder
int previousStateCLK;
int currentStateCLK;
String encdir ="";
int counter = 1; 
//

  

void setup() {
  //Serial Monitor-----------------------------
   Serial.begin (9600);
  // put your setup code here, to run once:
  Timer1.initialize(1000); // Inicializa o Timer1 e configura para um período de 0,001 segundos
  Timer1.attachInterrupt(contaMilisegundos); // Configura a função callback() como a função para ser chamada a cada interrupção do Timer1
  //Motor de vibração
  pinMode(g_motor, OUTPUT);
  //pinMode(g_led, OUTPUT);
  /// Encoder-----------------------------------
  pinMode (inputCLK,INPUT);
  pinMode (inputDT,INPUT);
  //Lê o estado inicial do inputClk
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
	Estado_BPMemMilis = 0,
	Estado_tempoON,
	Estado_tempoOFF,
};
void maquina_controlaMotor(int BPM)
{
  static float periodo_milis=0;
  static float tempo_on=0;
  static int estado =0;

  switch(estado)
  {
    case Estado_BPMemMilis:
      periodo_milis = 60000/BPM;//Período de uma bpm em milissegundos 
      g_milisecs = 0;//Reseta o timer
      tempo_on = periodo_milis/2;
      estado = Estado_tempoON;
      break;
    case Estado_tempoON:
      if(g_milisecs <= tempo_on)
      {
        digitalWrite(g_motor,HIGH);
        //digitalWrite(g_led,HIGH);
      }
      else
      {
        estado = Estado_tempoOFF;
      }
      break;
    case Estado_tempoOFF:
      if(g_milisecs <= periodo_milis)
      {
        digitalWrite(g_motor,LOW);
        //digitalWrite(g_led,LOW);
      }
      else
      {
        estado = Estado_BPMemMilis;
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
  g_display
}
enum
{
	Estado_getBPM = 0,
	Estado_mudaValorBPM,
};
void loop() {
  static int BPM = 60;
  static int estado = 0;
  static int encoderAtual = 1;
  static int encoderAnterior = 1;
  static int contador = 0;
  static int valor_encoder = 1;
  static bool mexeu = false;
  
  encoderAtual = encoder();
  
  contador++;
  switch(estado)
  {
    case Estado_getBPM:
      if(encoderAtual != encoderAnterior)
      {
        mexeu = true;
        encoderAnterior = encoderAtual;
        contador = 0;
      }
      if(contador >= 15 && mexeu == true)
      {
        valor_encoder = encoderAtual;
        mexeu = false;
        estado = Estado_mudaValorBPM;
      }
      break;
    case Estado_mudaValorBPM:
      BPM = valor_encoder;
      //encoderAnterior = encoderAtual;
      estado = Estado_getBPM;
      break;
    default:
      break;
  }
 
  maquina_controlaMotor(BPM);

  // u8g.firstPage();
  // do
  // {
     draw(BPM);
  // }while(u8g.nextPage());

  Serial.print(encoderAtual);
  Serial.print(" ");
  Serial.print(BPM);
  Serial.print(" ");
  Serial.println(contador);
}
