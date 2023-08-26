#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
	char buffer[2048];
	va_list arg;
	va_start(arg, fmt);
	int ret = vsprintf(buffer, fmt, arg);
	putstr(buffer);
	va_end(arg);
	return ret;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  int len=0;
  while(*fmt!='\0'){
	  /* no % */
	  if(*fmt!='%'){
		  *out=*fmt;
		  out++;fmt++;len++;continue;
	  }
	  /* if % fmt++ & get type */
	  fmt++;
	  switch(*fmt){
		  case 's':{
			  char* sp=va_arg(ap,char*);
			  while(*sp!='\0'){*out=*sp;out++;sp++;len++;}
			  break;}
		  case 'd':{
			  int num=va_arg(ap,int);
			  if(num==0){*out='0';out++;len++;break;}
			  if(num<0){*out='-';out++;len++;num=-num;}
			  assert(num>0);
			  char numb[12];int i=0;
			  while(num!=0){numb[i]='0'+num%10;i++;num/=10;}
			  while(i>0){i--;*out=numb[i];out++;len++;}
			  break;}
	  }
	  fmt++;
  }
  *out='\0';len++;
  return len;
}
int sprintf(char *out, const char *fmt, ...) {
  va_list ap;va_start(ap,fmt);
  unsigned int i;
  i=vsprintf(out,fmt,ap);
  va_end(ap);
  return i;
}







// int printf(const char *fmt, ...) {
//   panic("Not implemented");
// }
// int vsprintf(char *out, const char *fmt, va_list ap) {
//   panic("Not implemented");
// }

// int snprintf(char *out, size_t n, const char *fmt, ...) {
//   panic("Not implemented");
// }

// int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
//   panic("Not implemented");
// }

// int sprintf(char *str, const char *format, ...) {
//   // panic("Not implemented");
//     va_list args;
//     int len;
//     va_start(args, format);
//     len = vsprintf(str, format, args);
//     va_end(args);
//     return len;
// }
// static void reverse(char *s, int len) {
//   char *end = s + len - 1;
//   char tmp;
//   while (s < end) {
//     tmp = *s;
//     *s = *end;
//     *end = tmp;
//   }
// }

// /* itoa convert int to string under base. return string length */
// static int itoa(int n, char *s, int base) {
//   assert(base <= 16);

//   int i = 0, sign = n, bit;
//   if (sign < 0) n = -n;
//   do {
//     bit = n % base;
//     if (bit >= 10) s[i++] = 'a' + bit - 10;
//     else s[i++] = '0' + bit;
//   } while ((n /= base) > 0);
//   if (sign < 0) s[i++] = '-';
//   s[i] = '\0';
//   reverse(s, i);

//   return i;
// }

// int sprintf(char *out, const char *fmt, ...) {
//   va_list pArgs;
//   va_start(pArgs, fmt);
//   char *start = out;
  
//   for (; *fmt != '\0'; ++fmt) {
//     if (*fmt != '%') {
//       *out = *fmt;
//       ++out;
//     } else {
//       switch (*(++fmt)) {
//       case '%': *out = *fmt; ++out; break;
//       case 'd': out += itoa(va_arg(pArgs, int), out, 10); break;
//       case 's':
//         char *s = va_arg(pArgs, char*);
//         strcpy(out, s);
//         out += strlen(out);
//         break;
//       }
//     }
//   }
//   *out = '\0';
//   va_end(pArgs);

//   return out - start;
// }



#endif

