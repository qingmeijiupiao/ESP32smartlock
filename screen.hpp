#include <Arduino.h>
#include <ESP32Servo.h>
#include <U8g2lib.h>
#include <Wire.h>
#include"pin.h"
#include "FPMXX.h"
#ifndef SCREEN_H
#define SCREEN_H
extern FPMXX fpm;
class Screen;
void Enroll();
class option{
    public:
        option(String name);
        void setup();
        void setup(void (*funcPtr)(void));
        void option_func();
        option* next_option;
        option* last_option;
        String name;
        // 声明函数指针类型
    private:
        void (*FuncPtr)(void);
};

class menu{
    public:
        menu(String name);
        void setup();
        void setup(void (*funcPtr)(Screen*,String));
        void menu_func(Screen* screen, String effects);
        bool have_option;
        menu* next_menu;
        menu* last_menu;
        String name;
        option* option_list;
        // 声明函数指针类型
    private:
        void (*FuncPtr)(Screen* screen, String effects);
};

class Screen{
    public:
        Screen();
        void setup(menu* now_m);
        void update(String effects);
        //void next_option();
        //void previous_option();
        //void next_menu();
        //void previous_menu();
        int get_voltage();
        void check_battery();
        void enter_option();
        void  back_to_menu();
        void clear();
        //void show_menu_delete_finger();
        //void show_menu_add_finger();
        //void show_menu_finger_info();
        //void show_menu_info();
        void show_voltage();
        void show_selection_arrow();
        void add_text(String text,int high);
        //void set_cursor(int location_id);

        U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI u8g2;
        //U8G2_SH1106_128X64_NONAME_F_4W_SW_SPI u8g2;

        menu* now_menu;
        bool is_in_menu;
        bool is_in_option;
    private:
        int now_option;
        int quantity;
        
        
};

#endif //SCREEN_H


Screen::Screen():u8g2(U8G2_R0, /* clock=*/ 5, /* data=*/ 23, /* cs=*/ 22, /* dc=*/ 18, /* reset=*/ 4){
    now_option = 1;
    is_in_menu = true;

};

void Screen::enter_option(){
    is_in_menu = false;
};

void Screen::back_to_menu(){
    is_in_menu = true;
};

int Screen::get_voltage(){
    int adc_val;
    float voltage;

    adc_val = analogRead(voltagepin);
    voltage = (((double)adc_val) / 4095) * 3.3;
    float b_voltage;
    b_voltage = 2*voltage + 0.35; //加0.35v的误差补偿
    int quantity = (b_voltage - 3.2) * 100;//以3.2v为没电状态换算成百分比
    //对上下限进行限制
    if (quantity >= 98) {
        quantity = 100;
    }

    if(quantity <= 1) {
        quantity = 0;
    }

    return quantity;
};

void Screen::check_battery(){
    //电池电压检测，电量到0%时蜂鸣器报警
    if (int(get_voltage()) == 0) {
    digitalWrite(buzzPin, HIGH);
  } else {
    digitalWrite(buzzPin, LOW);
    }
};

void Screen::setup(menu* now_m){
    now_menu = now_m; ;
    pinMode(buzzPin, OUTPUT);
    pinMode(voltagepin,INPUT);
    u8g2.begin();
    u8g2.setBusClock(200000);
    u8g2.enableUTF8Print();		// 启用UTF8打印函数支持
    u8g2.setFont(u8g2_font_wqy13_t_gb2312b);  // 使用 u8g2_font_wqy13_t_gb2312 字体渲染
    u8g2.setFontDirection(0);
};

void Screen::clear(){
    u8g2.clearBuffer();
    u8g2.sendBuffer();
};

void Screen::show_voltage(){
    u8g2.setCursor(100, 13);
    u8g2.print(String(get_voltage()) + "%");
};
void Screen::show_selection_arrow(){
    int Triangle_x=30;
    int Triangle_y=34;
    //u8g2.setCursor(40, 40);
    u8g2.drawTriangle(Triangle_x+70,Triangle_y, Triangle_x+70+10,Triangle_y+6, Triangle_x+70+10,Triangle_y-6);
    u8g2.drawTriangle(Triangle_x,Triangle_y, Triangle_x-10,Triangle_y+6, Triangle_x-10,Triangle_y-6);
};

void Screen::add_text(String text,int high){
    int center_x;
    int chineseCount=0;
    int englishCount=0;
    int numberCount=0;
    int i=0,len=text.length();
    for(;i<len;i++){
        if(text.charAt(i)>='0'&&text.charAt(i)<='9'){
            numberCount++;
        }else if (text.charAt(i)>'9'&& text.charAt(i)<='Z'){
            englishCount++;
        }
        else{
            chineseCount++;
            i+=2;
        }
    }
    //Serial.println(chineseCount);
    center_x=63-6*chineseCount-3*englishCount-4*numberCount;
    u8g2.setCursor(center_x, high);
    u8g2.print(text);
};
void Screen::update(String effects ){
   if (effects=="DOWN"){
        if(is_in_menu){
            for (int i =0;i<24;i+=2){
            u8g2.clearBuffer();
            show_voltage();
            show_selection_arrow();
            add_text(now_menu->last_menu->last_menu->last_menu->name,15-24-i);
            add_text(now_menu->last_menu->last_menu->name,15-i);
            add_text(now_menu->last_menu->name,39-i);
            add_text(now_menu->name,63-i);
            add_text(now_menu->next_menu->name,63+24-i);
            u8g2.sendBuffer();
            }
        }
   } else if (effects=="UP"){
        if(is_in_menu){
            for (int i =0;i<24;i+=2){
            u8g2.clearBuffer();
            show_voltage();
            show_selection_arrow();
            add_text(now_menu->last_menu->name,15-24+i);
            add_text(now_menu->name,15+i);
            add_text(now_menu->next_menu->name,39+i);
            add_text(now_menu->next_menu->next_menu->name,63+i);
            add_text(now_menu->next_menu->next_menu->next_menu->name,63+24+i);
            u8g2.sendBuffer();
            }
        }
   } else if (effects=="SELECT"){
        if (is_in_menu){
            is_in_menu = false;
            is_in_option = true;
        }
   } else if (effects=="BACK"){
       if (is_in_option){
           is_in_option = false;
           is_in_menu = true;
       }
       
   }
   if (is_in_menu&& !is_in_option){
    u8g2.clearBuffer();
    show_voltage();
    show_selection_arrow();
    add_text(now_menu->last_menu->name,15);
    add_text(now_menu->name,39);
    add_text(now_menu->next_menu->name,63);
    u8g2.sendBuffer();
   }else{
    now_menu->menu_func(this,effects);
   }

};



menu::menu(String name){
    this->name = name;
};
void menu::setup(void (*funcPtr)(Screen*,String)){
    FuncPtr=funcPtr;
};

void menu::setup(){
    FuncPtr=nullptr;
};

void menu::menu_func(Screen* screen, String effects){
    if (FuncPtr != nullptr) {
        (*FuncPtr)(screen, effects);
    }
};

option::option(String name){
    this->name = name;
};
void option::setup(){
    FuncPtr=nullptr;
};

void option::setup(void (*funcPtr)(void) ){
    FuncPtr=funcPtr;
};
void option::option_func(){
    (*FuncPtr)();
};


void add_finger_func(Screen* screen,String effects){
    Enroll();
    delay(10000);
    screen->clear();
    screen->add_text("添加指纹成功",20);
    screen->u8g2.sendBuffer();
    delay(1000);
    screen->clear();
    screen->is_in_option = false;
    screen->is_in_menu = true;
};

void finger_info_func(Screen* screen,String effects){
    screen->clear();
    screen->add_text("指纹数量:"+String(fpm.getFingerNum()),20);
    screen->u8g2.sendBuffer();
    if (effects=="BACK"){
        screen->clear();
        screen->is_in_option = false;
        screen->is_in_menu = true;
    }
};

menu add_finger(String("添加指纹"));  //菜单对象（添加指纹）
menu delete_finger(String("删除指纹"));  //菜单对象（删除指纹）
menu finger_info(String("指纹信息"));  //菜单对象（指纹信息）
menu info(String("信息"));  //菜单对象（信息）


