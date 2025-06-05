#include <Wire.h>
#include <LiquidCrystal_I2C.h>   // Biblioteca para LCD I2C
#include <RTClib.h>              // Biblioteca para RTC DS1307
#include <EEPROM.h>              // Biblioteca para memória EEPROM
#include "DHT.h"                 // Biblioteca do sensor DHT22

// === DEFINIÇÕES DE PINOS E TIPOS ===
#define DHTPIN 7
#define DHTTYPE DHT22

#define TRIG_PIN 10
#define ECHO_PIN 9

#define BUZZER_PIN 8
#define LED_VERMELHO 11
#define LED_AMARELO 12
#define LED_VERDE 13

#define UTC_OFFSET -3  // Fuso horário (UTC-3)

// === OBJETOS ===
DHT dht(DHTPIN, DHTTYPE);
RTC_DS1307 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Endereço I2C do LCD

// === CONFIGURAÇÃO DA EEPROM ===
const int recordSize = 9;       // 4 (timestamp) + 2 (temperatura) + 2 (umidade) + 1 (distância)
const int maxRecords = 100;     // Máximo de registros
int currentAddress = 0;         // Endereço atual para gravação
int emergenciaCount = 0;        // Quantidade de emergências registradas
uint32_t ultimoTimestamp = 0;   // Timestamp da última emergência
int lastLoggedMinute = -1;      // Evita múltiplos registros no mesmo minuto

// === SETUP INICIAL ===
void setup() {
  Serial.begin(9600);
  Wire.begin();
  dht.begin();
  rtc.begin();
  lcd.init();
  lcd.backlight();

  // Configuração dos pinos
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(LED_AMARELO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);

  inicializarEEPROM(); // Recupera dados da EEPROM
}

// === LOOP PRINCIPAL ===
void loop() {
  // Leitura de data/hora com fuso
  DateTime now = rtc.now();
  uint32_t atualUnix = now.unixtime() + (UTC_OFFSET * 3600);
  DateTime ajustado = DateTime(atualUnix);

  // Leitura de sensores
  float distancia = medirDistanciaCM();
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();

  // Estados de alerta
  bool emergencia = distancia <= 15;
  bool alerta = distancia > 15 && distancia <= 30;
  bool normal = distancia > 30;

  // Reset dos atuadores
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AMARELO, LOW);
  digitalWrite(LED_VERMELHO, LOW);
  noTone(BUZZER_PIN);

  // Ações conforme o nível de risco
  if (emergencia) {
    digitalWrite(LED_VERMELHO, HIGH);
    tone(BUZZER_PIN, 1000);

    // Registra apenas uma vez por minuto
    if (ajustado.minute() != lastLoggedMinute) {
      lastLoggedMinute = ajustado.minute();
      salvarEventoEEPROM(atualUnix, temperatura, umidade, distancia);
      ultimoTimestamp = atualUnix;
    }

  } else if (alerta || umidade >= 85) {
    digitalWrite(LED_AMARELO, HIGH);
    lastLoggedMinute = -1;

  } else {
    digitalWrite(LED_VERDE, HIGH);
    lastLoggedMinute = -1;
  }

  // Atualização da interface
  mostrarLCD(ajustado);
  mostrarSerial(distancia, temperatura, umidade, emergencia, alerta, ajustado);

  delay(2000); // Intervalo entre ciclos
}

// === MEDIÇÃO COM O SENSOR ===
float medirDistanciaCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duracao = pulseIn(ECHO_PIN, HIGH);
  return duracao * 0.034 / 2;
}

// === INICIALIZAÇÃO E RECUPERAÇÃO DA EEPROM ===
void inicializarEEPROM() {
  uint32_t maisRecente = 0;
  int enderecoRecente = 0;
  emergenciaCount = 0;

  // Busca o timestamp mais recente
  for (int addr = 0; addr < recordSize * maxRecords; addr += recordSize) {
    uint32_t ts;
    EEPROM.get(addr, ts);

    if (ts != 0xFFFFFFFF && ts != 0) {
      emergenciaCount++;
      if (ts > maisRecente) {
        maisRecente = ts;
        enderecoRecente = addr;
      }
    }
  }

  // Define próximo endereço
  currentAddress = enderecoRecente + recordSize;
  if (currentAddress >= recordSize * maxRecords) {
    currentAddress = 0;
  }

  ultimoTimestamp = maisRecente;
}

// === SALVAR DADOS NA EEPROM ===
void salvarEventoEEPROM(uint32_t timestamp, float temp, float umid, float dist) {
  int tempInt = (int)(temp * 100);
  int umidInt = (int)(umid * 100);
  uint8_t distInt = (uint8_t)(dist * 10); // Salvo com 1 casa decimal

  EEPROM.put(currentAddress, timestamp);
  EEPROM.put(currentAddress + 4, tempInt);
  EEPROM.put(currentAddress + 6, umidInt);
  EEPROM.write(currentAddress + 8, distInt);

  currentAddress += recordSize;
  if (currentAddress >= (recordSize * maxRecords)) {
    currentAddress = 0;
  }

  if (emergenciaCount < maxRecords) {
    emergenciaCount++;
  }

  ultimoTimestamp = timestamp;
}

// === ATUALIZAÇÃO DO LCD ===
void mostrarLCD(DateTime agora) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Emergencias: ");
  lcd.print(emergenciaCount);

  if (ultimoTimestamp > 0) {
    uint32_t atual = agora.unixtime();
    uint32_t dif = atual - ultimoTimestamp;
    int dias = dif / 86400;
    int horas = (dif % 86400) / 3600;

    DateTime dt = DateTime(ultimoTimestamp);
    lcd.setCursor(0, 1);
    lcd.print("Ult: ");
    lcd.print(dias);
    lcd.print("d");
    lcd.print(horas);
    lcd.print("h (");
    lcd.print(dt.day() < 10 ? "0" : ""); lcd.print(dt.day());
    lcd.print("/");
    lcd.print(dt.month() < 10 ? "0" : ""); lcd.print(dt.month());
    lcd.print(")");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Sem emergencias");
  }
}

// === MONITOR SERIAL ===
void mostrarSerial(float distancia, float temperatura, float umidade, bool emergencia, bool alerta, DateTime agora) {
  Serial.println("----- DADOS ATUAIS -----");
  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");

  Serial.print("Umidade: ");
  Serial.print(umidade);
  Serial.println(" %");

  Serial.print("Status: ");
  if (emergencia) Serial.println("EMERGENCIA");
  else if (alerta || umidade >= 85) Serial.println("ALERTA");
  else Serial.println("NORMAL");

  Serial.print("Total de emergencias: ");
  Serial.println(emergenciaCount);

  if (ultimoTimestamp > 0) {
    DateTime dt = DateTime(ultimoTimestamp);
    Serial.print("Ultima emergencia: ");
    Serial.print(dt.day()); Serial.print("/");
    Serial.print(dt.month()); Serial.print("/");
    Serial.print(dt.year()); Serial.print(" ");
    Serial.print(dt.hour()); Serial.print(":");
    Serial.print(dt.minute()); Serial.print(":");
    Serial.println(dt.second());
  }

  Serial.println("-------------------------\n");
}
