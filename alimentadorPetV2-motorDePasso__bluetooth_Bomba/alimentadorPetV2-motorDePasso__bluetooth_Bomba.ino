// Biibliotecas
#include <Stepper.h> 

// --- Mapeamento de Hardware - portas --- 
const int stepsPerRevolution = 500; // Numero de passos do motor
int data = 0;
#define bomba 7

//Inicializa a biblioteca utilizando as portas de 8 a 11 para  
Stepper myStepper(stepsPerRevolution, 8,10,9,11); 
  
void setup() 
{
  // --- Configurações Iniciais --- 
  pinMode(bomba, OUTPUT);
  digitalWrite(bomba, LOW);   //bomba inicia desligada
  myStepper.setSpeed(60); //Determina a velocidade inicial do motor 
  Serial.begin(9600); // Inicia monitor serial
}
  
void loop()
 
{
     
  if(Serial.available() > 0)
  {
    Serial.print("1: 600");
    Serial.print("2: 800");
    Serial.print("3: 1200");
    Serial.print("4: BOMBA"); 
    
    data = Serial.read();
    Serial.print(data);
    Serial.print("\n");

    if(data == '1')
    {
      myStepper.step(600);
    }
    else if(data == '2')
    {
       myStepper.step(800);
    }

   else if(data == '3')
    {
       myStepper.step(1200);
    }

    else if(data == "4")
    {
      digitalWrite(bomba,HIGH);
      delay(5000);
      digitalWrite(bomba,LOW);
      
    }
    

  }
}
