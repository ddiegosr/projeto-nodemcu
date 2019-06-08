#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <ESP8266WebServer.h>
//#include <WiFiUDP.h>
#include <WiFiClient.h>
#include <Keypad.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include"FS.h"

#define BMP_SCK D4
#define BMP_SDI D5
#define BMP_CS D6
#define BMP_SDO D7

#define PATH "/Dados_Leitura.txt"

#define ssid "WiFi"
#define password "s3nh4f0rt3"

WiFiServer server(80);
//WiFiUDP udp;
//NTPClient ntp(udp, "a.st1.ntp.br", -3 * 3600, 60000);

Adafruit_BMP280 SensorBMP(BMP_CS, BMP_SDI, BMP_SDO, BMP_SCK);

int ADC_Value, cont;
char op, ChaveDigitada;
String senha = "123A";
String buff = "";// controle de acesso com senha
String buf; // Parte Web
const byte QuantLinhas  = 2;
const byte QuantColunas = 4;
byte TecladoLinha[QuantLinhas]   = {D3, 3}; // ,12,14,2
byte TecladoColuna[QuantColunas] = {D0, D1, D2, D8}; //,16

char MatrizTeclado[QuantLinhas][QuantColunas] = {{'1', '2', '3', 'A'},
                                                 {'4', '5', '6', 'B'}};

Keypad Teclado = Keypad(makeKeymap(MatrizTeclado), TecladoLinha, TecladoColuna, QuantLinhas, QuantColunas);

void setup() {
  Serial.begin(115200);
  SensorBMP.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); //Conecta ao WiFi

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print( "." );
  }
  
//  ntp.begin();  
  
  //Iniciando o servidor
  server.begin();
  Serial.print("\nServidor Iniciado em: ");
  Serial.println(WiFi.localIP());

  pinMode(ADC_Value, INPUT_PULLUP);

}
void loop() {
  //PedidoDeAcesso();
  do {
    Servidor();
  } while (MenuOpcoes());

}
//***************FUNÇÕES***************
void Servidor() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  Serial.println("Requisição de Dados feita com sucesso");
  while (!client.available()) {
    delay(1);
  }
  String req = client.readStringUntil('\r');
  Serial.print(req);
  client.flush();

  buf = "";
  buf += "<!DOCTYPE HTML>\r\n<html>\r\n";
  buf += "<head>\r\n<title>ESP8266 Web</title>\r\n<meta charset=\"utf-8\">\r\n";
  buf += "<style>body{font-family: sans-serif;}</style>\r\n</head>\r\n<body>";
  buf += "<h3>Servidor ESP8622</h3>";
  buf += "<div>";
  readFile();
  buf += "</div>";
  buf += "<h4 style=""font-family: sans-serif;""> Sitema de Coleta de dados</h4>";
  buf += "</body>\r\n</html>\n";

  client.print(buf);
  client.flush();

  Serial.println("Requisição Finalizada");
}
/*
  void PedidoDeAcesso(){
  ChaveDigitada = Teclado.getKey();
  if(ChaveDigitada != NO_KEY){
    if(ChaveDigitada == 'A'){
      cont = 0;
      buff = "";
    InserirSenha();
    //delay(3000);
    VerificaSenha();
    }
  }
  }
*/

/*
  void InserirSenha(){
  Serial.print("SENHA.: ");
  while(cont<4){
    ChaveDigitada = Teclado.getKey();
    if(ChaveDigitada != NO_KEY){
    buff+=ChaveDigitada;
    cont++;
    Serial.print("*");
    }
  }
  }*/

/*
  void VerificaSenha(){
  if(buff == senha){
      Serial.println("\nACESSO PERMITIDO");
      Serial.print("\n        ########### MENU DE OPÇÕES ###########\n");
      Serial.print("\n        1- Ler Umidade\n        2- Ler Temperatura\n        3- Ler Pressão\n");
      //MenuOpcoes();

  }else{
      Serial.println("\nACESSO NEGADO");
  }
  }*/


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
  delay(2000);

  File rFile = SPIFFS.open(PATH, "r"); //Abre o arquivo no modo leitura

  Serial.println("Lendo dados do arquivo...\n");
  while (rFile.available()) {
    String line = rFile.readStringUntil('\n');
    buf += line;
    buf += "<br>";
  }
  String readed = rFile.readString(); //Coleta todo conteúdo do arquivo e armazena na variável readed
  delay(2500);

  Serial.print(readed);
  Serial.print("\n");

  SPIFFS.end(); //Finaliza o sistema de arquivos para evitar perda de dados
}


void LeituraSerial() {
  openFS(); //Função responsável por abrir o sistema de arquivos
  delay(2000);

  File rFile = SPIFFS.open(PATH, "r"); //Abre o arquivo no modo leitura

  Serial.println("Lendo dados do arquivo...\n");
  String readed = rFile.readString(); //Coleta todo conteúdo do arquivo e armazena na variável readed
  delay(2500);
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
//    ntp.forceUpdate();
    delay(1200);

    rFile.print("|     Temperatura      ");
    rFile.print("|       ");
    rFile.print(dataTemperature);
    rFile.print("°C      |");
    rFile.print("      ");
//    rFile.print(ntp.getFormattedTime());
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
//    ntp.forceUpdate();
    delay(1200);

    rFile.print("|       Pressao        ");
    rFile.print("|    ");
    rFile.print(dataPressure);
    rFile.print(" Pa    |");
    rFile.print("      ");
//    rFile.print(ntp.getFormattedTime());
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
//    ntp.forceUpdate();
    delay(1200);

    rFile.print("|       Umidade        ");
    rFile.print("|       ");
    rFile.print(dataMoisture);
    rFile.print("%           |");
    rFile.print("      ");
//    rFile.print(ntp.getFormattedTime());
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
//    ntp.forceUpdate();
    delay(1200);

    rFile.print("|       Altitude       ");
    rFile.print("|        ");
    rFile.print(dataAltitude);
    rFile.print("m       |");
    rFile.print("      ");
//    rFile.print(ntp.getFormattedTime());
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
          writeAltitude(SensorBMP.readAltitude(1013.25), PATH);// referência de 1013.25 Pa
          //fileExistsVerify();
          //removeFile();
          break;
        case 'A':
          //readFile();
          LeituraSerial();
          //removeFile();
          //readFile(PATH);
          break;
        case '3':
          float porcentagem;
          int PortaSensorUmidade = 0;
          //Leitura do Sensor
          int ADC_Value = analogRead(PortaSensorUmidade); // 1024 em 3,3 volts
          delay(500);

          //UMIDADE EM PORCENTAGEM
          porcentagem = 100 * ((1024 - (float)ADC_Value) / 1023); // expressão para conversão do valor lido pelo sensor para porcentagem.

          float correcaoValor = ((100 * (1023 - ADC_Value)) / 512); // 512 é o valor da maior umidade lida pelo sensor, no caso, representa 100% de umidade.
          /*Serial.print("\nValor com correção: ");
            Serial.print(correcaoValor);
            Serial.print(" %");*/
          writeMoisture(correcaoValor, PATH);
          break;
      }
    }

  }
}
