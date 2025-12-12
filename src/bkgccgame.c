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

inline uint16_t writeWord(uint16_t addr, uint16_t value)
{
    uint16_t res;
    asm volatile (
        "mov %1, r0\n\t"
        "mov %0, (r0)\n"
        : : "r" (value), "r" (addr): "r0","cc"
    );
    return res ;
}

inline uint8_t keyHolded()
{
    if ((readWord(0177716) & 0100)!=0) return 0 ;
    return readWord(0177662) ;
}

inline void startTimer(uint16_t ticks)
{
    writeWord(0177706,ticks) ; // Длительность фрейма// (3777 - примерно 10 FPS)
    writeWord(0177712,024) ; // Разрешаем счет и индикацию
}

inline void waitFrameEnd()
{
    while ((readWord(0177712) & 0200)==0) ; // Ждем таймера
}

// Тип цвета и значения цветов для псевдографики
enum BkColor { Black=0, Red=1, Green=2, Blue=3 } ;

// Установка цвета переднего плана
void setColor(enum BkColor bkcolor) {
  const uint16_t colors[4] = { 0, 0177777, 0125252, 052525 } ;
  writeWord(0214,colors[bkcolor]) ;
}

void drawCharAt(uint8_t x, uint8_t y, char c) {
  EMT_24(x,y) ;
  EMT_16(c) ;
}

// Функции логики
uint8_t enemyx ;
uint8_t enemyy ;
uint8_t playerx ;
uint8_t playery ;
uint8_t bonusx ;
uint8_t bonusy ;
uint16_t T_enemy ;
uint16_t T_player ;

const char HERO = 0100 ;
const char ENEMY = 044 ;
const char BONUS = 053 ;
const char SPACE = 040 ;

const uint8_t BORDER = 1 ;
const uint8_t SIZEX = 30 ;
const uint8_t SIZEY = 20 ;
const uint8_t SPAWNX0 = 1 ;
const uint8_t SPAWNY0 = 1 ;

const uint8_t SPEEDUP_POS_X = 10 ;
const uint8_t GAMETIME_POS_X = 26 ;

void newEnemy() {
   enemyx = SPAWNX0 ;
   enemyy = SPAWNY0 ;
}

void movePlayer(int8_t dx, int8_t dy) {
    setColor(Green) ;
    drawCharAt(playerx,playery,SPACE) ;
    playerx+=dx ;
    playery+=dy ;
    drawCharAt(playerx,playery,HERO) ;
}

void moveEnemy(int8_t dx, int8_t dy) {
    setColor(Red) ;
    drawCharAt(enemyx,enemyy,SPACE) ;
    enemyx+=dx ;
    enemyy+=dy ;
    drawCharAt(enemyx,enemyy,ENEMY) ;
}

void drawDigitAt(uint8_t x, uint8_t y, uint8_t d) {
  EMT_24(x,y) ;
  EMT_16(060+d) ;
}

void DivMod10(uint16_t v, uint16_t r, uint16_t * d, uint16_t * m) {
  *d = 0 ;
  while (v>=r) {
    (*d)++ ;
    v-=r ;
  }
  *m = v ;
}


void drawUIntAt(uint8_t x, uint8_t y, uint16_t v) {
  const uint16_t D10[4] = { 10000, 1000, 100, 10 } ;
  static char str[6] ;
  str[5]=0 ;
  static uint16_t d ;
  static uint16_t m ;
  for (uint8_t i = 0; i<4; i++) {
    DivMod10(v,D10[i],&d,&m) ;
    str[i]=060+d ;
    v=m ;
  }
  str[4]=060+v ;

  EMT_24(x,y) ;
  EMT_20(str) ;
}

void drawStringAt(uint8_t x, uint8_t y, const char * str) {
  EMT_24(x,y) ;
  EMT_20(str) ;
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
    setColor(Green) ;
    EMT_16(0252) ;
    for (int i=BORDER; i<=SIZEX; i++) EMT_16(0265) ;
    EMT_16(0243) ;

    for (int i=BORDER; i<=SIZEY; i++) {
        drawCharAt(0,i,0267) ;
        drawCharAt(31,i,0267) ;
    }

    EMT_16(0246) ;
    for (int i=BORDER; i<=SIZEX; i++) EMT_16(0265) ;
    EMT_16(0271) ;

    // Вывод надписей
    drawStringAt(1,SIZEY+2,"SPEEDUP:") ;
    drawStringAt(20,SIZEY+2,"TIME:") ;

    newEnemy() ;
    playerx = 15 ;
    playery = 10 ;
    bonusx = 25 ;
    bonusy = 17 ;

    uint16_t gametime = 0 ;

    setColor(Green) ;
    drawCharAt(playerx,playery,HERO) ;
    setColor(Red) ;
    drawCharAt(enemyx,enemyy,ENEMY) ;
    setColor(Blue) ;
    drawCharAt(bonusx,bonusy,BONUS) ;

    uint16_t ticks_common = 0 ;
    uint16_t ticks_enemy = 0 ;
    uint16_t ticks_player = 0 ;
    uint8_t left_bonus = 0 ;

    T_enemy = 10 ;
    T_player = 10 ;

    setColor(Green) ;
    drawUIntAt(GAMETIME_POS_X,SIZEY+2,gametime) ;

    for (;;) {
       startTimer(03777) ; // 03777 - примерно 0.1 с

       // Проверка нажатия
       uint8_t key = keyHolded() ;
       if (ticks_player==0) { // Ограничения по тактам
         if (key==010) // Влево
           if (playerx>BORDER) movePlayer(-1,0) ;
         if (key==031) // Вправо
           if (playerx<=SIZEX-BORDER) movePlayer(1,0) ;
         if (key==032) // Вверх
           if (playery>BORDER) movePlayer(0,-1) ;
         if (key==033) // Вниз
           if (playery<=SIZEY-BORDER) movePlayer(0,1) ;
         if ((playerx==bonusx)&&(playery==bonusy)) {
           // Перекидывание бонуса на другой конец доски вместо рандома
           bonusx = SIZEX-bonusx ;
           bonusy = SIZEY-bonusy ;
           setColor(Blue) ;
           drawCharAt(bonusx,bonusy,BONUS) ;
           T_player = 5 ;
           left_bonus = 9 ;
           setColor(Green) ;
           drawDigitAt(SPEEDUP_POS_X,SIZEY+2,left_bonus) ;
         }
       }
       if (ticks_enemy==0) { // Ограничения по тактам
         int8_t dx=0 ;
         int8_t dy=0 ;
         if (playerx<enemyx) dx=-1 ;
         if (playerx>enemyx) dx=1 ;
         if (playery<enemyy) dy=-1 ;
         if (playery>enemyy) dy=1 ;

         moveEnemy(dx,dy) ;
         if ((iabs(playerx-enemyx)<2)&&(iabs(playery-enemyy)<2)) {
           setColor(Red) ;
           drawStringAt(1,SIZEY+3,"GAMEOVER, PRESS ENTER") ;
           break ;
         }
       }
       if (ticks_common==0) { // Ежесекундная процедура
         if (left_bonus>0) {
           left_bonus-- ;
           if (left_bonus==0) {
             T_player = 10 ;
             drawCharAt(SPEEDUP_POS_X,SIZEY+2,040) ;
           }
           else {
             setColor(Green) ;
             drawDigitAt(SPEEDUP_POS_X,SIZEY+2,left_bonus) ;
           }
         }
         gametime++ ;
         setColor(Green) ;
         drawUIntAt(GAMETIME_POS_X,SIZEY+2,gametime) ;
       }
       if (key==3) break ; // Выход по КТ
       ticks_common++ ;
       ticks_player++ ;
       ticks_enemy++ ;
       if (ticks_common>10) ticks_common=0 ;
       if (ticks_player>T_player) ticks_player=0 ;
       if (ticks_enemy>T_enemy) ticks_enemy=0 ;

       waitFrameEnd() ;
    }
    while (keyHolded()!=012) ;
    EMT_14() ;
}
