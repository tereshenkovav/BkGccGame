#ifndef BK0010_RTL_H
#define BK0010_RTL_H

#include "stdint.h"
// Тип цвета и значения цветов для псевдографики
enum BkColor { Black=0, Red=1, Green=2, Blue=3 } ;

// Константы для клавиш
static const char KEY_ENTER = 012 ;
static const char KEY_LEFT = 010 ;
static const char KEY_RIGHT = 031 ;
static const char KEY_UP = 032 ;
static const char KEY_DOWN = 033 ;
static const char KEY_KT = 03 ;

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

// Вывод строки по умолчанию с нулем на конце
inline void EMT_20(const char * str)
{
    asm volatile (
        "mov %0, r1\n\t"
        "mov $0, r2\n\t"
        "emt 020\n"
        : : "r" (str): "r1","r2","cc"
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

// Чтение слова по адресу
inline uint16_t readWord(uint16_t addr)
{
    uint16_t res;
    asm volatile (
        "mov %1, r0\n\t"
        "mov (r0),%0\n"
        : "=r" (res) : "r" (addr): "r0","cc"
    );
    return res ;
}

// Запись слова по адресу
inline void writeWord(uint16_t addr, uint16_t value)
{
    asm volatile (
        "mov %1, r0\n\t"
        "mov %0, (r0)\n"
        : : "r" (value), "r" (addr): "r0","cc"
    );
}

// Проверка удержания клавиши, 0 - если клавиши отпущены
inline uint8_t keyHolded()
{
    if ((readWord(0177716) & 0100)!=0) return 0 ;
    return readWord(0177662) ;
}

// Запуск таймера с указанной длиной отсчета
inline void startTimer(uint16_t ticks)
{
    writeWord(0177706,ticks) ; // Длительность фрейма
    writeWord(0177712,024) ; // Разрешаем счет и индикацию
}

// Ожидание прохода таймера через ноль
inline void waitFrameEnd()
{
    while ((readWord(0177712) & 0200)==0) ; // Ждем таймера
}

// Очистка экрана
void ClearScreen();
// Установка цвета переднего плана
void setColor(enum BkColor bkcolor);
// Играть звук длиной len тактов и с периодами внутри такта period
// Чем больше period, тем ниже звук
void playSound(uint16_t len, uint16_t period);
// Вывод символа в позиции
void drawCharAt(uint8_t x, uint8_t y, char c);
// Установка нового значения генератора
void Randomize(uint16_t v);
// Черновая процедура генерации числа в байте от 0 до d-1
uint8_t genRndByByte(uint8_t d);
// Узкоспециализированная процедура, которая возвращает случайное
// от 0 до 2 в степени n-1 путем обрезки байта
// Работает быстрее, чем основная, для малых n
// Можно улучшить константами
uint8_t genRndByByteN2(uint8_t n);
// Вывод цифры в позиции
void drawDigitAt(uint8_t x, uint8_t y, uint8_t d);
// Вывод десятичного числа с ведущими пробелами
void drawUIntAt(uint8_t x, uint8_t y, uint16_t v);
// Вывод нуль-терминальной строки в позиции
void drawStringAt(uint8_t x, uint8_t y, const char * str);
// Модуль для 8 битного числа
uint8_t iabs(int8_t v);

#endif
