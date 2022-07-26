main.h 里面不要#include "oled.h"不然就会报错  也不知道为什么这么奇怪 但是不包括还能够使用

如果想要修改管脚  需要去oled.c里面的一堆crl等寄存器里面修改 修改完不要忘记修改 oled.h里面的宏 OLED_CS 的端口号