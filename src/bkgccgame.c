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

inline void writeWord(uint16_t addr, uint16_t value)
{
    asm volatile (
        "mov %1, r0\n\t"
        "mov %0, (r0)\n"
        : : "r" (value), "r" (addr): "r0","cc"
    );
}

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

uint16_t seed ;

inline uint16_t XOR(uint16_t v1, uint16_t v2) {
    asm volatile (
        "mov %2, r0\n\t"
        "xor %1, r0\n\t"
        "mov r0,%0\n"
        : "=r" (v1) : "r" (v1), "r" (v2): "r0","cc"
    );
    return v1 ;
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

// Константы для клавиш
const char KEY_ENTER = 012 ;
const char KEY_LEFT = 010 ;
const char KEY_RIGHT = 031 ;
const char KEY_UP = 032 ;
const char KEY_DOWN = 033 ;
const char KEY_KT = 03 ;

// Функции логики
enum BonusType { btNone=0, btSpeedUp=1, btScore=2, btShield=3 } ;

struct Bonus {
  uint8_t x ;
  uint8_t y ;
  enum BonusType t ;
};

const char BONUS_CHARS[4] = { 0, 053, 052, 045 } ;

const uint16_t MAXBONUS = 16 ;
struct Bonus bonuses[16] ;
uint8_t enemyx ;
uint8_t enemyy ;
uint8_t playerx ;
uint8_t playery ;
uint16_t T_enemy ;
uint16_t T_player ;

const char HERO = 0100 ;
const char ENEMY = 044 ;
const char SPACE = 040 ;

const uint8_t BORDER = 1 ;
const uint8_t SIZEX = 30 ;
const uint8_t SIZEY = 20 ;
const uint8_t SPAWNX[4] = {BORDER,SIZEX,BORDER,SIZEX} ;
const uint8_t SPAWNY[4] = {BORDER,BORDER,SIZEY,SIZEY} ;

const uint8_t SPEEDUP_POS_X = 9 ;
const uint8_t SHIELD_POS_X = 18 ;
const uint8_t SCORE_POS_X = 26 ;

const uint16_t INC_SCORE_BY_BONUS = 100 ;
const uint16_t INC_SCORE_BY_KILLENEMY = 50 ;
const uint16_t INC_SCORE_BY_ONESEC = 10 ;

const uint16_t LIFETIME_FOR_BONUS = 9 ;
const uint16_t MOVE_PERIOD_ENEMY = 10 ;
const uint16_t MOVE_PERIOD_PLAYER = 10 ;
const uint16_t MOVE_PERIOD_PLAYER_FAST = 5 ;

void newEnemy() {
   uint8_t idx = genRndByByteN2(2) ;
   enemyx = SPAWNX[idx] ;
   enemyy = SPAWNY[idx] ;
}

// Возврат MAXBONUS - означает, что нет индекса
uint16_t getBonusIdxAt(uint8_t x, uint8_t y) {
   for (uint16_t i=0; i<MAXBONUS; i++)
     if (bonuses[i].t!=btNone)
       if ((x==bonuses[i].x)&&(y==bonuses[i].y))
         return i ;
   return MAXBONUS ;
}

uint16_t newBonus(enum BonusType t) {
   for (uint16_t i=0; i<MAXBONUS; i++)
     if (bonuses[i].t==btNone) {
       while (1) {
         uint8_t x = BORDER+genRndByByte(SIZEX) ;
         uint8_t y = BORDER+genRndByByte(SIZEY) ;
         if ((getBonusIdxAt(x,y)==MAXBONUS)&&((playerx!=x)||(playery!=y))) {
           bonuses[i].x=x ;
           bonuses[i].y=y ;
           bonuses[i].t=t ;
           break ;
         }
       }
       return i;
     }
}

void movePlayer(int8_t dx, int8_t dy) {
    drawCharAt(playerx,playery,SPACE) ;
    playerx+=dx ;
    playery+=dy ;
    setColor(Green) ;
    drawCharAt(playerx,playery,HERO) ;
}

void moveEnemy(int8_t dx, int8_t dy) {
    drawCharAt(enemyx,enemyy,SPACE) ;
    uint16_t idx = getBonusIdxAt(enemyx,enemyy) ;
    if (idx!=MAXBONUS) {
      setColor(Blue) ;
      drawCharAt(bonuses[idx].x,bonuses[idx].y,BONUS_CHARS[bonuses[idx].t]) ;
    }
    enemyx+=dx ;
    enemyy+=dy ;
    setColor(Red) ;
    drawCharAt(enemyx,enemyy,ENEMY) ;
}

void drawDigitAt(uint8_t x, uint8_t y, uint8_t d) {
  EMT_24(x,y) ;
  EMT_16('0'+d) ;
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
    str[i]='0'+d ;
    v=m ;
  }
  str[4]='0'+v ;

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

    drawStringAt(0,0,"PRESS ENTER TO START") ;
    seed=0 ;
    while (keyHolded()!=KEY_ENTER) seed++ ;

Game:
    ClearScreen() ;

    // Рисование рамки
    setColor(Green) ;
    drawCharAt(0,0,0252) ;
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
    drawStringAt(11,SIZEY+2,"SHIELD:") ;
    drawStringAt(20,SIZEY+2,"SCORE:") ;

    newEnemy() ;
    playerx = 15 ;
    playery = 10 ;

    for (uint16_t i=0; i<MAXBONUS; i++)
      bonuses[i].t = btNone ;
    newBonus(btSpeedUp) ;
    newBonus(btSpeedUp) ;
    newBonus(btSpeedUp) ;
    newBonus(btScore) ;
    newBonus(btScore) ;
    newBonus(btScore) ;
    newBonus(btShield) ;
    newBonus(btShield) ;
    newBonus(btShield) ;

    uint16_t score = 0 ;

    setColor(Green) ;
    drawCharAt(playerx,playery,HERO) ;
    setColor(Red) ;
    drawCharAt(enemyx,enemyy,ENEMY) ;
    setColor(Blue) ;
    for (uint16_t i=0; i<MAXBONUS; i++)
      if (bonuses[i].y!=btNone)
        drawCharAt(bonuses[i].x,bonuses[i].y,BONUS_CHARS[bonuses[i].t]) ;

    uint16_t ticks_common = 0 ;
    uint16_t ticks_enemy = 0 ;
    uint16_t ticks_player = 0 ;
    uint8_t left_bonus_speed = 0 ;
    uint8_t left_bonus_shield = 0 ;

    T_enemy = MOVE_PERIOD_ENEMY ;
    T_player = MOVE_PERIOD_PLAYER ;

    setColor(Green) ;
    drawUIntAt(SCORE_POS_X,SIZEY+2,score) ;

    for (;;) {
       startTimer(03777) ; // 03777 - примерно 0.1 с

       // Проверка нажатия
       uint8_t key = keyHolded() ;
       if (ticks_player==0) { // Ограничения по тактам
         if (key==KEY_LEFT)
           if (playerx>BORDER) movePlayer(-1,0) ;
         if (key==KEY_RIGHT)
           if (playerx<=SIZEX-BORDER) movePlayer(1,0) ;
         if (key==KEY_UP)
           if (playery>BORDER) movePlayer(0,-1) ;
         if (key==KEY_DOWN)
           if (playery<=SIZEY-BORDER) movePlayer(0,1) ;

         uint16_t idx = getBonusIdxAt(playerx,playery) ;
         if (idx!=MAXBONUS) {
               enum BonusType bt = bonuses[idx].t ;
               if (bt==btSpeedUp) {
                 T_player = MOVE_PERIOD_PLAYER_FAST ;
                 left_bonus_speed = LIFETIME_FOR_BONUS ;
                 setColor(Green) ;
                 drawDigitAt(SPEEDUP_POS_X,SIZEY+2,left_bonus_speed) ;
               }
               if (bt==btShield) {
                 left_bonus_shield = LIFETIME_FOR_BONUS ;
                 setColor(Green) ;
                 drawDigitAt(SHIELD_POS_X,SIZEY+2,left_bonus_shield) ;
               }
               if (bt==btScore)
                 score+=INC_SCORE_BY_BONUS ;
               bonuses[idx].t=btNone ;
               idx = newBonus(bt) ;
               setColor(Blue) ;
               drawCharAt(bonuses[idx].x,bonuses[idx].y,BONUS_CHARS[bonuses[idx].t]) ;
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
           if (left_bonus_shield==0) {
             setColor(Red) ;
             drawStringAt(1,SIZEY+3,"GAMEOVER, PRESS ENTER") ;
             break ;
           }
           else {
             score+=INC_SCORE_BY_KILLENEMY ;
             // Перерисовка игрока или затирание монстра, по ситуации
             if ((enemyx==playerx)&&(enemyy==playery)) {
               setColor(Green) ;
               drawCharAt(playerx,playery,HERO) ;
             }
             else
               drawCharAt(enemyx,enemyy,SPACE) ;
             newEnemy() ;
             setColor(Red) ;
             drawCharAt(enemyx,enemyy,ENEMY) ;
           }
         }
       }
       if (ticks_common==0) { // Ежесекундная процедура
         if (left_bonus_speed>0) {
           left_bonus_speed-- ;
           if (left_bonus_speed==0) {
             T_player = MOVE_PERIOD_PLAYER ;
             drawCharAt(SPEEDUP_POS_X,SIZEY+2,SPACE) ;
           }
           else {
             setColor(Green) ;
             drawDigitAt(SPEEDUP_POS_X,SIZEY+2,left_bonus_speed) ;
           }
         }
         if (left_bonus_shield>0) {
           left_bonus_shield-- ;
           if (left_bonus_shield==0) {
             drawCharAt(SHIELD_POS_X,SIZEY+2,SPACE) ;
           }
           else {
             setColor(Green) ;
             drawDigitAt(SHIELD_POS_X,SIZEY+2,left_bonus_shield) ;
           }
         }
         score+=INC_SCORE_BY_ONESEC ;
         setColor(Green) ;
         drawUIntAt(SCORE_POS_X,SIZEY+2,score) ;
       }
       if (key==KEY_KT) {
          setColor(Red) ;
          drawStringAt(1,SIZEY+3,"GAMEOVER, PRESS ENTER") ;
          break ;
       }
       ticks_common++ ;
       ticks_player++ ;
       ticks_enemy++ ;
       // Число тактов связано с FPS=10 и устанавливается в таймере
       if (ticks_common>10) ticks_common=0 ;
       if (ticks_player>T_player) ticks_player=0 ;
       if (ticks_enemy>T_enemy) ticks_enemy=0 ;

       waitFrameEnd() ;
    }
    while (keyHolded()!=KEY_ENTER) ;
    goto Game ;
}
