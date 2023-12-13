//网页相关库
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

/*根据需要修改*/
#define AP_MODE true                  //热点模式为true，wifi模式为false 
#define WIFI_SSID "wifi名称"        //WiFi名称
#define WIFI_PASSWORD ""                 //WiFi密码
#define DOORPASSWORD  "PASSWORD"              //开门密码,wifi模式密码为：当前日期的hash前四位 比如2023-09-02取md5算法hash前四位为：cb2e

//开门标志
extern bool should_open;

//网页内容，根据需要修改
const char* htmlContent = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta charset="UTF-8">
  <title>指纹门锁</title>
  <style>
    
    body {
      font-size: 64px;
      font-family: Arial, sans-serif;
    }
    
    .container {
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      height: 100vh;
      background: linear-gradient(#f2e9e4, #c9ada7);
    }
    h2 {
    font-size: 96px; /* 修改字体大小 */
    color: #22223b; /* 修改字体颜色 */
    text-align: center; /* 修改文本对齐方式 */
    /* 添加其他样式属性 */
    }

    .background {
      position: absolute;
      top: 0;
      left: 0;
      width: 100%;
      height: 100%;
      filter: blur(10px);
      z-index: -1;
    }
    
    .input-container {
      display: flex;
      flex-direction: column;
      align-items: center;
      margin-bottom: 20px;
    }
    
    input[type="password"] {
    font-size: 45px;
    width: 500px;
    padding: 40px;
    color: #838383;
    border-radius: 50px;
    box-shadow: 0 8px 28px #22223b;
    border: 5px solid #22223b; /* 统一指定边框样式 */
    }

    input[type="password"]:focus {
    outline: none;
    font-size: 45px;
    width: 500px;
    padding: 40px;
    color: #4a4e69;
    border-radius: 50px;
    box-shadow: 0 8px 28px #ffffff;
    border: 5px solid #4a4e69; /* 统一指定选中时的边框样式 */
    }
    button[type="button"] {
      font-size: 64px;
      font-weight: bold;
      padding: 60px 180px;
      background-color: #22223b;
      color: #ffffff;
      border: none;
      cursor: pointer;
      border-radius: 120px;
      box-shadow: 0 8px 28px #4a4e69;
    }

    button[type="button"]:active {
      font-size: 64px;
      font-weight: bold;
      padding: 60px 180px;
      background-color: #4a4e69;
      color: #f0eaef;
      border: none;
      cursor: pointer;
      border-radius: 120px;
      box-shadow: 0 8px 28px #4a4e69;
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="background"></div>
    <div id="message"></div>
    <h2>请输入开门密钥</h2>
    <div class="input-container">
      <input type="password" name="inputData" placeholder="请输入开门密钥">
    </div>
    <button type="button" onclick="submitForm()">确认</button>
  </div>
  <script>
    function submitForm() {
      // 获取输入框的值
      var Data = document.querySelector('input[name="inputData"]').value;
      // 创建一个新的XMLHttpRequest对象
      var xhr = new XMLHttpRequest();

      // 设置请求的类型、URL和异步标志
      xhr.open('POST', '/submit', true);

      // 设置请求头
      xhr.setRequestHeader('Content-Type','text');

      // 定义请求完成时的回调函数
      xhr.onload = function() {
        if (xhr.status === 300) {
          // 请求成功，显示成功消息
          window.alert('开门成功');
        } else {
          // 请求失败，显示失败消息
          window.alert('密码错误');
        }
      };
      // 将输入数据转换为JSON字符串
      var jsonData = JSON.stringify({ inputData: Data });
      
      // 发送请求，将输入数据作为请求体发送到服务器
      xhr.send(jsonData);
    }
  </script>
</body>
</html>

)rawliteral";
//创建一个异步Web服务器
WebServer server(80);  

//开门密码，根据需要修改密码生成方式
String generate_password() {
  if (!AP_MODE){
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)){
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d", &timeinfo);
    String hash = md5_string(timeStr);
    hash=hash.substring(0, 4);
    //Serial.println("scr:"+hash);
    return hash;//密码为当前时间的hash前四位 比如2023-09-02取md5算法hash前四位为：cb2e
    }
  }
  return DOORPASSWORD;
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  char timeStr[20];
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d", &timeinfo);
  String timeString(timeStr);
  Serial.println(timeString);
};


void handleRoot() {
  // 发送网页内容
    server.send(200, "text/html", htmlContent);
}

void handleSubmit() {
// 获取请求的内容类型
  String inputData = server.arg(0);
  DynamicJsonDocument jsonDoc(1024);   // 创建一个JSON文档对象，适当调整大小
  // 解析JSON文档
  DeserializationError error = deserializeJson(jsonDoc, inputData);
  if (error) {
    return ;
  }
  inputData = jsonDoc["inputData"].as<String>();
  Serial.println(String(inputData));
  // 如果请求的内容类型是generate_password()返回的值
  if (inputData == generate_password()){
    // 发送300状态码和文本响应
    server.send(300, "text/plain");
    // 设置should_open为true，表示需要打开锁
    should_open = true;
  } else {
    // 否则，发送200状态码和文本响应
    server.send(200, "text/plain");
  }
}

// 处理设备检测的请求
void handleDetect() {
  // 发送一个空白的响应，表示已经连接到网络，不需要登录
  server.send(204);
}

// 处理不存在的请求
void handleNotFound() {
  // 重定向到根目录，弹出网页
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}


