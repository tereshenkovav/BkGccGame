#include "stdint.h"
#include "bk0010_rtl.h"

// Очистка экрана
void ClearScreen()
{
    asm volatile (
	"mov $040000,r0\n\t"
	".l1_%=:\n\t"
	"clr	(r0)+\n\t"
	"cmp	r0,$0100000\n\t"
        "bne	.l1_%=\n"
        : : : "r0","cc"
    );
}

// Установка цвета переднего плана
void setColor(enum BkColor bkcolor) {
  const uint16_t colors[4] = { 0, 0177777, 0125252, 052525 } ;
  writeWord(0214,colors[bkcolor]) ;
}

// Играть звук длиной len тактов и с периодами внутри такта period
// Чем больше period, тем ниже звук
void playSound(uint16_t len, uint16_t period) {
  for (uint16_t i=0; i<len; i++) {
    writeWord(0177716, 0100) ;
    for (uint16_t j=0; j<period; j++)
       asm volatile ("nop\n") ;
    writeWord(0177716, 0) ;
    for (uint16_t j=0; j<period; j++)
       asm volatile ("nop\n") ;
  }
}

// Вывод символа в позиции
void drawCharAt(uint8_t x, uint8_t y, char c) {
  EMT_24(x,y) ;
  EMT_16(c) ;
}

// Текущее значение генератора
uint16_t seed = 0 ;

inline uint16_t XOR(uint16_t v1, uint16_t v2) {
    asm volatile (
        "mov %2, r0\n\t"
        "xor %1, r0\n\t"
        "mov r0,%0\n"
        : "=r" (v1) : "r" (v1), "r" (v2): "r0","cc"
    );
    return v1 ;
}

// Установка нового значения генератора
void Randomize(uint16_t v) {
  seed = v ;
}

// Получение следующего seed
// XOR используем через ассемблер, можно сделать функцию ^ для линкера
uint16_t getNewSeed() {
  seed = XOR(seed, seed << 7);
  seed = XOR(seed, seed >> 9);
  seed = XOR(seed, seed << 8);
  return seed ;
}

// Черновая процедура генерации числа в байте от 0 до d-1
uint8_t genRndByByte(uint8_t d) {
  uint8_t v = getNewSeed() ;
  while (v>=d) v-=d ;
  return v ;
}

// Узкоспециализированная процедура, которая возвращает случайное
// от 0 до 2 в степени n-1 путем обрезки байта
// Работает быстрее, чем основная, для малых n
// Можно улучшить константами
uint8_t genRndByByteN2(uint8_t n) {
  uint8_t v = 1 ;
  for (uint16_t i=1; i<n; i++)
    v+=(1<<i) ;
  return getNewSeed() & v ;
}

// Вывод цифры в позиции
void drawDigitAt(uint8_t x, uint8_t y, uint8_t d) {
  EMT_24(x,y) ;
  EMT_16('0'+d) ;
}

// Деление методом вычитания, используется только внутри drawUIntAt для перевода
// Алгоритм медленный при условии, что v намного больше r
void DivMod10(uint16_t v, uint16_t r, uint16_t * d, uint16_t * m) {
  *d = 0 ;
  while (v>=r) {
    (*d)++ ;
    v-=r ;
  }
  *m = v ;
}

// Вывод десятичного числа с ведущими пробелами
void drawUIntAt(uint8_t x, uint8_t y, uint16_t v) {
  const uint16_t D10[4] = { 10000, 1000, 100, 10 } ;
  static char str[6] ;
  str[5]=0 ;
  static uint16_t d ;
  static uint16_t m ;
  for (uint8_t i = 0; i<4; i++) {
    DivMod10(v,D10[i],&d,&m) ;
    str[i]='0'+d ;
    v=m ;
  }
  str[4]='0'+v ;

  EMT_24(x,y) ;
  EMT_20(str) ;
}

// Вывод нуль-терминальной строки в позиции
void drawStringAt(uint8_t x, uint8_t y, const char * str) {
  EMT_24(x,y) ;
  EMT_20(str) ;
}

// Модуль для 8 битного числа
uint8_t iabs(int8_t v) {
  if (v<0) return -v ; else return v ;
}
