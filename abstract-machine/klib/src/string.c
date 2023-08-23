#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  // panic("Not implemented");
  size_t ret = 0;
  while(*s){
    ret ++;
    s++;
  }
  return ret;
}

char *strcpy(char *dst, const char *src) {
  // panic("Not implemented");
  char* ret = dst;
  while(*src != '\0' ){
    *dst = *src;
    dst ++;
    src ++;
  }
  *dst = '\0';
  return ret;
}

char *strncpy(char *dst, const char *src, size_t n) {
  // panic("Not implemented");
   char* result = dst; 
    while (n > 0 && *src != '\0') {
        *dst = *src;
        dst++;
        src++;
        n--;
    }
    while (n > 0) {
        *dst = '\0';
        dst++;
        n--;
    }
    return result; // 返回目标字符串的起始地址
}

char *strcat(char *destination, const char *source) {
  // panic("Not implemented");
   char* result = destination; // 保存目标字符串的起始地址
    while (*destination != '\0') {
        destination++;
    }
    while (*source != '\0') {
        *destination = *source;
        destination++;
        source++;
    }
    *destination = '\0'; // 添加 null 终止符
    return result; // 返回目标字符串的起始地址
}

int strcmp(const char *str1, const char *str2) {
  // panic("Not implemented");
  while (*str1 != '\0' && *str2 != '\0') {
        if (*str1 != *str2) {
            return (*str1 - *str2);
        }
        str1++;
        str2++;
    }
    // 如果两个字符串都相等或者都到达了末尾，则返回 0
    return (*str1 - *str2);
}

int strncmp(const char *str1, const char *str2, size_t num) {
  // panic("Not implemented");
  while (num > 0 && *str1 != '\0' && *str2 != '\0') {
        if (*str1 != *str2) {
            return (*str1 - *str2);
        }
        str1++;
        str2++;
        num--;
    }
    if (num == 0) {
        return 0; // 达到指定的字符数
    }
    return (*str1 - *str2); // 两个字符串都相等或者都到达了末尾，返回 0
}

void *memset(void *ptr, int value, size_t num) {
  // panic("Not implemented");
  unsigned char* byte_ptr = (unsigned char*)ptr;
    for (size_t i = 0; i < num; i++) {
        *byte_ptr = (unsigned char)value;
        byte_ptr++;
    }
    return ptr;
}

void *memmove(void *destination, const void *source, size_t num) {
  // panic("Not implemented");
   unsigned char* dest_byte = (unsigned char*)destination;
    const unsigned char* src_byte = (const unsigned char*)source;
    // 检查是否存在重叠
    if (destination > source && destination < source + num) {
        // 从尾部开始逐个字节复制
        dest_byte += num;
        src_byte += num;
        while (num > 0) {
            *(--dest_byte) = *(--src_byte);
            num--;
        }
    } else {
        // 正常情况下从头部开始逐个字节复制
        for (size_t i = 0; i < num; i++) {
            *(dest_byte++) = *(src_byte++);
        }
    }
    return destination;
}

void *memcpy(void *destination, const void *source, size_t num) {
  // panic("Not implemented");
  unsigned char* dest_byte = (unsigned char*)destination;
    const unsigned char* src_byte = (const unsigned char*)source;
    // 逐个字节复制
    for (size_t i = 0; i < num; i++) {
        *(dest_byte++) = *(src_byte++);
    }
    return destination;
}

int memcmp(const void *ptr1, const void *ptr2, size_t num) {
  // panic("Not implemented");
  const unsigned char* byte_ptr1 = (const unsigned char*)ptr1;
    const unsigned char* byte_ptr2 = (const unsigned char*)ptr2;
    // 逐个字节比较
    for (size_t i = 0; i < num; i++) {
        if (*byte_ptr1 != *byte_ptr2) {
            return (*byte_ptr1 - *byte_ptr2);
        }
        byte_ptr1++;
        byte_ptr2++;
    }
    return 0; // 两个内存块相等
}

#endif
