#include <ESP8266WiFi.h>
#include <PubSubClient.h>
bool k = 0, j = 0;//控制只发送一次数据

const char* ssid = "LAPTOP-M6KFJVA6";//热点名词
const char* password = "Nb-87111";//热点密码
const char* mqtt_server = "183.230.40.39";//mqtt服务器ip
const char* DeviceID = "576919057"; //设备号
const char* ProductID = "289662";  //产品号
const char* AuthInfo = "dovahkiin"; //鉴权信息

WiFiClient espClient;//定义mqtt的client
WiFiClient wificlient;//定义用来发送post的client

PubSubClient client(espClient);

int Sensor_pin = 0;//传感器引脚
int relay_pin = 2;//继电器一脚、、引脚
//计时变量
long previousMillis = 0;
//long interval = 3600000;
int interval = 6000,delay_interval=12000;
unsigned long currentMillis = 0, leaveMillis = 0;

//建立WiFi连接
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("-)");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}





//设置回调函数来用返回的数据存储到payload中，操作继电器
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if ((payload[0] - '0') == 1)//如果返回的是1，接通继电器
  {
    Serial.print("on");
    digitalWrite(relay_pin, HIGH);
    previousMillis = currentMillis = 0;
    k = 0;
  }
  if ((payload[0] - '0') == 0)//如果返回的是0，则断开继电器
  {
    Serial.print("off");
    digitalWrite(relay_pin, LOW);
    j = 0;
  }
}




//建立与onenet服务器mqtt连接
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(DeviceID, ProductID, AuthInfo)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}



void setup()
{ setup_wifi();
  client.setServer(mqtt_server, 6002);//建立与onenet的mqtt协议连接
  client.setCallback(callback);
  Serial.begin(115200);//设置串口
  Serial.println("Ready");
  pinMode(relay_pin, OUTPUT);
  digitalWrite(relay_pin, HIGH);//初始化继电器
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }//保持mqtt连接
  client.loop();//保持mqtt连接接收信息



  currentMillis = millis();
  int val = digitalRead(Sensor_pin);//读取红外传感器数值，1为有人，0为无人

  if (val == 1)//当有人时
  { Serial.println("Carbon-based creature detected");
    delay(1000);
    //    digitalWrite(relay_pin, HIGH);//继电器接通
    previousMillis = currentMillis   ;//计时器停止工作
    k = 0;
    if (j == 0) {//k，j的存在使当val数值变化后，只向服务器发送一词报文
      Serial.println("Msg Sent.");
      wificlient.connect( "183.230.40.33", 80);
      if (wificlient.connected()) {//用http连接发送post报文新增数据点
        String url = "http://api.heclouds.com/devices/576919057/datapoints?type=3";
        wificlient.print(String("POST ") + url + " HTTP/1.1\r\n");
        wificlient.print(String("api-key: ") + "p9fU76aB5ze2zDVK1ZHNmXgJuj0=\r\n");
        wificlient.print(String("Host: ") + "api.heclouds.com\r\n");
        wificlient.print("Connection: close\r\n");
        wificlient.print("Content-Length:12\r\n\r\n");
        wificlient.print(String("{\"tixing\":"));
        wificlient.print(1);
        wificlient.print(String("}"));
      }
      wificlient.stop();//断开http连接
      j++;
    }

  }

  else
  { Serial.println("There is no carbon-based creature moving for");//如果红外传感器检测到没人，在串口打已经没有人的时长
    Serial.print((currentMillis - previousMillis) / 1000, DEC);
    Serial.println(" second");
    delay(1000);
  }
  if (currentMillis - previousMillis > interval)//如果没人时长大于1小时，向服务器发送报文新增数据点。
  { 
    if (k == 0&&j!=0)
    { 
      leaveMillis=currentMillis;
      wificlient.connect( "183.230.40.33", 80);//http连接服务器，发送tixing=0的报文
      if (wificlient.connected()) {
        String url = "http://api.heclouds.com/devices/576919057/datapoints?type=3";
        wificlient.print(String("POST ") + url + " HTTP/1.1\r\n");
        wificlient.print(String("api-key: ") + "p9fU76aB5ze2zDVK1ZHNmXgJuj0=\r\n");
        wificlient.print(String("Host: ") + "api.heclouds.com\r\n");
        wificlient.print("Connection: close\r\n");
        wificlient.print("Content-Length:12\r\n\r\n");
        wificlient.print(String("{\"tixing\":"));
        wificlient.print(0);
        wificlient.print(String("}"));

      }
      wificlient.stop();
      k++;

    }
    j = 0;
    if (currentMillis-leaveMillis>delay_interval&&leaveMillis!=0) {
      digitalWrite(relay_pin,LOW);
      leaveMillis=0;
    }
  }


  if (currentMillis < previousMillis)//当计时器溢出，归零
    previousMillis = currentMillis = 0;


}
