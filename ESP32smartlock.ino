#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "screen.hpp"
#include "FPMXX.h"
#include "MD5_String.hpp"

// 网页相关
#include "web.hpp"

// 引入所需的库
#include "pin.h"
#include "Servo.hpp"


// 开门和关门时的舵机角度
#define OPEN_ANGLE 110 /*根据测试情况修改*/
#define CLOSE_ANGLE 60 /*根据测试情况修改*/


// 定义wifi管理器对象
WiFiManager wm;
// 定义DNS服务器对象,用于自动弹出网页
DNSServer dns;

//创建一个屏幕对象
Screen screen; 
//创建一个舵机对象               
Servo32 myservo(servoPin);      
//创建一个指纹模块对象          
FPMXX fpm(&Serial2);

//网页相关
IPAddress local_IP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
const char* ntpServer = "ntp1.aliyun.com";
const long gmtOffset_sec = 8 * 3600;
const int daylightOffset_sec = 0;
const char* ssid = WIFI_SSID;          
const char* password = WIFI_PASSWORD;  

//一些全局变量
bool should_open = false;
bool down_pressed = false;    //按下按钮标志
bool select_pressed = false;  //按下按钮标志
bool up_pressed = false;      //按下按钮标志
bool back_pressed = false;    //按下按钮标志
bool should_show;             //是否显示屏幕内容
String eff = "";
int timer =0; //计时器变量


   

// 设置蜂鸣器发声的时间，默认为100毫秒
void buzz(int time = 100) {
  // 将蜂鸣器引脚设置为高电平，使其发声
  digitalWrite(buzzPin, HIGH);
  // 延迟指定的时间
  delay(time);
  // 将蜂鸣器引脚设置为低电平，停止发声
  digitalWrite(buzzPin, LOW);
};
// 打开锁函数
void open_lock() {
  buzz();
  myservo.write(OPEN_ANGLE);
  delay(1000);
  myservo.write(CLOSE_ANGLE);
  delay(1000);
  should_open = false;
};
// 注册函数
void Enroll() {
  // 调用buzz函数，让蜂鸣器发声
  buzz(100);
  // 取消自动模式
  fpm.Cancel_Auto();
  // 延迟100毫秒
  delay(100);
  // 开始自动注册
  fpm.AutoEnroll();
  // 延迟，等待注册完成
  delay(MAX_SCRENN_SHOW_TIME);
}


void MAIN(void* xTask1) {
  while (1)
  {
        //更新按钮
    down_pressed = digitalRead(down_buttonPin);
    select_pressed = digitalRead(select_buttonPin);
    up_pressed = digitalRead(up_buttonPin);
    back_pressed = digitalRead(back_buttonPin);
    eff = "";

// 如果按下了向下、选择、向上或返回键
if (down_pressed || select_pressed || up_pressed || back_pressed) {
  // 重置计时器
  timer = 0;
  
  // 如果应该显示内容
  if (should_show) {
    // 调用buzz函数，让蜂鸣器发声
    buzz(100);
    
    // 如果按下了向上键
    if (up_pressed) {
      // 切换到下一个菜单
      screen.now_menu = screen.now_menu->last_menu;
      eff = "UP";
      Serial.println("up");
    }
    
    // 如果按下了向下键
    if (down_pressed) {
      // 切换到上一个菜单
      screen.now_menu = screen.now_menu->next_menu;
      eff = "DOWN";
    }
    
    // 如果按下了返回键
    if (back_pressed) {
      eff = "BACK";
    }
    
    // 如果按下了选择键
    if (select_pressed) {
      eff = "SELECT";
      // 执行当前菜单的功能
      //screen.now_menu->menu_func();
    }
  }
  // 设置为应该显示内容
  should_show = true;
}

    if (should_show) {
      if(timer>MAX_SCRENN_SHOW_TIME){
        should_show=false;//超时息屏
      }
      screen.update(eff);
    } else {
      screen.clear();
      timer=0;
    }
    //延时不能注释掉，否则会无限重启
    delay(100);

  }

}


void FPM(void* xTask2) {
  while (1) {
    // 如果不应该显示内容
    if (!should_show) {
      // 如果自动识别的结果大于1000
      if (fpm.AutoIdentify() > 1000) {
        should_open = true; //开门
      }
      // 延迟500毫秒
      delay(500);
    }
    // 延迟100毫秒
    delay(100);
  }
}

//计时器线程
void Timer(void* xTask3){
  while(1){
    timer+=100;
    delay(100);
  }
}


// 初始化函数
void setup() {
  add_finger.setup(add_finger_func);
  finger_info.setup(finger_info_func);
  Serial.println("setup");
  screen.setup(&add_finger);


  if(AP_MODE){
    // 创建wifi访问点，设置名称和密码
    WiFi.softAP(ssid, password);
    // 获取wifi访问点的IP地址
    IPAddress apIP = WiFi.softAPIP();
    // 启动DNS服务器，将所有域名解析到wifi访问点的IP地址，实现重定向功能
    dns.start(53, "*", apIP);
  }else{
    // 连接WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED && timer<5000) {
      delay(1000);
      timer+=1000;
      screen.u8g2.clearBuffer();
      screen.add_text("Connecting to WiFi...", 35);
      screen.u8g2.sendBuffer();
    };
    timer=0;
    if (WiFi.status() == WL_CONNECTED) {
      // 显示本地IP地址
      screen.u8g2.clearBuffer();
      screen.u8g2.setCursor(20, 31);
      screen.u8g2.print(WiFi.localIP());
      screen.u8g2.sendBuffer();
      Serial.println(WiFi.localIP());
      delay(3000);
    };
  }
  









  server.on("/", handleRoot); // 根目录请求
  server.on("/submit", handleSubmit); // 提交请求
  server.on("/gen_204", handleDetect); // Google设备检测请求
  server.on("/hotspot-detect.html", handleDetect); // Apple设备检测请求
  server.onNotFound(handleNotFound); // 其他不存在的请求
  server.begin();

  //获取时间
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  // 设置菜单链表
  add_finger.next_menu = &delete_finger;
  delete_finger.next_menu = &finger_info;
  finger_info.next_menu = &info;
  info.next_menu = &add_finger;

  add_finger.last_menu = &info;
  info.last_menu = &finger_info;
  finger_info.last_menu = &delete_finger;
  delete_finger.last_menu = &add_finger;

  Serial.begin(9600);
  Serial2.begin(57600);

  myservo.setup();


  // 设置按钮引脚为下拉输入模式
  pinMode(down_buttonPin, INPUT_PULLDOWN);
  pinMode(select_buttonPin, INPUT_PULLDOWN);
  pinMode(up_buttonPin, INPUT_PULLDOWN);
  pinMode(back_buttonPin, INPUT_PULLDOWN);
  pinMode(wake_uppin, INPUT_PULLDOWN);
  //open_lock();

  // 创建并启动任务
  xTaskCreatePinnedToCore(MAIN, "main", 20*1024, NULL,1, NULL, 0);
  xTaskCreatePinnedToCore(FPM, "FPM", 4*1024, NULL, 2, NULL, 1);  //Task2在 1核心
  xTaskCreatePinnedToCore(Timer, "Timer", 1024, NULL,0, NULL, 1);
};

void loop() {
  if(AP_MODE){
    dns.processNextRequest(); // 处理DNS请求
  }
  server.handleClient(); // 处理web请求
  if (should_open) {
    open_lock();
  }
  delay(100);
};
