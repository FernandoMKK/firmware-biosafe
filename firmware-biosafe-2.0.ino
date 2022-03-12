 /*  ===========================Firmware BioSafe 2.0===========================
   Descrição: Firmware criado para controlar a máquina BioSafe, na sua segunda
   versão, no semestre de 2021-2. O código controla sistemas de aquecimento,
   de rotação e de luminosisdade, além de realizar leituras de fim de curso,
   fazendo a segurança do equipamento. Também é capaz de receber comandos via
   HTTP, permitindo a integração com um aplicativo.

   Notas:
      * O programa roda em multi core. O controlador possui 2 núcleos (0 e 1).

      * Por padrão, a função loop() roda no core 1. Portanto, todo código 
        dentro dessa função, está rodando no core 1.

      * O controle de temperatura é executado no Core 1.
      
      * A verificação do aperto do botão e todo o processo é executado no
        core 0.

      * O compartilhamento de memória entre os núcleos se dá de forma nativa,
        não necessitando de nenhuma ação de invoke.

   Autor: Fernando Marcon Kirchner
   Última modificação: 02/11/2021
*/

/* ===================================================Inclusão de Libs=================================================== */
//Bibliotecas feitas
#include "LED.h"
#include "EndSwitch.h"
#include "Relay.h"

//Bibliotecas prontas
#include <Wire.h>                 //Comunicação I2C
#include <LiquidCrystal_I2C.h>    //Display LCD I2C
#include <A4988.h>                //Step Driver A4988
#include <DallasTemperature.h>    //Sensor de Temperatura
#include <OneWire.h>              //Comunicação OneWire  
#include <WiFi.h>                 //Comunicação WiFi
#include <WebServer.h>            //Servidor HTTP

//Definições
#define STEPS_PER_REVOLUTION  200
#define DIR_PIN               4
#define STEP_PIN              5
#define RPM                   45
#define MS1                   13
#define MS2                   14
#define MS3                   15
#define FULL_STEP_MODE        1
#define MICROSTEPS_MODE       16

#define SDA_PIN               21
#define SCL_PIN               22

#define LCD_ADRESS            0x27
#define LCD_COLUMNS           16
#define LCD_ROWS              2

#define KY_CHANNEL_A          35
#define KY_CHANNEL_B          36
#define KY_SWITCH             39

#define END_SWITCH_PIN        34
#define LED_STANDBY_PIN       19
#define LED_ONCICLE_PIN       18
#define LED_STOP_PIN          23
#define RELAY_LAMP_PIN        26
#define RELAY_HEAT_PIN        25

#define TEMP_SENSOR_PIN       27

/* ===================================================Criação dos componentes=================================================== */
LED ledStandBy;
LED ledOnCicle;
LED ledStop;

Relay relayLamp;
Relay relayHeat;

EndSwitch endSwitch;

OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature temperatureSensor(&oneWire);

A4988 stepper(STEPS_PER_REVOLUTION, DIR_PIN, STEP_PIN, MS1, MS2, MS3);

LiquidCrystal_I2C lcd(LCD_ADRESS, LCD_COLUMNS, LCD_ROWS);

WebServer server(80);

/* ===================================================Criação de Variáveis=================================================== */
//Handle para tasks do Multiprocessing
TaskHandle_t Task1;


//Flags
bool doorIsOpen;
bool flagProcessOn;
bool abortFlag = false;
bool kyClick = false;
bool flagStart = false;
bool wifiConnected = false;

//Variáveis
float temperature;
int ndispositivos = 0;
unsigned short int menu_index = 0;
unsigned int timeSetPoint = 10;
unsigned int remainTime = 0;
String httpCommand;

bool pinALast;
bool channelA_state, channelB_state;
unsigned short int ndevices = 0;

//Credenciais WiFi
char ssid[] = "FMK";
char pass[] = "Fernand3";

/* ===================================================Corpo das Funções Auxiliares=================================================== */
void ky_click(){
  //Serial.print("kyClick: "); Serial.println(kyClick);
  if(digitalRead(KY_SWITCH) == LOW){
    Serial.println("Opcao Selecionada");
    kyClick = true;
  }
  else
    kyClick = false;
}

void ky_tick() {
  channelA_state = digitalRead(KY_CHANNEL_A);
  if(channelA_state != pinALast){
    if(digitalRead(KY_CHANNEL_B) != channelA_state){  //Encoder girado no sentido horário
      if(menu_index >= 2)
        menu_index = 2;
      else
        menu_index++;
      
    }
    else{                                             //Encoder girado no sentido anti-horário
      if(menu_index <= 0)
        menu_index = 0;
      else
        menu_index--;
    }

     Serial.print("Menu index: "); Serial.println(menu_index);

     if(menu_index == 0){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Ciclo Normal");
      } 
      if(menu_index == 1){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Ciclo Rápido");
      }
      if(menu_index == 2){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Abortar Ciclo");
      }
  }
  
  pinALast = channelA_state;

  ky_click();
}

void executeCicle(){
  Serial.println("Ciclo Selecionado");
  flagProcessOn = true;
  abortFlag = false;
  float currentTime = 0;
  float startTime = 0;

  lcd.clear();

  if(menu_index == 0){
    lcd.setCursor(0,0); lcd.print("Ciclo Normal");
    lcd.setCursor(0,1); lcd.print("30 minutos");
  }
  else if(menu_index == 1){
    lcd.setCursor(0,0); lcd.print("Ciclo Rapido");
    lcd.setCursor(0,1); lcd.print("5 minutos");
  }
  

  delay(1000);
  
  ledStandBy.setOff();
  ledOnCicle.setOn();
  ledStop.setOff();

  startTime = millis();
  while(currentTime < timeSetPoint){
    relayLamp.setRelay(HIGH);
    
    temperatureSensor.requestTemperatures();
    for(int i = 0; i < ndispositivos; i++){
    Serial.println("Graus Celsius:");
    Serial.print("Sensor ");
    Serial.print(i+1);
    Serial.print(": ");
    temperature = temperatureSensor.getTempCByIndex(i);
    Serial.print(temperature);
    Serial.println("ºC");
  }
    //Controle ON/OFF de temperatura
    if (flagProcessOn == true){
        if(temperature < 35.00)
          relayHeat.setRelay(HIGH);
        else
          relayHeat.setRelay(LOW);
    }
    
    checkDoorIsOpen();
    Serial.println("No ciclo");
    if(abortFlag == true){
      ledStop.setOn();
      ledOnCicle.setOff();
      ledStandBy.setOff();
      
      lcd.clear();
      lcd.setCursor(0,0); lcd.print("Ciclo Abortado");
      break;
    }

    remainTime = timeSetPoint - currentTime;
    currentTime = (millis() - startTime)/1000.00;

    //Evita underflow da variável
    if(remainTime < 0)
      remainTime = 0;

    lcd.clear();
    lcd.setCursor(0,0); lcd.print("Tempo Restante:");
    lcd.setCursor(0,1); lcd.print(remainTime);

    stepper.rotate(360);
  }
  flagStart = false;

  Serial.println("Terminou");
  flagProcessOn = false;

  relayLamp.setRelay(LOW);
  relayHeat.setRelay(LOW);

  if(abortFlag == false){
    ledStop.setOff();
    ledOnCicle.setOff();
    ledStandBy.setOn();

    lcd.clear();
    lcd.setCursor(0,0); lcd.print("Ciclo Finalizado");
  }

   delay(1000);
}

void checkDoorIsOpen(){
  if(endSwitch.read() == HIGH){      //HIGH = porta aberta
    //Serial.println("Porta aberta");
     abortFlag = true;

     if(flagProcessOn = false){
       ledStop.setOn();
       ledOnCicle.setOff();
       ledStandBy.setOn();
     }
  }
  else{
    //Serial.println("Porta Fechada");
  }
}

void serverMonitor(){
  //Estrutura do Payload: ledStopStatus, ledOnCicleStatus, ledStandByStatus, relayHeatStatus, relayLampStatus, doorStatus, temperature, timeRemaining
  String response = "";

  //Led Stop Status
  if(ledStop.getStatus() == true)
      response += "ON;";
  else
      response += "OFF;";

  //Led On Cicle Status
  if(ledOnCicle.getStatus() == true)
      response += "ON;";
  else
      response += "OFF;";

  //Led StandBy Status
  if(ledStandBy.getStatus() == true)
      response += "ON;";
  else
      response += "OFF;";

  //Relay Heat Status
  if(relayHeat.getStatusRelay() == true)
      response += "ON;";
  else
      response += "OFF;";

  //Relay Lamp Status
  if(relayLamp.getStatusRelay() == true)
      response += "ON;";
  else
      response += "OFF;";

  //Door Status
  if(endSwitch.read() == HIGH)
      response += "OPEN;";
  else
      response += "CLOSED;";

  //temperature
  response.concat(temperature);
  response += ";";

  //Time Remaining
  response.concat(remainTime);

  server.send(200, "text/plain", response);
}

void serverControl(){
  for (int i = 0; i < server.args(); i++){
    if(server.argName(i).equals("button")){
      httpCommand = server.arg(i);

      if(httpCommand == "Normal"){
        menu_index = 0;
        flagStart = true;
      }
      else if(httpCommand == "Rapid"){
        menu_index = 1;
       flagStart = true;

      }

      else if(httpCommand == "Abort"){
        abortFlag = true;
      }
    }
  }
}

/* ===================================================Configuração Inicial=================================================== */
void setup() {
  Wire.begin(SDA_PIN,SCL_PIN);
  Serial.begin(115200);
  pinMode(KY_CHANNEL_A, INPUT);
  pinMode(KY_CHANNEL_B, INPUT);
  pinMode(KY_SWITCH, INPUT);
  
  ledStandBy.attachToPin(LED_STANDBY_PIN);
  ledOnCicle.attachToPin(LED_ONCICLE_PIN);
  ledStop.attachToPin(LED_STOP_PIN);

  endSwitch.attachToPin(END_SWITCH_PIN);

  relayLamp.attachToPin(RELAY_LAMP_PIN);
  relayHeat.attachToPin(RELAY_HEAT_PIN);

  temperatureSensor.begin();
  ndispositivos = temperatureSensor.getDeviceCount();

  stepper.begin(RPM, MICROSTEPS_MODE);
  
  lcd.init();
  lcd.backlight();
  lcd.clear();

  ledStandBy.setOn();
  ledOnCicle.setOff();
  ledStop.setOff();

  relayLamp.setRelay(LOW);
  relayHeat.setRelay(LOW);


  pinALast = digitalRead(KY_CHANNEL_A);

 WiFi.begin(ssid, pass);
 float startTimeWifi = millis();
 float timeout = 0;
 
 while(WiFi.status() != WL_CONNECTED){
  timeout = (millis() - startTimeWifi)/1000.00;
  Serial.print("timeout: "); Serial.println(timeout);

  if(timeout > 10.00)
    break;
  
  delay(500);
  Serial.println("Aguardando Conexao...");
 }

 if(WiFi.status() == WL_CONNECTED){
  wifiConnected = true;
  Serial.print("IP address: "); Serial.println(WiFi.localIP());
  server.on("/control", serverControl);
  server.on("/monitor", serverMonitor);
  server.begin();
 }
 else{
  wifiConnected = false;
  ledStop.setOn();
  Serial.println("Wifi não conectado");
 }
 
 xTaskCreatePinnedToCore(runProcess, "PROCESSO", 10000, NULL, 0, &Task1, 0);     //Aloca a função "runProcess" no núcleo 0 do controlador.
}


/* ===================================================Loop principal do segundo CORE (CORE 1)=================================================== */
void loop() {  
  //Verificação da porta antes do processo
  checkDoorIsOpen();
  ky_tick();

  if (kyClick == true || flagStart == true) {
      Serial.println("Clicado");
      switch (menu_index){
        case 0: timeSetPoint = 10; executeCicle(); break;
        case 1: timeSetPoint = 5; executeCicle(); break;
        case 2: abortFlag = true; break;
  }
      
}
}

/* ===================================================Loop principal do primeiro CORE (CORE 0)=================================================== */
void runProcess(void * parameters) {
  for (;;) 
  {
      if(wifiConnected == true)
        server.handleClient();
  }
}
