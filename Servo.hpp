#include <Arduino.h>

class Servo32 {
public:
    // 构造函数，初始化舵机参数,引脚,频率,PWM通道,PWM分辨率
    Servo32(int Pin, int Freq = 50, int Channel = 9, int Resolution = 8);
    // 析构函数
    ~Servo32();
    // 设置舵机
    void setup();
    // 控制舵机角度
    void write(int angle);

private:
    int pin;
    int freq;
    int channel;
    int resolution;
    // 计算占空比
    int calculatePWM(int degree);
};

Servo32::Servo32(int Pin, int Freq, int Channel, int Resolution) {
    pin = Pin;
    freq = Freq;
    channel = Channel;
    resolution = Resolution;
}

Servo32::~Servo32() {
    
}

void Servo32::setup() {
    // 设置 PWM 输出
    ledcSetup(channel, freq, resolution);
    // 绑定引脚与 PWM 通道
    ledcAttachPin(pin, channel);
}

void Servo32::write(int angle) {
    // 控制舵机角度
    ledcWrite(channel, calculatePWM(angle));
}

int Servo32::calculatePWM(int degree) {
    float deadZone = 6.4;
    float max = 32;
    
    if (degree < 0)
        degree = 0;
    if (degree > 180)
        degree = 180;
    
    // 根据角度计算占空比
    return static_cast<int>(((max - deadZone) / 180) * degree + deadZone);
}
