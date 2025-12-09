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

inline uint8_t keyHolded()
{
    uint8_t rv;
    asm volatile (
        "mov $0177716,r0\n"
        "mov (r0),%0\n"
        : "=r" (rv): : "r0","cc"
    );

    if ((rv & 0100)!=0) return 0 ;

    asm volatile (
        "mov $0177662,r0\n"
        "mov (r0),%0\n"
        : "=r" (rv): : "r0","cc"
    );
    return rv;
}

// Тип цвета и значения цветов для псевдографики
enum BkColor { Black=0, Red=1, Green=2, Blue=3 } ;
const uint16_t colors[4] = { 0, 0177777, 0125252, 052525 } ;

// Установка цвета переднего плана
void setColor(enum BkColor bkcolor) {
  const uint16_t CELL_COLOR = 0214 ;
  *((uint16_t *)CELL_COLOR)=colors[bkcolor];
}

// Функции логики
uint8_t enemyx ;
uint8_t enemyy ;
uint8_t playerx ;
uint8_t playery ;

const uint8_t SPACE = 1 ;
const uint8_t SIZEX = 30 ;
const uint8_t SIZEY = 20 ;

void newEnemy() {
   enemyx = SPACE ;
   enemyy = SPACE ;
}

void movePlayer(int8_t dx, int8_t dy) {
    setColor(Green) ;
    EMT_24(playerx,playery) ;
    EMT_16(040) ;
    playerx+=dx ;
    playery+=dy ;
    EMT_24(playerx,playery) ;
    EMT_16(043) ;
}

void moveEnemy(int8_t dx, int8_t dy) {
    setColor(Red) ;
    EMT_24(enemyx,enemyy) ;
    EMT_16(040) ;
    enemyx+=dx ;
    enemyy+=dy ;
    EMT_24(enemyx,enemyy) ;
    EMT_16(044) ;
}

uint8_t iabs(int8_t v) {
  if (v<0) return -v ; else return v ;
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
    for (int i=1; i<=30; i++) EMT_16(0265) ;
    EMT_16(0243) ;

    setColor(Green) ;
    for (int i=1; i<=20; i++) {
        EMT_24(0,i) ;
        EMT_16(0267) ;
        EMT_24(31,i) ;
        EMT_16(0267) ;
    }

    setColor(Blue) ;
    EMT_16(0246) ;
    for (int i=1; i<31; i++) EMT_16(0265) ;
    EMT_16(0271) ;
    
    newEnemy() ;
    playerx = 15 ;
    playery = 10 ;

    setColor(Green) ;
    EMT_24(playerx,playery) ;
    EMT_16(043) ;
    setColor(Red) ;
    EMT_24(enemyx,enemyy) ;
    EMT_16(044) ;

    uint16_t ticks = 0 ;

    for (;;) {
       // Проверка нажатия
       uint8_t key = keyHolded() ;
       if (ticks==0) { // Ограничения по тактам
         if (key==010) // Влево
           if (playerx>SPACE) movePlayer(-1,0) ;
         if (key==031) // Вправо
           if (playerx<=SIZEX-SPACE) movePlayer(1,0) ;
         if (key==032) // Вверх
           if (playery>SPACE) movePlayer(0,-1) ;
         if (key==033) // Вниз
           if (playery<=SIZEY-SPACE) movePlayer(0,1) ;

         int8_t dx=0 ;
         int8_t dy=0 ;
         if (playerx<enemyx) dx=-1 ;
         if (playerx>enemyx) dx=1 ;
         if (playery<enemyy) dy=-1 ;
         if (playery>enemyy) dy=1 ;
         moveEnemy(dx,dy) ;
         if ((iabs(playerx-enemyx)<2)&&(iabs(playery-enemyy)<2)) {
           EMT_24(enemyx,enemyy) ;
           EMT_16(040) ;
           newEnemy() ;
           EMT_24(enemyx,enemyy) ;
           EMT_16(044) ;
         }
       }
       if (key==3) break ; // Выход по КТ
       ticks++ ;
       if (ticks>2000) ticks=0 ;
    }
    EMT_14() ;
}
