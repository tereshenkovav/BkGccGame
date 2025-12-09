#include "stdint.h"

// Ожидание клавиши
inline char EMT_6()
{
    char rv;
    asm volatile (
        "emt 06\n"
        "mov r0, %0"
        : "=r" (rv)
    );

    return rv;
}

// Установка вектора прерываний
inline void EMT_14()
{
    asm volatile (
        "emt 014\n"
        : : : "cc"
    );
}


// Вывод символа
inline void EMT_16(char c)
{
    asm volatile (
        "mov %0, r0\n\t"
        "emt 016\n"
        : : "r" (c) : "r0", "cc"
    );
}

// Установка позиции курсора
inline void EMT_24(uint8_t x, uint8_t y)
{
    asm volatile (
        "mov %0, r1\n\t"
        "mov %1, r2\n\t"
        "emt 024\n"
        : : "r" (x), "r" (y) : "r1","r2","cc"
    );
}

// Запрет на прерывание клавиатуры
inline void DenyKeyboardInterrupt()
{
    asm (
        "mov $0177660, r0\n\t"
        "bis $0100, (r0)\n"
        : : : "r0","cc"
    );
}

// Тип цвета и значения цветов для псевдографики
enum BkColor { Black=0, Red=1, Green=2, Blue=3 } ;
const uint16_t colors[4] = { 0, 0177777, 0125252, 052525 } ;

// Установка цвета переднего плана
void setColor(enum BkColor bkcolor) {
  const uint16_t CELL_COLOR = 0214 ;
  *((uint16_t *)CELL_COLOR)=colors[bkcolor];
}

void main()
{
    EMT_14() ;
    // Режим 32 символа
    EMT_16(0233) ;
    // Скрытие курсора
    EMT_16(0232) ;

    //Запрещаем прерывания от клавиатуры, чтобы не мешало игре
    DenyKeyboardInterrupt() ;

    // Рисование рамки
    setColor(Red) ;
    EMT_16(0252) ;
    for (int i=1; i<31; i++) EMT_16(0265) ;
    EMT_16(0243) ;

    setColor(Green) ;
    for (int i=1; i<20; i++) {
        EMT_24(0,i) ;
        EMT_16(0267) ;
        EMT_24(31,i) ;
        EMT_16(0267) ;
    }

    setColor(Blue) ;
    EMT_16(0246) ;
    for (int i=1; i<31; i++) EMT_16(0265) ;
    EMT_16(0271) ;

    // Ожидание нажатия
    char c = EMT_6() ;
    EMT_14() ;
}
