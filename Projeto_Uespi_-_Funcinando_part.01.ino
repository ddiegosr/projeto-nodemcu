#include <ESP8266WiFi.h> //INCLUSÃO DA BIBLIOTECA NECESSÁRIA PARA FUNCIONAMENTO DO CÓDIGO
#include <NTPClient.h>
//#include "SPIFFS.h"
#include <ESP8266WebServer.h>
#include <WiFiUDP.h>
#include <WiFiClient.h>
#include <Keypad.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include "FS.h"

#define BMP_SCK D4 // SCK = SCL (D4)
#define BMP_SDI D5 // SDA = SDI (D5)
#define BMP_CS D6  // CS  = CSB (D6)
#define BMP_SDO D7 // SHO = SHO (D7)

#define PATH "/Dados_Leitura.txt"

WiFiServer server(8082);
WiFiUDP udp;
NTPClient ntp(udp, "a.st1.ntp.br", -3 * 3600, 60000);

Adafruit_BMP280 SensorBMP(BMP_CS, BMP_SDI, BMP_SDO, BMP_SCK);

int ADC_Value, cont;
char op, ChaveDigitada;
String senha = "123A";
String buff = "";// controle de acesso com senha
String buf; // Parte Web
const byte QuantLinhas  = 1;
const byte QuantColunas = 4;
byte TecladoLinha[QuantLinhas]   = {D0}; //D0
byte TecladoColuna[QuantColunas] = {D1, D2, D3, D8}; //D1,D2,D3,D8

char MatrizTeclado[QuantLinhas][QuantColunas] = {{'1', '2', '3', 'A'}};

Keypad Teclado = Keypad(makeKeymap(MatrizTeclado), TecladoLinha, TecladoColuna, QuantLinhas, QuantColunas);  
 
const char* ssid = "zulum"; //VARIÁVEL QUE ARMAZENA O NOME DA REDE SEM FIO EM QUE VAI CONECTAR
const char* password = "Xuruleibe1956"; //VARIÁVEL QUE ARMAZENA A SENHA DA REDE SEM FIO EM QUE VAI CONECTAR
 
//WiFiServer server(8082); //CASO OCORRA PROBLEMAS COM A PORTA 80, UTILIZE OUTRA (EX:8082,8089) E A CHAMADA DA URL FICARÁ IP:PORTA(EX: 192.168.0.15:8082)
 
void setup() {
  Serial.begin(115200); //INICIALIZA A SERIAL
  SensorBMP.begin();

  delay(10); //INTERVALO DE 10 MILISEGUNDOS
 
  Serial.println(""); //PULA UMA LINHA NA JANELA SERIAL
  Serial.println(""); //PULA UMA LINHA NA JANELA SERIAL
  Serial.print("Conectando a "); //ESCREVE O TEXTO NA SERIAL
  Serial.print(ssid); //ESCREVE O NOME DA REDE NA SERIAL
   
  WiFi.begin(ssid, password); //PASSA OS PARÂMETROS PARA A FUNÇÃO QUE VAI FAZER A CONEXÃO COM A REDE SEM FIO
   
  while (WiFi.status() != WL_CONNECTED) { //ENQUANTO STATUS FOR DIFERENTE DE CONECTADO
  delay(500); //INTERVALO DE 500 MILISEGUNDOS
  Serial.print("."); //ESCREVE O CARACTER NA SERIAL
  }
  ntp.begin();
  Serial.println(""); //PULA UMA LINHA NA JANELA SERIAL
  Serial.print("Conectado a rede sem fio "); //ESCREVE O TEXTO NA SERIAL
  Serial.println(ssid); //ESCREVE O NOME DA REDE NA SERIAL
  server.begin(); //INICIA O SERVIDOR PARA RECEBER DADOS NA PORTA DEFINIDA EM "WiFiServer server(porta);"
  Serial.println("Servidor iniciado"); //ESCREVE O TEXTO NA SERIAL
   
  Serial.print("IP para se conectar ao NodeMCU: "); //ESCREVE O TEXTO NA SERIAL
  Serial.print("http://"); //ESCREVE O TEXTO NA SERIAL
  Serial.println(WiFi.localIP()); //ESCREVE NA SERIAL O IP RECEBIDO DENTRO DA REDE SEM FIO (O IP NESSA PRÁTICA É RECEBIDO DE FORMA AUTOMÁTICA)
}
void loop() {
  Servidor();
  MenuOpcoes();

//  do {
//    Servidor();
//  } while (MenuOpcoes());

  
}


void Servidor(){
  WiFiClient client = server.available(); //VERIFICA SE ALGUM CLIENTE ESTÁ CONECTADO NO SERVIDOR
  
  if (!client) { //SE NÃO EXISTIR CLIENTE CONECTADO, FAZ
    return; //REEXECUTA O PROCESSO ATÉ QUE ALGUM CLIENTE SE CONECTE AO SERVIDOR
  }
  
  Serial.println("Novo cliente se conectou!"); //ESCREVE O TEXTO NA SERIAL
  while(!client.available()){ //ENQUANTO CLIENTE ESTIVER CONECTADO
    delay(1); //INTERVALO DE 1 MILISEGUNDO
  }
  
  String request = client.readStringUntil('\r'); //FAZ A LEITURA DA PRIMEIRA LINHA DA REQUISIÇÃO
  Serial.println(request); //ESCREVE A REQUISIÇÃO NA SERIAL
  client.flush(); //AGUARDA ATÉ QUE TODOS OS DADOS DE SAÍDA SEJAM ENVIADOS AO CLIENTE
   
  client.println("HTTP/1.1 200 OK"); //ESCREVE PARA O CLIENTE A VERSÃO DO HTTP
  client.println("Content-Type: text/html"); //ESCREVE PARA O CLIENTE O TIPO DE CONTEÚDO(texto/html)
  client.println("");
  client.println("<!DOCTYPE HTML>"); //INFORMA AO NAVEGADOR A ESPECIFICAÇÃO DO HTML
  client.println("<html>"); //ABRE A TAG "html"
  client.println("<h1><center>Projeto Uespi</center></h1>"); //ESCREVE "Ola cliente!" NA PÁGINA
  client.println("<center><font size='3'>Dados Coletados</center>"); //ESCREVE "Seja bem vindo!" NA PÁGINA
  readFile();
  client.println(buf);
  client.flush();
  client.println("</html>"); //FECHA A TAG "html"
  delay(1); //INTERVALO DE 1 MILISEGUNDO
  Serial.println("Cliente desconectado"); //ESCREVE O TEXTO NA SERIAL
  Serial.println(""); //PULA UMA LINHA NA JANELA SERIAL
}

void openFS(void) {
  if (!SPIFFS.begin()) {
    Serial.println("Falha ao abrir o sistema de arquivos!");
    delay(3000);
    exit(0);
  } else {
    Serial.println("Sistema de arquivos carregado com sucesso!\n");
  }
}

void removeFile(void) {
  openFS();
  delay(2500);
  bool verifyFile = SPIFFS.exists(PATH); //Armazena o resultado da verificação da existência do arquivo
  if (verifyFile) {
    Serial.println("Preparando arquivo para remoção...\n");
    delay(2000);
    Serial.println("Removendo arquivo...\n");
    delay(1800);
    bool dropFile = SPIFFS.remove(PATH); //Armazena o resultado da verificação da exclusão do arquivo
    if (dropFile) {
      Serial.println("Arquivo removido do sistema de arquivos, por favor, realize o upload do arquivo novamente!\n");
    }
  } else {
    Serial.println("Não existem arquivos para remoção!");
  }
}

void readFile() {
  openFS(); //Função responsável por abrir o sistema de arquivos
  delay(3000);

  File rFile = SPIFFS.open(PATH, "r"); //Abre o arquivo no modo leitura

  Serial.println("Lendo dados do arquivo...\n");
  while (rFile.available()) {
    String line = rFile.readStringUntil('\n');
    buf += line;
    buf += "<br />";
  }
  String readed = rFile.readString(); //Coleta todo conteúdo do arquivo e armazena na variável readed
  delay(3500);

  Serial.print(readed);
  Serial.print("\n");

  SPIFFS.end(); //Finaliza o sistema de arquivos para evitar perda de dados
}
void LeituraSerial() {
  openFS(); //Função responsável por abrir o sistema de arquivos
  delay(3000);

  File rFile = SPIFFS.open(PATH, "r"); //Abre o arquivo no modo leitura

  Serial.println("Lendo dados do arquivo...\n");
  String readed = rFile.readString(); //Coleta todo conteúdo do arquivo e armazena na variável readed
  delay(3500);
  Serial.print(readed);
  Serial.print("\n");

  SPIFFS.end(); //Finaliza o sistema de arquivos para evitar perda de dados
}
//FUNÇÃO PARA VERIFICAR SE DETERMINADO ARQUIVO EXISTE NO SISTEMA DE ARQUIVOS
void fileExistsVerify(void) {
  SPIFFS.begin();
  bool verifyFile = SPIFFS.exists(PATH);
  if (verifyFile) {
    Serial.println("Arquivo existe!");
  } else {
    Serial.println("Arquivo não existe!");
  }
  SPIFFS.end();
}

//FUNÇÃO PARA GRAVAR A LEITURA DA TEMPERATURA NO ARQUIVO
void writeTemperature(float dataTemperature, String path) {
  openFS();
  delay(1600);

  File rFile = SPIFFS.open(path, "a"); //Abertura do arquivo no mode append para adicionar conteúdo ao já existente no arquivo
  if (!rFile) {
    Serial.println("Erro ao abrir arquivo!");
  } else {
    //Faz a gravação das próximas 3 linhas no arquivo
    Serial.println("Salvando dados coletados...\n");
    ntp.forceUpdate();
    delay(1200);

    rFile.print("|     Temperatura      ");
    rFile.print("|       ");
    rFile.print(dataTemperature);
    rFile.print("°C      |");
    rFile.print("      ");
    rFile.print(ntp.getFormattedTime());
    rFile.println("      |");
    rFile.println("+----------------------+--------------------+--------------------+");

    Serial.println("Temperatura Salvo(a) com sucesso!\n");
  }
  rFile.close(); //Fecha o arquivo
  SPIFFS.end(); //Encerra o sistema de arquivos

  /************************************************************************|
    |Os comentários anteriores servem para as próximas 3 funções de gravação |
    |************************************************************************/
}

//FUNÇÃO PARA GRAVAR A LEITURA DA PRESSÃO NO ARQUIVO
void writePressure(float dataPressure, String path) {
  openFS();
  delay(1600);

  File rFile = SPIFFS.open(path, "a");
  if (!rFile) {
    Serial.println("Erro ao abrir arquivo!");
  } else {
    Serial.println("Salvando dados coletados...\n");
    ntp.forceUpdate();
    delay(1200);

    rFile.print("|       Pressao        ");
    rFile.print("|    ");
    rFile.print(dataPressure);
    rFile.print(" Pa    |");
    rFile.print("      ");
    rFile.print(ntp.getFormattedTime());
    rFile.println("      |");
    rFile.println("+----------------------+--------------------+--------------------+");

    Serial.println("Pressão Salvo(a) com sucesso!\n");
  }
  rFile.close();
  SPIFFS.end();
}

//FUNÇÃO PARA GRAVAR A LEITURA DA UMIDADE DO SOLO NO ARQUIVO
void writeMoisture(float dataMoisture, String path) {
  openFS();
  delay(1600);

  File rFile = SPIFFS.open(path, "a");
  if (!rFile) {
    Serial.println("Erro ao abrir arquivo!");
  } else {
    Serial.println("Salvando dados coletados...\n");
    ntp.forceUpdate();
    delay(1200);

    rFile.print("|       Umidade        ");
    rFile.print("|       ");
    rFile.print(dataMoisture);
    rFile.print("%           |");
    rFile.print("      ");
    rFile.print(ntp.getFormattedTime());
    rFile.println("          |");
    rFile.println("+----------------------+--------------------+--------------------+");

    Serial.println("Umidade Salvo(a) com sucesso!\n");
  }
  rFile.close();
  SPIFFS.end();
}

//FUNÇÃO PARA GRAVAR A LEITURA DA ALTITUDE NO ARQUIVO
void writeAltitude(float dataAltitude, String path) {
  openFS();
  delay(1600);

  File rFile = SPIFFS.open(path, "a");
  if (!rFile) {
    Serial.println("Erro ao abrir arquivo!");
  } else {
    Serial.println("Salvando dados coletados...\n");
    ntp.forceUpdate();
    delay(1200);

    rFile.print("|       Altitude       ");
    rFile.print("|        ");
    rFile.print(dataAltitude);
    rFile.print("m       |");
    rFile.print("      ");
    rFile.print(ntp.getFormattedTime());
    rFile.println("      |");
    rFile.println("+----------------------+--------------------+--------------------+");

    Serial.println("Altitude Salvo(a) com sucesso!\n");
  }
  rFile.close();
  SPIFFS.end();
}

bool MenuOpcoes() {
  op = Teclado.getKey();
  if (op != NO_KEY) {
    if (op) {
      switch (op) {
        case'1':
          writeTemperature(SensorBMP.readTemperature(), PATH);
          break;
        case'2':
          writePressure(SensorBMP.readPressure(), PATH);
          break;
        case'B':
          //writeAltitude(SensorBMP.readAltitude(1013.25), PATH);// referência de 1013.25 Pa
          //fileExistsVerify();
          //removeFile();
          break;
        case 'A':
          //readFile();
          LeituraSerial();
          //removeFile();
          break;
        case '3':
          float porcentagem;
          int PortaSensorUmidade = A0;
          //Leitura do Sensor
          int ADC_Value = analogRead(PortaSensorUmidade); // 1024 em 3,3 volts
          delay(500);

          //UMIDADE EM PORCENTAGEM
          porcentagem = 100 * ((1024 - (float)ADC_Value) / 1023); // expressão para conversão do valor lido pelo sensor para porcentagem.

          float correcaoValor = ((100 * (1023 - ADC_Value)) / 515); // 515 é o valor da maior umidade lida pelo sensor, no caso, representa 100% de umidade.
          /*Serial.print("\nValor com correção: ");
            Serial.print(correcaoValor);
            Serial.print(" %");*/
          writeMoisture(correcaoValor, PATH);
          break;
      }
    }

  }
}
