/*
*Author:qingmeijiupiao
*2023/7/13
*link:https://github.com/qingmeijiupiao/
*基于下面作者的库修改和完善而来
* Author: Chenhe 晨鹤
* Created: 2017/5/28
* Blog: http://chenhe.cc
* 
经测试FPM383 系列可以使用 FPM133系列不能使用
*/


#if ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "FPMXX.h"

/*
使用默认地址：0xFFFFFFFF，默认波特率：576000.
*/
FPMXX::FPMXX(HardwareSerial * Serial)
{
	mySerial = Serial;
	mySerial->begin(57600);
	
}

FPMXX::FPMXX(HardwareSerial * Serial, uint8_t adder[])
{
	mySerial = Serial;
	mySerial->begin(57600);
	
	for (int i = 0; i < 4; i++)
	{
		Adder[i] = adder[i];
	}
}

FPMXX::FPMXX(HardwareSerial * Serial, uint16_t baudRate)
{
	mySerial = Serial;
	mySerial->begin(baudRate);

}

FPMXX::FPMXX(HardwareSerial * Serial, uint8_t adder[], uint16_t baudRate)
{
	mySerial = Serial;
	mySerial->begin(baudRate);
	
	for (int i = 0; i < 4; i++)
	{
		Adder[i] = adder[i];
	}
}



void FPMXX::setup(){
	while (true)
	{
		uint8_t r = verifyPassword();
		if(r==FPMXX_CODE_OK){
			break;
		}
	}
	getFingerStoreSize();
	getFingerNum();
}

void FPMXX::update(){
	getFingerStoreSize();
	getFingerNum();
}





/*
设置模块地址，用于通讯。默认为0xFFFFFFFF。
本函数仅设置库内部的变量以便通讯，并不是修改模块的地址。
Params:
address: 模块地址，长度为4。
*/
void FPMXX::setCurrentAddress(uint8_t address[])
{
	for (int i = 0; i < 4; i++)
		Adder[i] = address[i];
}

/*
获取指纹容量。
Params:
num: 储存指纹容量。
*/
uint8_t FPMXX::getFingerStoreSize()
{
	uint8_t content[] = { 0x01,0x00,0x03,0x0F };
	sendCmd(3, content);
	uint8_t reply[17];
	uint8_t r = getReply(reply);
	if (r == 17)
	{
		/*Serial.println("--------");
		for (int i = 0;i < 17;i++)
		Serial.println(reply[i],HEX);
		Serial.println("--------");*/
		if (reply[0] != FPMXX_CODE_OK) return FPMXX_CODE_DATA_ERROR;
		sensorFingerStoreSize = reply[5];
		sensorFingerStoreSize <<= 8;
		sensorFingerStoreSize |= reply[6];
		return reply[0];
	}
	return sensorFingerStoreSize;
}

/*
获取已储存的指纹个数。
Params:
num: 储存个数。
*/
uint8_t FPMXX::getFingerNum(){ 
  //Cancel_Auto();
	uint8_t content[] = { 0x01,0x00,0x03,0x1D };
	sendCmd(3, content);
	uint8_t r = getReply(content);
	if (r == 3)
	{
		if (content[0] != FPMXX_CODE_OK)
		 return FPMXX_CODE_DATA_ERROR;
		FingerNum = content[1];
		FingerNum <<= 8;
		FingerNum |= content[2];
		return FingerNum;
	}
	return FingerNum;
}

/*
设置模块安全等级，等级越高拒认率越高。
重启后生效。
Params:
level: 0~5
*/
uint8_t FPMXX::setSecurityLevel(uint8_t level)
{
	uint8_t content[] = { 0x01,0x00,0x05,0x0E,0x05,0x00 };
	content[5] = level;
	sendCmd(5, content);
	uint8_t r = getReply(content);
	if (r == 1)
		return content[0];
	return r;
}

// int FPMXX::getSecurityLevel(){
// 	uint8_t content[] = { 0x01,0x00,0x03,0x0F };
// 	sendCmd(3, content);
// 	uint8_t r = getReply(content);
// 	if (r == 1)
// 		return content[0];
// 	return r;
// }



/*
设置模块口令。
Params:
password: 新口令，长度为4。
*/
uint8_t FPMXX::setPassword(uint8_t password[])
{
	uint8_t content[] = { 0x01,0x00,0x07,0x12,0x00,0x00,0x00,0x00 };
	for (int i = 0; i < 4; i++)
		content[4 + i] = password[i];
	sendCmd(7, content);
	uint8_t r = getReply(content);
	if (r == 1)
		return content[0];
	return r;
}

/*
验证模块口令，使用默认:0x00000000
*/
uint8_t FPMXX::verifyPassword()
{
	uint8_t content[] = { 0x01,0x00,0x07,0x13,0x00,0x00,0x00,0x00 };
	sendCmd(7, content);
	uint8_t r = getReply(content);
	if (r == 1)
		return content[0];
	return r;
}

/*
验证模块口令。
Params:
password: 口令，长度为4。
*/
uint8_t FPMXX::verifyPassword(uint8_t password[])
{
	uint8_t content[] = { 0x01,0x00,0x07,0x13,0x00,0x00,0x00,0x00 };
	for (int i = 0; i < 4; i++)
		content[4 + i] = password[i];
	sendCmd(7, content);
	uint8_t r = getReply(content);
	if (r == 1)
		return content[0];
	return r;
}

/*
录入指纹图像。
探测到后录入指纹图像存于模块ImageBuffer
*/
uint8_t FPMXX::getFingerImage()
{
	uint8_t content[] = { 0x01,0x00,0x03,0x01 };
	sendCmd(3, content);
	uint8_t r = getReply(content, 1000);
	if (r == 1){
		if (content[0]==FPMXX_CODE_OK){
			return content[0];
		}
	}
	else
		getFingerImage();
	
}

/*
将ImageBuffer图像生成指纹特征。
存于 CharBuffer1 或 CharBuffer2
Params:
bufferId:BufferID 表示此次提取的特征存放在缓冲区中的位置 ，其他值按2处理。
*/
bool FPMXX::image2tz(uint8_t bufferId)
{
	uint8_t content[] = { 0x01,0x00,0x04,0x02,0x00 };
	content[4] = bufferId;
	sendCmd(4, content);
	uint8_t r = getReply(content);
	if (r == 1)
		if (content[0] == FPMXX_CODE_OK)
		return true;
	return false;
}

/*
将 CharBuffer的特征文件合并生成模板。
结果存于 CharBuffer1
*/
bool FPMXX::buildTemplate()
{
	uint8_t content[] = { 0x01,0x00,0x03,0x05 };
	sendCmd(3, content);
	uint8_t r = getReply(content);
	if (r == 1){
		if (content[0]==FPMXX_CODE_OK){
			return true;
		}
	}
	return false;
}

/*
储存指纹。
Params:
bufferId: 特征bufferId。
pageId: 指纹库位置，从0开始。
*/
bool FPMXX::storeFinger(uint8_t bufferId, uint16_t pageId)
{
	uint8_t content[] = { 0x01,0x00,0x06,0x06,0x00,0x00,0x00 };
	content[4] = bufferId;
	content[5] = (uint8_t)(pageId >> 8);
	content[6] = (uint8_t)pageId;
	sendCmd(6, content);
	uint8_t r = getReply(content);
	if (r == 1){
		if (content[0]==FPMXX_CODE_OK){
			return true;
		}
	}
	return false;
		

}

/*
读取指纹。
将 flash 数据库中指定 ID 号的指纹模板读入到模板缓冲区。
Params:
bufferId: 特征bufferId，1~2。
pageId: 指纹库位置，从0开始。
*/
bool FPMXX::loadFinger(uint8_t bufferId, uint16_t pageId)
{
	uint8_t content[] = { 0x01,0x00,0x06,0x07,0x00,0x00,0x00 };
	content[4] = bufferId;
	content[5] = (uint8_t)(pageId >> 8);
	content[6] = (uint8_t)pageId;
	sendCmd(6, content);
	uint8_t r = getReply(content);
	if (r == 1){
		if (content[0]==FPMXX_CODE_OK){
			return true;
		}
	}
	return r;
}

/*
删除指纹。
删除指定 ID 号开始的 N 个指纹模板。
Params:
pageId: 指纹库位置，从0开始。
count: 要删除的个数。
*/
bool FPMXX::deleteFinger(uint16_t pageId, uint16_t count)
{
	uint8_t content[] = { 0x01,0x00,0x07,0x0C,0x00,0x00,0x00,0x00 };
	content[4] = (uint8_t)(pageId >> 8);
	content[5] = (uint8_t)pageId;
	content[6] = (uint8_t)(count >> 8);
	content[7] = (uint8_t)count;
	sendCmd(7, content);
	uint8_t r = getReply(content);
	if (r == 1){
		if (content[0]==FPMXX_CODE_OK){
			return true;
		}
	}
	return false;
}

/*
清空指纹库。
*/
bool FPMXX::emptyFinger()
{
	uint8_t content[] = { 0x01,0x00,0x03,0x0D };
	sendCmd(3, content);
	uint8_t r = getReply(content);
	if (r == 1){
		if (content[0]==FPMXX_CODE_OK){
			return true;
		}
	}
	return false;
}

/*
精确比对 CharBuffer1 与 CharBuffer2 中的特征文件。
两特征缓冲区中的内容不变。
Params:
score: 记录比对得分。
*/
int FPMXX::contrastFinger()
{
	int score = 0;
	uint8_t content[] = { 0x01,0x00,0x03,0x03 };
	sendCmd(3, content);
	uint8_t r = getReply(content);
	if (r == 3) {
		if (content[0] != FPMXX_CODE_OK) score = 0;
		else {
			score = content[1];
			score <<= 8;
			score |= content[2];
		}
		return score;
	}
	return r;
}

/*
以buffer中的特征搜索指纹库。
Params:
bufferId: 特征bufferId。
startPage: 开始页面。
endPage: 结束页码。
pageId: 记录搜索到的页码。
score: 记录比对得分。
*/
int FPMXX::searchFinger(uint8_t bufferId, uint16_t startPage, uint16_t endPage, uint16_t pageId, uint16_t score)
{
	uint8_t content[] = { 0x01,0x00,0x08,0x04,0x00,0xFF,0xFF,0x00,0x00 };
	content[4] = bufferId;
	content[5] = (uint8_t)(startPage >> 8);
	content[6] = (uint8_t)startPage;
	content[7] = (uint8_t)(endPage >> 8);
	content[8] = (uint8_t)endPage;
	sendCmd(8, content);
	uint8_t r = getReply(content);
	if (r == 5) {
		if (content[0] != FPMXX_CODE_OK) score = 0;
		else {
			pageId = content[1];
			pageId <<= 8;
			pageId |= content[2];
			score = content[3];
			score <<= 8;
			score |= content[4];
		}
		return content[0];
	}
	return r;
}

int FPMXX::searchFinger(uint8_t bufferId, uint16_t startPage, uint16_t endPage, uint16_t  pageId)
{
	uint16_t s=0;
	return searchFinger(bufferId, startPage, endPage, pageId, s);
}

/*
自动注册指纹
参数：最低位为 bit0。
1) bit0：采图背光灯控制位，0-LED 长亮，1-LED 获取图像成功后灭；
2) bit1：采图预处理控制位，0-关闭预处理，1-打开预处理；
3) bit2：注册过程中，是否要求模组在关键步骤，返回当前状态，0-要求返回，1-不
要求返回；
4) bit3：是否允许覆盖 ID 号，0-不允许，1-允许；
5) bit4：允许指纹重复注册控制位，0-允许，1-不允许；
6) bit5：注册时，多次指纹采集过程中，是否要求手指离开才能进入下一次指纹图
像采集， 0-要求离开；1-不要求离开；
7) bit6~bit15：预留
*/
void FPMXX::AutoEnroll(uint8_t Enroll_num,int FingerId,bool bit0,bool bit1,bool bit2,bool bit3,bool bit4,bool bit5){
	FingerNum=getFingerNum();
  delay(100);
	uint16_t pageId;
	if (FingerId==0)
		pageId=FingerNum;
	else
		pageId=FingerId;
  
	uint8_t content[] = { 0x01,0x00,0x08,0x31,0x00,0x00,0xFF,0x00,0x01 };
	content[4]= pageId >> 8;
	content[5]= pageId;
	content[6]=Enroll_num;
	//计算参数位
	content[8]=uint8_t(1*bit0+2*bit1+4*bit2+8*bit3+16*bit4+32*bit5);
	
	sendCmd(8, content);
	uint8_t r = getReply(content);
};


/*自动指纹识别*/
int  FPMXX::AutoIdentify(){
	uint8_t content[] = { 0x01,0x00,0x08,0x32,0x01,0xFF,0xFF,0x00,0x07};
	sendCmd(8, content);
	uint8_t r = getReply(content);
	if (r == 6) {
		if (content[0] == FPMXX_CODE_OK){
			return content[4]*256+content[5];
		}
		else {
			return 0;
		}
		
	}
}

bool FPMXX::Cancel_Auto(){
	uint8_t content[] = { 0x01,0x00,0x03,0x30};
	sendCmd(3, content);
	uint8_t r = getReply(content);
	if (r == 1) {
		if(content[0]==FPMXX_CODE_OK){
			return true;
		}
	}
	return false;
}


/*
设置传感器进入休眠模式
*/
bool FPMXX::sleep(bool LED_OFF){
	uint8_t content[] = { 0x01,0x00,0x03,0x33 };
	while (true)
	{
		sendCmd(3, content);
		uint8_t r = getReply(content);
		if (r == 1) {
			if (content[0] == FPMXX_CODE_OK){
				if (LED_OFF)
					LEDOFF();
				return true;
			}
		}
	}
}


/*功能说明： 控制灯指令主要分为两类：一般指示灯和七彩编程呼吸灯。
输入参数： 功能码，起始颜色，结束颜色，循环次数
返回参数： 确认字
指令代码： 3CH
辅助说明
功能码：LED 灯模式控制位，1-普通呼吸灯，2-闪烁灯，3-常开灯，4-常闭灯，5-渐开灯，
6-渐闭灯，其他功能码不适用于此指令包格式；
起始颜色：设置为普通呼吸灯时，由灭到亮的颜色，只限于普通呼吸灯（功能码 01）功能，
其他功能时，与结束颜色保持一致。其中，bit0 是蓝灯控制位；bit1 是绿灯控制位；bit2 是
红灯控制位。置 1 灯亮，置 0 灯灭。例如 0x01_蓝灯亮，0x02_绿灯亮，0x04_红灯亮，0x06_红绿
灯亮，0x05_红蓝灯亮，0x03_绿蓝灯亮，0x07_红绿蓝灯亮，0x00_全灭；
结束颜色：设置为普通呼吸灯时，由亮到灭的颜色，只限于普通呼吸灯（功能码 0x01），其
他功能时，与起始颜色保持一致。设置方式与起始颜色一样；
循环次数：表示呼吸或者闪烁灯的次数。当设为 0 时，表示无限循环，当设为其他值时，
表示呼吸有限次数。循环次数适用于呼吸、闪烁功能，其他功能中无效，例如在常开、常闭、渐
开和渐闭中是无效的；
*/
bool FPMXX::NormalLEDControl(uint8_t FuncCode,uint8_t begainColor,uint8_t endColor,uint8_t LoopNum){
	uint8_t content[] = { 0x01,0x00,0x07,0x3C,0x00,0x00,0x00,0x00};
	content[4] = FuncCode;
	content[5] = begainColor;
	content[6] = endColor;
	content[7] = LoopNum;
	sendCmd(7, content);
	uint8_t r = getReply(content);
	if (r == 1) {
			if (content[0] == FPMXX_CODE_OK)
				return true;
			else 
				return false;
		}
}
//关灯
bool FPMXX::LEDOFF(){
	//循环执行确保成功，最高尝试10次
	for(int i=0;i<10;i++){
		if (NormalLEDControl(0x04,0x00,0x00,0x00))
			return true;
	}
	return false ;
}

//简单呼吸灯控制函数
bool FPMXX::LEDBreathing(String BeginColor,String EndColor,uint8_t LoopNum){
	uint8_t content[] = { 0x01,0x00,0x07,0x3C,0x01,0x00,0x00,0x00};
	//计算循环次数
	content[7] = LoopNum;
	//计算开始颜色
	if (BeginColor=="RED")
		content[5] = 0x04;
	else if(BeginColor=="GREEN")
		content[5] = 0x02;
	else if (BeginColor=="BLUE")
		content[5] = 0x01;
	else if(BeginColor=="REDGREEN" || BeginColor=="GREENBLUE")
		content[5] = 0x06;
	else if (BeginColor=="REDBLUE" || BeginColor=="BLUERED" )
		content[5] = 0x05;
	else if (BeginColor=="BLUEGREEN" || BeginColor=="GREENBLUE")
		content[5] = 0x03;
	else if (BeginColor=="ALL")
		content[5] = 0x07;
	else
		content[5] = 0x00;
	//计算结束颜色
	if (EndColor=="RED")
		content[6] = 0x04;
	else if(EndColor=="GREEN")
		content[6] = 0x02;
	else if (EndColor=="BLUE")
		content[6] = 0x01;
	else if(EndColor=="REDGREEN" || EndColor=="GREENBLUE")
		content[6] = 0x06;
	else if (EndColor=="REDBLUE" || EndColor=="BLUERED" )
		content[6] = 0x05;
	else if (EndColor=="BLUEGREEN" || EndColor=="GREENBLUE")
		content[6] = 0x03;
	else if (EndColor=="ALL")
		content[6] = 0x07;
	else
		content[6] = 0x00;
	//发送命令
	sendCmd(7, content);
	uint8_t r = getReply(content);
	if (r == 1) {
			if (content[0] == FPMXX_CODE_OK)
				return true;
			else
	sendCmd(7, content);
	uint8_t r = getReply(content);
	if (r == 1) {
			if (content[0] == FPMXX_CODE_OK)
				return true;
			else 
				return false;
		}
	
	}
}
/* 灯控制-复杂版
功能码：7-七彩编程呼吸灯，其他功能码不适用于此指令包格式；
时间位：用来控制灯呼吸一次的时间，即从灭到亮，再到灭的时间。单次呼吸的时间范围
是 0.1s~10.0s 左右，用 1~100 之间的数字表示，超出此范围的数字无效。即时间位设为 1 对
应0.1s，时间位设为 100 对应 10.0s。推荐时间位设置为 36，呼吸时间与普通呼吸灯（功能码
0x01）相同，大概是 3.6s 左右。
颜色码：共 5 个字节组成，如下表所示，每个字节的颜色码分成 2 个单元，每个单元有 4
位，从高位开始分为一个有效位，和 3 个颜色控制位。每个单元控制某种颜色的灯从灭到
亮，再到灭的过程。另外，编程后的呼吸灯 1 次循环点亮顺序是从颜色 1 的单元 1 开始，然
后是颜色 1的单元 2，接着是颜色 2 的单元 1，以此类推
*/
bool FPMXX::ComplexLEDControl(
    uint8_t time,
    String begin_Color1,
    String end_Color1,
    String begin_Color2,
    String end_Color2,
    String begin_Color3,
    String end_Color3,
    String begin_Color4,
    String end_Color4,
    String begin_Color5,
    String end_Color5,
    uint8_t LoopNum) {


    uint8_t content[] = {0x01, 0x00 ,0x0B, 0x3C, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    // 设置时间位，控制呼吸一次的时间
    if (time > 100)
        content[5] = 100;
    else if (time < 1)
        content[5] = 1;
    else
        content[5] = time;
    
    // 设置单元1有效位和颜色
    if (begin_Color1 != "NONE")
    {
        content[6] |= (1 << 7); // 将第7位设置为1，表示单元1有效

        if (begin_Color1 == "RED")
            content[6] |= (1 << 6); // 将第6位设置为1，表示红色
        else if (begin_Color1 == "GREEN")
            content[6] |= (1 << 5); // 将第5位设置为1，表示绿色
        else if (begin_Color1 == "BLUE")
            content[6] |= (1 << 4); // 将第4位设置为1，表示蓝色
        else if (begin_Color1 == "REDGREEN" || begin_Color1 == "GREENRED")
            content[6] |= (1 << 6) | (1 << 5); // 将第6位和第5位同时设置为1，表示红绿双色
        else if (begin_Color1 == "REDBLUE" || begin_Color1 == "BLUERED")
            content[6] |= (1 << 6) | (1 << 4); // 将第6位和第4位同时设置为1，表示红蓝双色
        else if (begin_Color1 == "BLUEGREEN" || begin_Color1 == "GREENBLUE")
            content[6] |= (1 << 5) | (1 << 4); // 将第5位和第4位同时设置为1，表示绿蓝双色
        else if (begin_Color1 == "ALL")
            content[6] |= (1 << 6) | (1 << 5) | (1 << 4); // 将第6位、第5位和第4位同时设置为1，表示全色
    }

    // 设置单元2有效位和颜色
    if (end_Color1 != "NONE")
    {
        content[6] |= (1 << 3); // 将第3位设置为1，表示单元2有效

        if (end_Color1 == "RED")
            content[6] |= (1 << 2); // 将第2位设置为1，表示红色
        else if (end_Color1 == "GREEN")
            content[6] |= (1 << 1); // 将第1位设置为1，表示绿色
        else if (end_Color1 == "BLUE")
            content[6] |= (1 << 0); // 将第0位设置为1，表示蓝色
        else if (end_Color1 == "REDGREEN" || end_Color1 == "GREENBLUE")
            content[6] |= (1 << 2) | (1 << 1); // 将第2位和第1位同时设置为1，表示红绿双色
        else if (end_Color1 == "REDBLUE" || end_Color1 == "BLUERED")
            content[6] |= (1 << 2) | (1 << 0); // 将第2位和第0位同时设置为1，表示红蓝双色
        else if (end_Color1 == "BLUEGREEN" || end_Color1 == "GREENBLUE")
            content[6] |= (1 << 1) | (1 << 0); // 将第1位和第0位同时设置为1，表示绿蓝双色
        else if (end_Color1 == "ALL")
            content[6] |= (1 << 2) | (1 << 1) | (1 << 0); // 将第2位、第1位和第0位同时设置为1，表示全色
    }
        // 设置单元3有效位和颜色
    if (begin_Color2 != "NONE")
    {
        content[7] |= (1 << 7); // 将第7位设置为1，表示单元3有效

        if (begin_Color2 == "RED")
            content[7] |= (1 << 6); // 将第6位设置为1，表示红色
        else if (begin_Color2 == "GREEN")
            content[7] |= (1 << 5); // 将第5位设置为1，表示绿色
        else if (begin_Color2 == "BLUE")
            content[7] |= (1 << 4); // 将第4位设置为1，表示蓝色
        else if (begin_Color2 == "REDGREEN" || begin_Color2 == "GREENRED")
            content[7] |= (1 << 6) | (1 << 5); // 将第6位和第5位同时设置为1，表示红绿双色
        else if (begin_Color2 == "REDBLUE" || begin_Color2 == "BLUERED")
            content[7] |= (1 << 6) | (1 << 4); // 将第6位和第4位同时设置为1，表示红蓝双色
        else if (begin_Color2 == "BLUEGREEN" || begin_Color2 == "GREENBLUE")
            content[7] |= (1 << 5) | (1 << 4); // 将第5位和第4位同时设置为1，表示绿蓝双色
        else if (begin_Color2 == "ALL")
            content[7] |= (1 << 6) | (1 << 5) | (1 << 4); // 将第6位、第5位和第4位同时设置为1，表示全色
    }

    // 设置单元4有效位和颜色
    if (end_Color2 != "NONE")
    {
        content[7] |= (1 << 3); // 将第3位设置为1，表示单元4有效

        if (end_Color2 == "RED")
            content[7] |= (1 << 2); // 将第2位设置为1，表示红色
        else if (end_Color2 == "GREEN")
            content[7] |= (1 << 1); // 将第1位设置为1，表示绿色
        else if (end_Color2 == "BLUE")
            content[7] |= (1 << 0); // 将第0位设置为1，表示蓝色
        else if (end_Color2 == "REDGREEN" || end_Color2 == "GREENBLUE")
            content[7] |= (1 << 2) | (1 << 1); // 将第2位和第1位同时设置为1，表示红绿双色
        else if (end_Color2 == "REDBLUE" || end_Color2 == "BLUERED")
            content[7] |= (1 << 2) | (1 << 0); // 将第2位和第0位同时设置为1，表示红蓝双色
        else if (end_Color2 == "BLUEGREEN" || end_Color2 == "GREENBLUE")
            content[7] |= (1 << 1) | (1 << 0); // 将第1位和第0位同时设置为1，表示绿蓝双色
        else if (end_Color2 == "ALL")
            content[7] |= (1 << 2) | (1 << 1) | (1 << 0); // 将第2位、第1位和第0位同时设置为1，表示全色
    }

    // 设置单元5有效位和颜色
    if (begin_Color3 != "NONE")
    {
        content[8] |= (1 << 7); // 将第7位设置为1，表示单元5有效

        if (begin_Color3 == "RED")
            content[8] |= (1 << 6); // 将第6位设置为1，表示红色
        else if (begin_Color3 == "GREEN")
            content[8] |= (1 << 5); // 将第5位设置为1，表示绿色
        else if (begin_Color3 == "BLUE")
            content[8] |= (1 << 4); // 将第4位设置为1，表示蓝色
        else if (begin_Color3 == "REDGREEN" || begin_Color3 == "GREENRED")
            content[8] |= (1 << 6) | (1 << 5); // 将第6位和第5位同时设置为1，表示红绿双色
        else if (begin_Color3 == "REDBLUE" || begin_Color3 == "BLUERED")
            content[8] |= (1 << 6) | (1 << 4); // 将第6位和第4位同时设置为1，表示红蓝双色
        else if (begin_Color3 == "BLUEGREEN" || begin_Color3 == "GREENBLUE")
            content[8] |= (1 << 5) | (1 << 4); // 将第5位和第4位同时设置为1，表示绿蓝双色
        else if (begin_Color3 == "ALL")
            content[8] |= (1 << 6) | (1 << 5) | (1 << 4); // 将第6位、第5位和第4位同时设置为1，表示全色
    }

    // 设置单元6有效位和颜色
    if (end_Color3 != "NONE")
    {
        content[8] |= (1 << 3); // 将第3位设置为1，表示单元6有效

        if (end_Color3 == "RED")
            content[8] |= (1 << 2); // 将第2位设置为1，表示红色
        else if (end_Color3 == "GREEN")
            content[8] |= (1 << 1); // 将第1位设置为1，表示绿色
        else if (end_Color3 == "BLUE")
            content[8] |= (1 << 0); // 将第0位设置为1，表示蓝色
        else if (end_Color3 == "REDGREEN" || end_Color3 == "GREENBLUE")
            content[8] |= (1 << 2) | (1 << 1); // 将第2位和第1位同时设置为1，表示红绿双色
        else if (end_Color3 == "REDBLUE" || end_Color3 == "BLUERED")
            content[8] |= (1 << 2) | (1 << 0); // 将第2位和第0位同时设置为1，表示红蓝双色
        else if (end_Color3 == "BLUEGREEN" || end_Color3 == "GREENBLUE")
            content[8] |= (1 << 1) | (1 << 0); // 将第1位和第0位同时设置为1，表示绿蓝双色
        else if (end_Color3 == "ALL")
            content[8] |= (1 << 2) | (1 << 1) | (1 << 0); // 将第2位、第1位和第0位同时设置为1，表示全色
    }

// 设置单元7有效位和颜色
if (begin_Color4 != "NONE")
{
    content[9] |= (1 << 7); // 将第7位设置为1，表示单元7有效

    if (begin_Color4 == "RED")
        content[9] |= (1 << 6); // 将第6位设置为1，表示红色
    else if (begin_Color4 == "GREEN")
        content[9] |= (1 << 5); // 将第5位设置为1，表示绿色
    else if (begin_Color4 == "BLUE")
        content[9] |= (1 << 4); // 将第4位设置为1，表示蓝色
    else if (begin_Color4 == "REDGREEN" || begin_Color4 == "GREENRED")
        content[9] |= (1 << 6) | (1 << 5); // 将第6位和第5位同时设置为1，表示红绿双色
    else if (begin_Color4 == "REDBLUE" || begin_Color4 == "BLUERED")
        content[9] |= (1 << 6) | (1 << 4); // 将第6位和第4位同时设置为1，表示红蓝双色
    else if (begin_Color4 == "BLUEGREEN" || begin_Color4 == "GREENBLUE")
        content[9] |= (1 << 5) | (1 << 4); // 将第5位和第4位同时设置为1，表示绿蓝双色
    else if (begin_Color4 == "ALL")
        content[9] |= (1 << 6) | (1 << 5) | (1 << 4); // 将第6位、第5位和第4位同时设置为1，表示全色
}

// 设置单元8有效位和颜色
if (end_Color4 != "NONE")
{
    content[9] |= (1 << 3); // 将第3位设置为1，表示单元8有效

    if (end_Color4 == "RED")
        content[9] |= (1 << 2); // 将第2位设置为1，表示红色
    else if (end_Color4 == "GREEN")
        content[9] |= (1 << 1); // 将第1位设置为1，表示绿色
    else if (end_Color4 == "BLUE")
        content[9] |= (1 << 0); // 将第0位设置为1，表示蓝色
    else if (end_Color4 == "REDGREEN" || end_Color4 == "GREENBLUE")
        content[9] |= (1 << 2) | (1 << 1); // 将第2位和第1位同时设置为1，表示红绿双色
    else if (end_Color4 == "REDBLUE" || end_Color4 == "BLUERED")
        content[9] |= (1 << 2) | (1 << 0); // 将第2位和第0位同时设置为1，表示红蓝双色
    else if (end_Color4 == "BLUEGREEN" || end_Color4 == "GREENBLUE")
        content[9] |= (1 << 1) | (1 << 0); // 将第1位和第0位同时设置为1，表示绿蓝双色
    else if (end_Color4 == "ALL")
        content[9] |= (1 << 2) | (1 << 1) | (1 << 0); // 将第2位、第1位和第0位同时设置为1，表示全色
}

// 设置单元9有效位和颜色
if (begin_Color5 != "NONE")
{
    content[10] |= (1 << 7); // 将第7位设置为1，表示单元9有效

    if (begin_Color5 == "RED")
        content[10] |= (1 << 6); // 将第6位设置为1，表示红色
    else if (begin_Color5 == "GREEN")
        content[10] |= (1 << 5); // 将第5位设置为1，表示绿色
    else if (begin_Color5 == "BLUE")
        content[10] |= (1 << 4); // 将第4位设置为1，表示蓝色
    else if (begin_Color5 == "REDGREEN" || begin_Color5 == "GREENRED")
        content[10] |= (1 << 6) | (1 << 5); // 将第6位和第5位同时设置为1，表示红绿双色
    else if (begin_Color5 == "REDBLUE" || begin_Color5 == "BLUERED")
        content[10] |= (1 << 6) | (1 << 4); // 将第6位和第4位同时设置为1，表示红蓝双色
    else if (begin_Color5 == "BLUEGREEN" || begin_Color5 == "GREENBLUE")
        content[10] |= (1 << 5) | (1 << 4); // 将第5位和第4位同时设置为1，表示绿蓝双色
    else if (begin_Color5 == "ALL")
        content[10] |= (1 << 6) | (1 << 5) | (1 << 4); // 将第6位、第5位和第4位同时设置为1，表示全色
}

// 设置单元10有效位和颜色
if (end_Color5 != "NONE")
{
    content[10] |= (1 << 3); // 将第3位设置为1，表示单元10有效

    if (end_Color5 == "RED")
        content[10] |= (1 << 2); // 将第2位设置为1，表示红色
    else if (end_Color5 == "GREEN")
        content[10] |= (1 << 1); // 将第1位设置为1，表示绿色
    else if (end_Color5 == "BLUE")
        content[10] |= (1 << 0); // 将第0位设置为1，表示蓝色
    else if (end_Color5 == "REDGREEN" || end_Color5 == "GREENBLUE")
        content[10] |= (1 << 2) | (1 << 1); // 将第2位和第1位同时设置为1，表示红绿双色
    else if (end_Color5 == "REDBLUE" || end_Color5 == "BLUERED")
        content[10] |= (1 << 2) | (1 << 0); // 将第2位和第0位同时设置为1，表示红蓝双色
    else if (end_Color5 == "BLUEGREEN" || end_Color5 == "GREENBLUE")
        content[10] |= (1 << 1) | (1 << 0); // 将第1位和第0位同时设置为1，表示绿蓝双色
    else if (end_Color5 == "ALL")
        content[10] |= (1 << 2) | (1 << 1) | (1 << 0); // 将第2位、第1位和第0位同时设置为1，表示全色
}

    // 设置循环次数
    content[11] = LoopNum;

    // 发送命令
    sendCmd(11, content);

    uint8_t r = getReply(content);
    if (r == 1) {
        if (content[0] == FPMXX_CODE_OK) {
            return true;
        } else {
            return false;
        }
    }

    return false;
}


/*
向传感器发送数据（命令）
Params:
len: 通信协议中的包长度值
content: 要发送的数据，从包标识开始。
*/
void FPMXX::sendCmd(uint16_t len, uint8_t *content)
{
	//mySerial->listen();
	uint8_t i;

	//计算校验和
	uint16_t sum = 0;
	for (i = 0; i < len + 1; i++)
		sum += *(content + i);
	
		//等待缓冲区
		mySerial->flush();

		//发送包头
		for (i = 0; i < 2; i++)
			mySerial->write(Pack_Head[i]);

		//发送模块地址
		for (i = 0; i < 4; i++)
			mySerial->write(Adder[i]);

		//发送指令内容
		for (i = 0; i < len + 1; i++)
		{
			mySerial->write(*content);
			content++;
		}
		//发送校验和
		mySerial->write((uint8_t)(sum >> 8));
		mySerial->write((uint8_t)sum);
	
	
}

/*
接收传感器的返回数据并存入参数变量。
仅返回从确认码至校验和之前的数据与长度
param:
pkg: 用于存储返回数据的变量
timeout: 超时返回（毫秒）
return: 返回数据长度，或FPMXX_REPLY_开头错误码；
*/
uint8_t FPMXX::getReply(uint8_t pkg[], uint16_t timeout)
{
	uint16_t timer = 0;
	uint16_t len = 0; //通信协议中包长度的值
	uint8_t reply[40], i = 0;
	uint8_t pkgType; //通信协议中包标识的值
	while (1)
	{	

		while (!mySerial->available())
		{
			delay(1);
			timer++;
			if (timer >= timeout) return FPMXX_REPLY_TIME_OUT;
		}

		reply[i] = mySerial->read();
			//Serial.println(reply[i],HEX);

		//检查数据
		if (i == 1)
		{
			if (reply[0] != Pack_Head[0] ||
				reply[1] != Pack_Head[1])
				//包头不符合
				return FPMXX_REPLY_BAD_PACKAGE;
		}
		else if (i == 5)
		{
			if (reply[2] != Adder[0] ||
				reply[3] != Adder[1] ||
				reply[4] != Adder[2] ||
				reply[5] != Adder[3])
				//地址不符合
				return FPMXX_REPLY_BAD_PACKAGE;
		}
		else if (i == 6) pkgType = reply[6]; //取包标识
		else if (i == 8) //取包长度
		{
			len = reply[7];
			len <<= 8;
			len |= reply[8];
		}
		if (i == 8 + len) break; //因为i从0开始所以不是9而是8
		i++;
	}

	//数据接收完毕
	if (i == 8 + len)
	{
		//校验
		uint16_t sum = 0;
		for (i = 6; i < 6 + len + 1; i++)
			sum += reply[i];

		if (((uint8_t)(sum >> 8)) != reply[7 + len] ||
			((uint8_t)sum) != reply[8 + len])
			//校验不通过
			return FPMXX_REPLY_BAD_PACKAGE;

		len -= 2; //去掉最后的校验和
		for (i = 0; i < len; i++) //从确认码开始返回
			pkg[i] = reply[9 + i];
		return len;
	}
}

uint8_t FPMXX::getReply(uint8_t pkg[])
{
	return getReply(pkg, FPMXX_DEFAULT_REPLY_TIME_OUT);
}