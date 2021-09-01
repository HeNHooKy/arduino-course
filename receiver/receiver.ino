#include <ArduinoJson.h>
#include <SPI.h>                                          // Подключаем библиотеку  для работы с шиной SPI
#include <nRF24L01.h>                                     // Подключаем файл настроек из библиотеки RF24
#include <RF24.h>                                         // Подключаем библиотеку  для работы с nRF24L01+

//CE - 9; CSN - 10; SCK - 52; MO - 51; MI - 50;

#define BUFFER_SIZE 255
#define TRANSMITTER_BUFFER 32
#define STOP_WORD "</END>"

struct data {
  int temp;
  int humidity;
};
//TODO: добавить обработку ошибок

struct data all_datas[6] = {
  {0, 0},
  {0, 0},
  {0, 0},
  {0, 0},
  {0, 0},
  {0, 0}
};

RF24           radio(9, 10);                              // Создаём объект radio   для работы с библиотекой RF24, указывая номера выводов nRF24L01+ (CE, CSN)
unsigned long  data;                                      // Создаём массив для приёма данных

char byteArray[BUFFER_SIZE];
char transmittedByteArray[TRANSMITTER_BUFFER];

void setup(){
  Serial.begin(9600);
  delay(2000);
  radio.begin();                                        // Инициируем работу nRF24L01+
  radio.setChannel(1);                                  // Указываем канал приёма данных (от 0 до 127), 5 - значит приём данных осуществляется на частоте 2,405 ГГц (на одном канале может быть только 1 приёмник и до 6 передатчиков)
  radio.setDataRate     (RF24_250KBPS);                   // Указываем скорость передачи данных (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS), RF24_1MBPS - 1Мбит/сек
  radio.setPALevel      (RF24_PA_HIGH);                  // Указываем мощность передатчика (RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBm, RF24_PA_MAX=0dBm)
  radio.openReadingPipe (1, 0x60);                      // Открываем 1 трубу с идентификатором 0x1234567890 для приема данных (на ожном канале может быть открыто до 6 разных труб, которые должны отличаться только последним байтом идентификатора)
  radio.startListening  ();                             // Включаем приемник, начинаем прослушивать открытую трубу
  //radio.stopListening   ();                             // Выключаем приёмник, если потребуется передать данные
}

void loop(){
  if(radio.available()){                                // Если в буфере имеются принятые данные

    RadioReadWholeString();
    Serial.print("Receive: ");
    Serial.println(String((char *)byteArray));
    StaticJsonDocument<BUFFER_SIZE> doc;                    //подготовим переменную для хранения json объекта
    DeserializationError error = deserializeJson(doc, byteArray);
    
    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    //read the data
    int id = doc["id"];
    struct data read_data = {
      doc["data"]["temp"],
      doc["data"]["humidity"]
    };
    
    
    //write to serial port
    Serial.println("temperature: " + id);
    Serial.println("temperature: " + read_data.temp);
    Serial.println("humidity: " + read_data.humidity);
  }
}

void RadioReadWholeString() {
  radio.read(&byteArray, sizeof(byteArray)); 
  Serial.println(byteArray);
  while(true) {
    Serial.println("await...");
    if(!radio.available()) {
      continue;
    }

    
    
    
    radio.read(&transmittedByteArray, sizeof(transmittedByteArray));
    char* stopWordIndex = strstr(transmittedByteArray, STOP_WORD);
    
    if(stopWordIndex) {
      return;
    }

    cpy(byteArray, strlen(byteArray), transmittedByteArray, strlen(transmittedByteArray));
  }
}

char* cpy(char* arr, int arrLength, char* brr, int brrLength) {
  for(int i = arrLength; i < arrLength + brrLength; i++) {
    arr[i] = brr[i - arrLength];
  }
  
  return arr;
}

char* substr(char* arr, int begin, int len)
{
    char* res = new char[len + 1];
    for (int i = 0; i < len; i++)
        res[i] = *(arr + begin + i);
    res[len] = 0;
    return res;
}
