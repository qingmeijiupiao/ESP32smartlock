# ESP32smartlock
ESP32为主控的智能门锁，可选fpm383c指纹模块，实现了连接wifi后弹出开门网页
嘉立创开源地址：https://oshwhub.com/qingmeijiupiao/zhi-wen-suo-jie-xian-ban

菜单只有添加指纹和查看指纹信息可用

#使用方式
1.烧录代码到门锁，并测试舵机旋转角度，在.ino文件中修改开门和关门的角度
2.在web.hpp中选择AP模式或者wifi模式并设置wifi名称和密码，注意wifi模式连接的wifi需要能访问公网
