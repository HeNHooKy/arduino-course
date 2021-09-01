#include <ArduinoJson.h>

#include <SPI.h>                                          // Подключаем библиотеку для работы с шиной SPI
#include <nRF24L01.h>                                     // Подключаем файл настроек из библиотеки RF24
#include <RF24.h>                                         // Подключаем библиотеку для работы с nRF24L01+

//CE - 9; CSN - 10; SCK - 52; MO - 51; MI - 50;

#define BUFFER_SIZE 255
#define TRANSMITTER_BUFFER 32

#define STOP_WORD_SIZE 6
#define STOP_WORD "</END>"

RF24           radio(9, 10);                              // Создаём объект radio для работы с библиотекой RF24, указывая номера выводов nRF24L01+ (CE, CSN)
unsigned long  data = 0;                                      // Создаём массив для приёма данных
unsigned int  temp = 24;
unsigned int humidity = 72;
unsigned int id = 0;
char transmitEnd[STOP_WORD_SIZE] = STOP_WORD;

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

    StaticJsonDocument<BUFFER_SIZE> doc;
    JsonObject root = doc.to<JsonObject>();
    
    root["id"] = id;
    JsonObject data = root.createNestedObject("data");
    data["temp"] = temp;
    data["humidity"] = humidity;
    
    char json[BUFFER_SIZE];
    serializeJson(doc, json);
    
    RadioWriteWholeString(json, strlen(json));                     // отправляем данные из массива data указывая сколько байт массива мы хотим отправить. Отправить данные можно с проверкой их доставки: if( radio.write(&data, sizeof(data)) ){данные приняты приёмником;}else{данные не приняты приёмником;}
  }
}

void RadioWriteWholeString(char * string, int strlength) {
  int lastStep = 0;
  int lenght = strlength;
  
  Serial.println(0);
  
  while(lastStep < strlength) {
    int lastIndex = min(lastStep + TRANSMITTER_BUFFER, strlength - 1);
    
    char transmittedString[TRANSMITTER_BUFFER];
    strcpy(transmittedString, substr(string, lastStep, lastIndex));
    
    radio.write(&transmittedString, sizeof(transmittedString));
    Serial.print(transmittedString);
    lastStep += TRANSMITTER_BUFFER;
  }
  
  radio.write(&transmitEnd, sizeof(transmitEnd));
  Serial.println();
}

char* substr(char* arr, int begin, int len)
{
    char* res = new char[len + 1];
    for (int i = 0; i < len; i++)
        res[i] = *(arr + begin + i);
    res[len] = 0;
    return res;
}
