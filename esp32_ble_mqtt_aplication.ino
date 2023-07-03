#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
// #include <BLEEddystoneURL.h>
// #include <BLEEddystoneTLM.h>
#include <BLEBeacon.h>
#include <WiFi.h>
#include <math.h>
#include <PubSubClient.h>


//Usuario e Senha Rede
// const char *SSID = "Maquina de Lavar";
// const char *PWD = "Brandenburg";
const char *SSID = "Maquina de Lavar";
const char *PWD = "Brandenburg";

//Variaveis
int scanTime = 5; //In seconds
BLEScan *pBLEScan;
const int ledPin = 21;//Entrada 21 para o pino

//MQTT CLIENT
char *mqttServer = "mqtt-dashboard.com";
int mqttPort = 1883;
int iteracao = 0;
//char data[10];
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

//WIFI
void connectToWiFi() {
  Serial.print("wifi...");
 
  WiFi.begin(SSID, PWD);
  Serial.println(SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("Connected");
}


//BLE
float distanciaRssi(int measured_power, int rssi, float n){
  float distancia = pow(10,float(measured_power-rssi)/(10*n));
  return distancia;
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
      if (advertisedDevice.haveName())
      {
        //Serial.print(F("Device name: "));
        //Serial.println(advertisedDevice.getName().c_str());
        //Serial.println("");
      }

      if (advertisedDevice.haveServiceUUID())
      {
        BLEUUID devUUID = advertisedDevice.getServiceUUID();
        Serial.print("Found ServiceUUID: ");
        //Serial.println(devUUID.toString().c_str());
        //Serial.println("");
      }
      
      if (advertisedDevice.haveManufacturerData() == true)
      {
        std::string strManufacturerData = advertisedDevice.getManufacturerData();

        uint8_t cManufacturerData[100];
        strManufacturerData.copy((char *)cManufacturerData, strManufacturerData.length(), 0);

        if (strManufacturerData.length() == 25 && cManufacturerData[0] == 0x4C && cManufacturerData[1] == 0x00)
        {
          //Serial.println("Found an iBeacon!");
          BLEBeacon oBeacon = BLEBeacon();
          oBeacon.setData(strManufacturerData);
          //Serial.printf("iBeacon Frame\n");
          //Serial.printf("ID: %04X Major: %d Minor: %d UUID: %s Power: %d\n", oBeacon.getManufacturerId(), ENDIAN_CHANGE_U16(oBeacon.getMajor()), ENDIAN_CHANGE_U16(oBeacon.getMinor()), oBeacon.getProximityUUID().toString().c_str(), oBeacon.getSignalPower());
        }
        else
        {
          //Serial.println(F("Found another manufacturers beacon!"));
          //Serial.printf("strManufacturerData: %d ", strManufacturerData.length());
          // for (int i = 0; i < strManufacturerData.length(); i++)
          // {
          //   Serial.printf("[%X]", cManufacturerData[i]);
          // }
          //Serial.printf("\n");
        }
      }

      uint8_t *payLoad = advertisedDevice.getPayload();
      // search for Eddystone Service Data in the advertising payload
      // *payload shall point to eddystone data or to its end when not found
      const uint8_t serviceDataEddystone[3] = {0x16, 0xAA, 0xFE}; // it has Eddystone BLE UUID
      const size_t payLoadLen = advertisedDevice.getPayloadLength();
      uint8_t *payLoadEnd = payLoad + payLoadLen - 1; // address of the end of payLoad space
      while (payLoad < payLoadEnd) {
        if (payLoad[1] == serviceDataEddystone[0] && payLoad[2] == serviceDataEddystone[1] && payLoad[3] == serviceDataEddystone[2]) {
          // found!
          payLoad += 4;
          break;
        }
        payLoad += *payLoad + 1;  // payLoad[0] has the field Length
      }

      // if (payLoad < payLoadEnd) // Eddystone Service Data and respective BLE UUID were found
      // {
      //   if (*payLoad == 0x10)
      //   {
      //     Serial.println(F("bcn fnd"));
      //     BLEEddystoneURL foundEddyURL = BLEEddystoneURL();
      //     uint8_t URLLen = *(payLoad - 4) - 3;  // Get Field Length less 3 bytes (type and UUID) 
      //     foundEddyURL.setData(std::string((char*)payLoad, URLLen));
      //     std::string bareURL = foundEddyURL.getURL();
      //     if (bareURL[0] == 0x00)
      //     {
      //       // dumps all bytes in advertising payload
      //       Serial.println("DATA-->");
      //       uint8_t *payLoad = advertisedDevice.getPayload();
      //       for (int idx = 0; idx < payLoadLen; idx++)
      //       {
      //         Serial.printf("0x%02X ", payLoad[idx]);
      //       }
      //       Serial.println("inv dtd");
      //       return;
      //     }

      //     Serial.printf("fd: %s\n", foundEddyURL.getURL().c_str());
      //     Serial.printf("dec: %s\n", foundEddyURL.getDecodedURL().c_str());
      //     Serial.printf("TX pwr %d\n", foundEddyURL.getPower());
      //     Serial.println("\n");
      //   } 
      //   else if (*payLoad == 0x20)
      //   {
      //     Serial.println(F("Found an EddystoneTLM beacon!"));
 
      //     BLEEddystoneTLM eddystoneTLM;
      //     eddystoneTLM.setData(std::string((char*)payLoad, 14));
      //     //era aqui
      //     Serial.println("\n");
      //   }
      // }
    }
};

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Callback - ");
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
}

void setupMQTT() {
  mqttClient.setServer(mqttServer, mqttPort);
  // set the callback function
  mqttClient.setCallback(callback);
}

void setupBLE(){
  BLEDevice::init("ESP32 Scanner");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); // less or equal setInterval value
}



void reconnect() {
  Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected()) {
    Serial.println("Reconnecting to MQTT Broker..");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    //String clientId = "clientId-JGmMf2Sx4L";
      
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("mqtt Connected.");
      // subscribe to topic
      mqttClient.subscribe("/swa/commands");
    } else {
      Serial.print("Failed to connect to MQTT Broker, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);//Declara a Serial Port Para Leitura
  pinMode(ledPin, OUTPUT);//Pin foi Declarado como 21
  //Serial.println("Scanning...");

  BLEDevice::init("ESP32 Scanner");//Declara um dispositivo BLE
  pBLEScan = BLEDevice::getScan(); //Declara o SCAN
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); // less or equal setInterval value
  connectToWiFi();//Conecta com o WIfi
  setupMQTT();
}



void loop()
{
  // put your main code here, to run repeatedly:
  Serial.println(iteracao);
  iteracao +=1;
  
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);

  int count = foundDevices.getCount();
  if (count >0){
    //Serial.println"Devices found: ");
    //Serial.println(count);
     // Valor inicial menor possível
    float distancias[count];
    int strongestRSSI = -100;
    int strongestRSSIposition = 0;
    for (int i = 0; i < count; i++) {
      //BLEAdvertisedDevice device = scanResults.getDevice(i);
      int rssi = foundDevices.getDevice(i).getRSSI();
      //Serial.println(rssi);
      if (rssi > strongestRSSI) {
        strongestRSSIposition = i;
        strongestRSSI = rssi;
      }
      distancias[i] = distanciaRssi(-69,rssi,2);
    }
    //float distancia = distanciaRssi(-69,distancias[strongestRSSIposition],2);
    int brightness = map(distancias[strongestRSSIposition], 10, 0, 0, 255);
    analogWrite(ledPin, brightness);
    Serial.println(distancias[strongestRSSIposition]);

    if (!mqttClient.connected())
      reconnect();
    for (int i=0; i < count; i++){
      char data[50];
      sprintf(data,"Loop:%d, Device: %d, Distance:%f",iteracao,i,distancias[i]);
      mqttClient.publish("/swa/distancia",data);
      if (i==strongestRSSIposition){
        Serial.println(data);
      }
      pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
    }

    delay(200);
    //End
  }
  // if (iteracao > 1000){
  //   // if (!mqttClient.connected())
  //   //   reconnect();
  //   //   delay(100);
  //   connectToWiFi();
  //   setupMQTT();
  //   iteracao = 0;
  //   char[10] data;
  //   sprintf(data,"%f",distancia);
  //   mqttClient.publish("swa/distancia",data);
  //   //mqttClient.disconnect();
  //   //Wifi.disconnect();
  // }


}

