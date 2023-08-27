#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  // kbd->keydown = 0;
  // kbd->keycode = AM_KEY_NONE;
  uint32_t tmp = inl(KBD_ADDR); // get info 
  if(tmp != AM_KEY_NONE){
    // tong guo & caozuo lai huo de gai keydown de zhi.
    kbd -> keydown = KEYDOWN_MASK  & tmp; 
    kbd -> keycode = tmp & (~KEYDOWN_MASK);
  }
  else {
    kbd -> keydown = false;
    kbd -> keycode = AM_KEY_NONE;
  }


}
/*
  AM_INPUT_KEYBRD, AM键盘控制器, 可读出按键信息. 
  keydown为true时表示按下按键, 否则表示释放按键. keycode为按键的断码, 没有按键时, keycode为AM_KEY_NONE
  i8042芯片初始化时会分别注册0x60处长度为4个字节的端口, 以及0xa0000060处长度为4字节的MMIO空间,
*/
