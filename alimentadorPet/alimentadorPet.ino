// bibliotecas
#include <Stepper.h> 

// --- Mapeamento de Hardware ---
#define led         13                             //led onboard
#define bomba        7                             //Pino de controle da bomba d'água
  #define echo       3                            //Pino de echo do HC-SR04
#define trig         4                         //Pino de trigger do HC-SR04
const int stepsPerRevolution = 500;

Stepper myStepper(stepsPerRevolution, 8,10,9,11); //Inicializa a biblioteca utilizando as portas de 3 a 6 para  ligacao ao motor 

// --- Constantes ---
#define full_sec     3                           //tempo para encher recipiente (em segundos)
 

// --- Protótipo das Funções Auxiliares ---
float measureDistance();                      //Função para medir, calcular e retornar a distância em cm
void trigPulse();                             //Função que gera o pulso de trigger de 10µs


// --- Variáveis Globais ---
 
boolean water;                                //Verdadeiro para objetos detectador por mais de 2 segundos
boolean control_end;                          //Indicação de processo concluído
float dist_cm;                                //Armazena a distância em centímetros entre o robô e o obstáculo
short counter = 0x00;                         //Contador auxiliar para interrupção
short test_Object = 0x00;                     //Verifica se haverá um objeto de fato
short seconds = 0x00;                         //Tempo de operação da bomba em segundos
int data = 0;


// --- Rotina de Interrupção ---
ISR(TIMER2_OVF_vect)
{
    TCNT2=100;                            //Reinicializa o registrador do Timer2
    counter++;                            //Incrementa counter
    
    
    // -- Base de tempo de 1 segundo --
    if(counter == 0x64)                   //counter igual a 100d?
    {                                     //Sim...
        counter = 0x00;                   //Reinicia
        test_Object++;                    //incrementa variável de teste
        
        if(test_Object == 0x02)           //test_Object igual a 2?
        {                                 //Sim...
            test_Object = 0x00;           //Reinicia
            water       = 0x01;           //seta variável de detecção
                   
        
        } //end if test_Object
        
        
        if(water)                         //variável de detecção de objeto setada?
        {                                 //Sim...
           seconds++;                     //incrementa os segundos
           digitalWrite(bomba, HIGH);     //liga bomba
           
           if(seconds == full_sec)        //atingiu o tempo para encher recipiente?
           {                              //Sim...
               digitalWrite(bomba, LOW);  //desliga bomba
               seconds = 0x00;            //reinicia segundos
               water   = 0x00;            //limpa water
               TIMSK2  = 0x00;            //desabilita interrupção do timer2
               control_end = 0x01;        //seta control_end 
                
           }  //end if seconds
           
           
        } //end if water
    
    
        digitalWrite(10, !digitalRead(10));   //Inverte o estado da saída (teste da base de tempo)
    
    
    } //end if counter
    
    
    
} //end ISR


// --- Configurações Iniciais ---
void setup()
{
  // -- Configuração de Entradas e Saídas --
     pinMode(10,    OUTPUT);
     pinMode(led,   OUTPUT);
     pinMode(bomba, OUTPUT);
     pinMode(trig,  OUTPUT);
     pinMode(echo,   INPUT);
     myStepper.setSpeed(60); //Determina a velocidade inicial do motor 
     Serial.begin(9600);
     
     
/*     
    Cálculo do estouro do Timer2

        Estouro = (256 - TCNT2) x prescaler x ciclo de máquina
       
        Ciclo de máquina = 1/Fosc = 1/16E6 = 62,5ns = 62,5E-9s
       
        Estouro = (256 - 100) x 1024 x 62,5E-9 = 9,98ms 
*/     

  // -- Configuração dos registradores do Atmega 328p --
     TCCR2A = 0x00;   //Timer operando em modo normal
     TCCR2B = 0x07;   //Prescaler 1:1024
     TCNT2  = 100;    //10 ms overflow again
     TIMSK2 = 0x00;   //Interrupção desabilitada inicialmente
     
  // -- Inicialização de variáveis e I/Os --
     digitalWrite(bomba, LOW);   //bomba inicia desligada
     water  = 0x00;              //detecção de objeto inicia falsa
     control_end = 0x00;         //limpa control_end
 
 
} //end setup
 

// --- Loop Infinito ---
void loop()
{
   
  myStepper.step(0); // motor parado
   dist_cm = measureDistance();               //Atualiza distância em centímetros
   
   
   if(dist_cm < 0x06 && !control_end)         //Distância menor que 6cm e control_end limpa?
   {                                          //Sim...
       
       TIMSK2 = 0x01;                         //Habilita interrupção do Timer2
       digitalWrite(led, HIGH);               //liga LED
   
   } //end if
   
   else                                       //Senão...
   {
       
       TIMSK2 = 0x00;                         //Desabilita interrupção do Timer2
       digitalWrite(led, LOW);                //desliga LED
       test_Object = 0x00;                    //Reinicia test_Object
       if(dist_cm > 0xA) control_end = 0x00;  //Limpa control_end
   
   } //end else
   
   delay(65);                                //Leituras a cada 65 ms


   
  if(Serial.available() > 0)
  {
    Serial.print("A: 600");
    Serial.print("B: 800");
    Serial.print("C: 1200"); 
    
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

  }
} //end loop



// --- Desenvolvimento das Funções Auxiliares ---


float measureDistance()                       //Função que retorna a distância em centímetros
{
  float pulse;                                //Armazena o valor de tempo em µs que o pino echo fica em nível alto
        
  trigPulse();                                //Envia pulso de 10µs para o pino de trigger do sensor
  
  pulse = pulseIn(echo, HIGH);                //Mede o tempo em que echo fica em nível alto e armazena na variável pulse
    
  /*
    >>> Cálculo da Conversão de µs para cm:
    
   Velocidade do som = 340 m/s = 34000 cm/s
   
   1 segundo = 1000000 micro segundos
   
      1000000 µs - 34000 cm/s
            X µs - 1 cm
            
                  1E6
            X = ------- = 29.41
                 34000
                 
    Para compensar o ECHO (ida e volta do ultrassom) multiplica-se por 2
    
    X' = 29.41 x 2 = 58.82
 
  */
  
  return (pulse/58.82);                      //Calcula distância em centímetros e retorna o valor
  
  
} //end measureDistante


void trigPulse()                             //Função para gerar o pulso de trigger para o sensor HC-SR04
{
   digitalWrite(trig,HIGH);                  //Saída de trigger em nível alto
   delayMicroseconds(10);                    //Por 10µs ...
   digitalWrite(trig,LOW);                   //Saída de trigger volta a nível baixo

} //end trigPulse
