#include <ArduinoJson.h>
#include <SPI.h>                                          // Подключаем библиотеку  для работы с шиной SPI
#include <nRF24L01.h>                                     // Подключаем файл настроек из библиотеки RF24
#include <RF24.h>                                         // Подключаем библиотеку  для работы с nRF24L01+

//CE - 9; CSN - 10; SCK - 52; MO - 51; MI - 50;

#define DOC_SIZE 512
#define BUFFER_SIZE 16
#define TRANSMITTERS_NUMBER 6
struct data {
  int temp;       //temp c
  int humidity;
};

struct data all_datas[TRANSMITTERS_NUMBER] = {
  {-274, -1},
  {-274, -1},
  {-274, -1},
  {-274, -1},
  {-274, -1},
  {-274, -1}
};

RF24           radio(9, 10);                              // Создаём объект radio   для работы с библиотекой RF24, указывая номера выводов nRF24L01+ (CE, CSN)

int byteArray[BUFFER_SIZE];

unsigned long  sendResponseTimer = 0;

void setup(){
  Serial.begin(9600);
  delay(2000);
  radio.begin();                                          // Инициируем работу nRF24L01+
  radio.setChannel(1);                                    // Указываем канал приёма данных (от 0 до 127), 5 - значит приём данных осуществляется на частоте 2,405 ГГц (на одном канале может быть только 1 приёмник и до 6 передатчиков)
  radio.setDataRate     (RF24_250KBPS);                   // Указываем скорость передачи данных (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS), RF24_1MBPS - 1Мбит/сек
  radio.setPALevel      (RF24_PA_HIGH);                   // Указываем мощность передатчика (RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBm, RF24_PA_MAX=0dBm)
  radio.openReadingPipe (1, 0x60);                        // Открываем 1 трубу с идентификатором 0x1234567890 для приема данных (на ожном канале может быть открыто до 6 разных труб, которые должны отличаться только последним байтом идентификатора)
  radio.startListening  ();                               // Включаем приемник, начинаем прослушивать открытую трубу
}

void loop(){
  //RADIO PART
  if(radio.available()){                                  // Если в буфере имеются принятые данные
    radio.read(&byteArray, sizeof(byteArray));

    //read the data
    int id = byteArray[0];
    struct data read_data = {
      byteArray[1],
      byteArray[2]
    };

    all_datas[id] = read_data;

    /*
    //write to serial port
    Serial.print("id: ");
    Serial.println(id);
    
    Serial.print("temperature: ");
    Serial.println(read_data.temp);
     
    Serial.print("humidity: ");
    Serial.println(read_data.humidity);
    */
  }
  
  //SEND RESPONSE PART
  if (millis() - sendResponseTimer >= 1000){
    sendResponseTimer = millis();
    Serial.print("Send to Pi: ");
    String json = GetJsonData();
    Serial.println(json);
  }
}

String GetJsonData() {
  StaticJsonDocument<DOC_SIZE> doc;
  JsonObject root = doc.to<JsonObject>();
  root["status"] = "OK";
  
  JsonArray data = root.createNestedArray("data");
  
  for(int i = 0; i < TRANSMITTERS_NUMBER; i++) {
    JsonObject transmitter = data.createNestedObject();
    transmitter["id"] = i;
    transmitter["temp"] = all_datas[i].temp;
    transmitter["humidiy"] = all_datas[i].humidity;
    data.add(transmitter);
  }
  String result;
  serializeJson(doc, result);
  return result;
}
