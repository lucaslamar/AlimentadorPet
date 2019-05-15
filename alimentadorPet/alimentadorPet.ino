#include <Stepper.h> 
 
const int stepsPerRevolution = 500; 
int data = 0;

//Inicializa a biblioteca utilizando as portas de 8 a 11 para 
//ligacao ao motor 
Stepper myStepper(stepsPerRevolution, 8,10,9,11); 
  
void setup() 
{ 
 //Determina a velocidade inicial do motor 
 myStepper.setSpeed(60);
  Serial.begin(9600);
}
  
void loop()
 
{
   myStepper.step(0);  
  if(Serial.available() > 0)
  {
    data = Serial.read();
    Serial.print(data);
    Serial.print("\n");

    if(data == '1')
    {
      myStepper.step(500);
    }
    else if(data == '0')
    {
         
 
    }

 //Gira o motor no sentido anti-horario a 120 graus
  }
}
