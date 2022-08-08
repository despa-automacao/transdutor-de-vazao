#include <Arduino.h>
#include <Wire.h>
#include "SparkFunBME280.h"
#include <MicroLCD.h>
#include <string.h>
BME280 mySensorA; //Uses default I2C address 0x77
BME280 mySensorB; //Uses I2C address 0x76 (jumper closed)
LCD_SSD1306 lcd;
double Ptomada;
double Psaida;
double vazao;
double pstela;
double pttela;
String Ttomada;
String Tsaida;
double modulo;
double raiz;

//Variaveis auxiliares para o calculo mais preciso da vazão na função loop
double var1Vazao, var2Vazao, var3Vazao;

String vS;

//######___ variavesis do filtro de media movel ___######
#define nA 20// numero de pontor 
#define nB 20
double soma_tomada; //somatoria dos valores
double soma_saida;
float Psaida_filtrada; // valor filtrado
float Ptomada_filtrada;
double vetorA[nA]; //vetor do filtro
double vetorB[nB];
//----------------------------------------------

unsigned long Tempo_amostragem_tela = 0;
unsigned long tempo_entre_leituraras = 0;
void setup()
{
  Serial.begin(9600);
  Wire.begin();
  //Wire.setClock(400000);
  lcd.begin();
  millis();

  mySensorA.setI2CAddress(0x77);
  if (mySensorA.beginI2C() == false) Serial.println("Sensor A connect failed");
  mySensorB.setI2CAddress(0x76);
  if (mySensorB.beginI2C() == false) Serial.println("Sensor A connect failed");
  delay(500);
}


void loop()
{

  if (millis() - tempo_entre_leituraras > 200) { //aquisição de dados
    Tsaida = mySensorA.readTempC();
    Ttomada = mySensorB.readTempC();
    Ptomada = mySensorA.readFloatPressure();
    Psaida = mySensorB.readFloatPressure();
    Ptomada_filtrada = filtro_pressao_tomada(); // chamada do filtro da tomada
    Psaida_filtrada = filtro_pressao_saida();  // chamada do filtro da saida
    pstela = (Psaida_filtrada - 91127) -368; //valor em psi
    pttela = (Ptomada_filtrada - 91127) -386; //valor em psi
    modulo = abs(Ptomada_filtrada - Psaida_filtrada);
    raiz = sqrt(modulo);
    //Calculo da vazão utilizando variaveis auxiliares
    var1Vazao = raiz * 0.9478;
    var2Vazao = var1Vazao - 3.59;
    var3Vazao = var2Vazao * 10.628;
    vazao = var3Vazao; //pressão em hPa(hectopascal)
    
    tempo_entre_leituraras = millis();
  }

  if (millis() - Tempo_amostragem_tela > 1000) { //print dos dados
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.setFontSize(FONT_SIZE_MEDIUM);
    lcd.print("S");
    lcd.setCursor(13, 0);
    lcd.setFontSize(FONT_SIZE_MEDIUM);
    lcd.print("hPa = ");
    lcd.setCursor(78, 0);
    lcd.setFontSize(FONT_SIZE_MEDIUM);
    if(pstela<0)
    lcd.print((pstela*(-1)),0);
    else lcd.print(pstela, 0);
    lcd.setCursor(0, 2);
    lcd.setFontSize(FONT_SIZE_MEDIUM);
    lcd.print("T");
    lcd.setCursor(13, 2);
    lcd.setFontSize(FONT_SIZE_MEDIUM);
    lcd.print("hPa = ");
    lcd.setCursor(78, 2);
    lcd.setFontSize(FONT_SIZE_MEDIUM);
    if(pttela<0)
    lcd.print((pttela*(-1)),0);
    else lcd.print(pttela, 0);
    lcd.setCursor(2, 4);
    lcd.setFontSize(FONT_SIZE_MEDIUM);
    lcd.print("VAZAO");
    lcd.setCursor(56, 4);
    lcd.setFontSize(FONT_SIZE_MEDIUM);
    lcd.print("-");
    lcd.setCursor(76, 4);
    lcd.setFontSize(FONT_SIZE_MEDIUM);
    lcd.print("L/min  ");
    lcd.setCursor(45, 6);
    lcd.setFontSize(FONT_SIZE_MEDIUM);
    if(vazao < 0)
    lcd.print((vazao*(-1)),2);
    else lcd.print(vazao,2);

    // Serial.print(" Pressure: ");
    //  Serial.print(Ttomada);
    // Serial.print(" - ");
    // Serial.println(Tsaida);
    vS = vazao;
    Serial.println(vS);

    Tempo_amostragem_tela = millis();
  }
}

//--------------- fim do loop ------------------------


double filtro_pressao_tomada() { //função do filtro da pressão de tomada
  for (int i = nA - 1; i > 0; i--) {
    vetorA[i] = vetorA[i - 1];
  }
  double accA = 0;
  vetorA[0] = Ptomada;
  for (int i = 0; i < nA; i++) {
    accA += vetorA[i];
  }
  return accA / nA;
}

double filtro_pressao_saida() {  //função do filtro da pressão de saida
  for (int i = nB - 1; i > 0; i--) {
    vetorB[i] = vetorB[i - 1];
  }
  double accB = 0;
  vetorB[0] = Psaida;
  for (int i = 0; i < nB; i++) {
    accB += vetorB[i];
  }
  return accB / nB;
}
