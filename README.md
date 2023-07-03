# esp32_ble_mqtt_aplication
 Trabalho Final de OIRC

Requerimentos:
    - Ferramenta para a compilação de código Arduino (Recomendo: Arduino IDE 2.1.1)
    - Porta COM para Arduino
      - Instrução para instalação porta COM https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
    - Bibliotecas:
      - BLEDevice
      - BLEUtils
      - BLEScan
      - BLEAdvertisedDevice
      - BLEBeacon
      - PubSubClient
      

Instruções:
  - Configure o ESP 32 em uma Protoboard para o pino 21 ou altere a variável ledPin para o pino desejado
  - Altere os valores de SSID e PWD para, respectivamente, o usuário e senha da rede a ser utilizada.
  - Defina a Serial Port para 115200 ou altere o valor em setup
  - Compile
    - Caso o processo de compilação estoure a memória, defina o esquema de partição para Huge App

Intruções Broker HIVEMQ:
  - Entre Nesse Link: https://www.hivemq.com/demos/websocket-client/
  - Ceritifique-se de que o host é mqtt-dashboard.com
  - Conecte-se
  - Adicione o tópico /swa/distancia

    