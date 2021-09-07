#include <SPI.h>                                          // Подключаем библиотеку для работы с шиной SPI
#include <nRF24L01.h>                                     // Подключаем файл настроек из библиотеки RF24
#include <RF24.h>                                         // Подключаем библиотеку для работы с nRF24L01+

//CE - 9; CSN - 10; SCK - 52; MO - 51; MI - 50;
#define BUFFER_SIZE 16

RF24           radio(9, 10);                              // Создаём объект radio для работы с библиотекой RF24, указывая номера выводов nRF24L01+ (CE, CSN)
unsigned long  data = 0;                                      // Создаём массив для приёма данных
unsigned int  temp = 24;
unsigned int humidity = 72;
unsigned int id = 0;

void setup(){
    Serial.begin(9600);
    delay(2000);
    radio.begin();                                        // Инициируем работу nRF24L01+
    radio.setChannel(1);                                  // Указываем канал передачи данных (от 0 до 127), 5 - значит передача данных осуществляется на частоте 2,405 ГГц (на одном канале может быть только 1 приёмник и до 6 передатчиков)
    radio.setDataRate     (RF24_250KBPS);                   // Указываем скорость передачи данных (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS), RF24_1MBPS - 1Мбит/сек
    radio.setPALevel      (RF24_PA_HIGH);                  // Указываем мощность передатчика (RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBm, RF24_PA_MAX=0dBm)
    radio.openWritingPipe (0x60);                         // Открываем трубу с идентификатором 0x1234567890 для передачи данных (на одном канале может быть открыто до 6 разных труб, которые должны отличаться только последним байтом идентификатора)
}


void loop(){
  if (millis() - data >= 1000){
    data = millis();
    Serial.print("Send: ");
    
    int transmittedData[BUFFER_SIZE];
    transmittedData[0] = id;
    transmittedData[1] = temp;
    transmittedData[2] = humidity;
    Serial.println(sizeof(transmittedData));
    radio.write(&transmittedData, sizeof(transmittedData));
  }
}
