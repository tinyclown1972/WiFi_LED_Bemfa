/*
 *.Author : XHY
 *.Date : 2022/8/12
 */

/*
 * 智能语言控制控制，支持同时天猫、小爱、小度、google Assistent控制
 * 也同时支持web控制、小程序控制、app控制，定时控制等
 * QQ群：566565915
 * 项目示例：通过发送on或off控制开关
 * 官网：bemfa.com
 */

#include <ESP8266WiFi.h>  //默认，加载WIFI头文件
#include "PubSubClient.h" //默认，加载MQTT库文件
#include <FastLED.h>
#include <Ticker.h>

//********************需要修改的部分*******************//
const char *ssid = "";          //修改，你的路由去WIFI名字
const char *password = "";      //你的WIFI密码
#define ID_MQTT ""              //用户私钥，控制台获取
const char *topic = "light002"; //主题名字，可在巴法云控制台自行创建，名称随意
#define LED_PIN 4               // ws2812 数据接口
#define NUM_LEDS 46             // 灯珠数量
CRGB leds[NUM_LEDS];
//**************************************************//

Ticker ticker;
const char *mqtt_server = "bemfa.com"; //默认，MQTT服务器
const int mqtt_server_port = 9501;     //默认，MQTT服务器
WiFiClient espClient;
PubSubClient client(espClient);

int color_R = 255;
int color_G = 255;
int color_B = 255;

boolean status_on = false;

//灯光函数及引脚定义
void turnOnLed();
void turnOffLed();
void changeBrightness(int bri);
void changeColor(int G, int R, int B);

void setup_wifi()
{

    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void changeColor_(String color)
{
    if (color == "Red")
    {
        color_R = 255;
        color_G = 0;
        color_B = 0;
    }
    else if (color == "Blue")
    {
        color_R = 0;
        color_G = 0;
        color_B = 255;
    }
    else if (color == "White")
    {
        color_R = 255;
        color_G = 255;
        color_B = 255;
    }
    else if (color == "Yellow")
    {
        color_R = 255;
        color_G = 255;
        color_B = 0;
    }
    else if (color == "Orange")
    {
        color_R = 255;
        color_G = 97;
        color_B = 0;
    }
    else if (color == "Green")
    {
        color_R = 0;
        color_G = 255;
        color_B = 0;
    }
    else if (color == "LightGreen")
    {
        color_R = 0;
        color_G = 255;
        color_B = 255;
    }
    else if (color == "Black")
    {
        color_R = 0;
        color_G = 0;
        color_B = 0;
    }
    else
    {
        color_R = 255;
        color_G = 255;
        color_B = 255;
    }
    // turnOnLed();
    FastLED.show();
}

void msgProcess(String temp)
{
    if (temp == "on" || temp == "off")
    {
        // 说明是第一个参数
        Serial.print("try to change state ");
        Serial.println(temp);
        if (temp == "on")
        { //如果接收字符on，亮灯
            // turnOnLed(); //开灯函数
            status_on = true;
        }
        else if (temp == "off")
        { //如果接收字符off，亮灯
            // turnOffLed(); //关灯函数
            status_on = false;
            // break;
        }
    }
    else if (temp.toInt() >= 0 && temp.toInt() <= 100)
    {
        Serial.print("try to change Brightness ");
        Serial.println(temp);
        changeBrightness(temp.toInt());
    }
    else if (temp.toInt() > 100)
    {
        int color = temp.toInt();
        Serial.print("try to change color ");
        Serial.println(color);
        // if (temp == "255")
        //     changeColor_("Blue");
        // else if (temp == "16711680")
        //     changeColor_("Red");
        // else if (temp == "16768685")
        //     changeColor_("White");
        // else if (temp == "65280")
        //     changeColor_("LightGreen");
        // else if (temp == "16744192")
        //     changeColor_("Orange");
        // else if (temp == "65535")
        //     changeColor_("GreenYellow");
        // else if (temp == "16776960")
        //     changeColor_("Yellow");
        // else
        //     changeColor_("White");
        color_B = color & 0xff;
        color_G = (color >> 8) & 0xff;
        color_R = (color >> 16) & 0xff;
    }
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Topic:");
    Serial.println(topic);
    int commaPosition;
    int count = 0;
    String msg = "";
    String temp = "";
    for (int i = 0; i < length; i++)
    {
        msg += (char)payload[i];
    }
    Serial.print("Msg:");
    Serial.println(msg);

    do
    {
        commaPosition = msg.indexOf('#');
        count++;
        if (commaPosition != -1)
        {
            //
            temp = msg.substring(0, commaPosition);
            // Serial.println(temp);
            msgProcess(temp);
            msg = msg.substring(commaPosition + 1, msg.length());
            //  Serial.println(msg);
        }
        else
        {
            if (msg.length() > 0)
            {
                msgProcess(msg);
            }
            // FastLED.show();
        }
    } while (commaPosition >= 0);

    changeColor(color_G, color_R, color_B);

    // FastLED.show();
    msg = "";
}

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect(ID_MQTT))
        {
            Serial.println("connected");
            Serial.print("subscribe:");
            Serial.println(topic);
            //订阅主题，如果需要订阅多个主题，可发送多条订阅指令client.subscribe(topic2);client.subscribe(topic3);
            client.subscribe(topic);
            changeColor_("Green");
            for (int i = 0; i < 5; i++)
            {
                delay(100);
                // turnOffLed();
                status_on = false;
                delay(100);
                // turnOnLed();
                status_on = true;
            }
            changeColor_("White");
            // turnOffLed();
            status_on = false;
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // turnOffLed();
            status_on = false;
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void onTimer()
{
    if (status_on)
        turnOnLed();
    else
        turnOffLed();
}

void setup()
{
    // pinMode(B_led, OUTPUT);    //设置引脚为输出模式
    // digitalWrite(B_led, HIGH); //默认引脚上电高电平
    Serial.begin(115200); //设置波特率115200
    ticker.attach_ms(50, onTimer);

    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
    delay(10);

    initStart();

    // turnOnLed();
    status_on = true;
    changeColor_("Red");
    setup_wifi();                                    //设置wifi的函数，连接wifi
    client.setServer(mqtt_server, mqtt_server_port); //设置mqtt服务器
    client.setCallback(callback);                    // mqtt消息处理
}
void loop()
{
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();
}

void initStart()
{
    for (int i = 0; i <= NUM_LEDS; i++)
    {
        int temp = (int)(255 / NUM_LEDS * i);
        leds[i] = CRGB(temp, temp, temp);
        FastLED.show();
        delay(50);
    }
    // turnOffLed();
    status_on = false;
    // turnOnLed();
    // delay(1000);
    // turnOffLed();
    // delay(1000);
    // changeColor(0, 255, 0);
    // changeBrightness(10);
    // delay(1000);
    // changeBrightness(100);
}

//设置灯带亮度
void changeBrightness(int bri)
{
    FastLED.setBrightness(25 * bri);
    FastLED.show();
}

//设置灯带颜色
void changeColor(int G, int R, int B)
{
    for (int i = 1; i <= NUM_LEDS; i++)
    {
        leds[i] = CRGB(R, G, B);

        // delay(40);
    }
    FastLED.show();
}
//打开灯泡
void turnOnLed()
{
    // Serial.println("turn on light");
    // color_R = 255;
    // color_G = 255;
    // color_B = 255;
    changeColor(color_G, color_R, color_B);
    // digitalWrite(B_led, LOW);
}
//关闭灯泡
void turnOffLed()
{
    // Serial.println("turn off light");
    // digitalWrite(B_led, HIGH);
    // color_R = 0;
    // color_G = 0;
    // color_B = 0;
    // changeColor_("Black");
    changeColor(0, 0, 0);
    FastLED.clear();
    changeColor(0, 0, 0);
    FastLED.show();
}
