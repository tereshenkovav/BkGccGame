#include "stdint.h"
#include "bk0010_rtl.h"

#ifdef GAMELANG_EN
#include "const_en.h"
#endif
#ifdef GAMELANG_RU
#include "const_ru.h"
#endif

// Функции логики
enum BonusType { btNone=0, btSpeedUp=1, btScore=2, btShield=3 } ;

// Структура бонуса
struct Bonus {
  uint8_t x ;
  uint8_t y ;
  enum BonusType t ;
};

// Структура врагов
struct Enemy {
  uint8_t x ;
  uint8_t y ;
  uint8_t exist ;
};

// Символы для бонусов
const char BONUS_CHARS[4] = { 0, 053, 052, 045 } ;

// Переменные игры
const uint16_t MAXBONUS = 16 ;
const uint16_t MAXENEMY = 4 ;
struct Bonus bonuses[16] ;
struct Enemy enemy[16] ;
uint8_t playerx ;
uint8_t playery ;
uint16_t T_enemy ;
uint16_t T_newenemy ;
uint16_t T_player ;

// Символы для вывода героя, врага и пробела
const char HERO = 0100 ;
const char ENEMY = 044 ;
const char SPACE = 040 ;

// Размеры поля и точки спавна врагов
const uint8_t BORDER = 1 ;
const uint8_t SIZEX = 30 ;
const uint8_t SIZEY = 20 ;
const uint8_t SPAWNX[4] = {BORDER,SIZEX,BORDER,SIZEX} ;
const uint8_t SPAWNY[4] = {BORDER,BORDER,SIZEY,SIZEY} ;

const uint8_t SCORE_POS_X = 26 ;

// Константы игрового баланса
const uint16_t INC_SCORE_BY_BONUS = 100 ;
const uint16_t INC_SCORE_BY_KILLENEMY = 50 ;
const uint16_t INC_SCORE_BY_ONESEC = 10 ;

const uint16_t LIFETIME_FOR_BONUS = 9 ;
const uint16_t MOVE_PERIOD_ENEMY = 5 ;
const uint16_t MOVE_PERIOD_PLAYER = 6 ;
const uint16_t MOVE_PERIOD_PLAYER_FAST = 3 ;
const uint16_t PERIOD_NEW_ENEMY = 100 ;

// Переменные настройки игры - звук и джойстик
uint16_t soundon = 1 ;
uint16_t joyon = 0 ;

// Новый враг
uint16_t newEnemy() {
   uint8_t idx = genRndByByteN2(2) ;
   for (uint16_t i=0; i<MAXENEMY; i++)
     if (!enemy[i].exist) {
       enemy[i].x = SPAWNX[idx] ;
       enemy[i].y = SPAWNY[idx] ;
       enemy[i].exist=1 ;
       return i ;
     }
   return MAXENEMY ;
}

// Возврат MAXBONUS - означает, что нет индекса
uint16_t getBonusIdxAt(uint8_t x, uint8_t y) {
   for (uint16_t i=0; i<MAXBONUS; i++)
     if (bonuses[i].t!=btNone)
       if ((x==bonuses[i].x)&&(y==bonuses[i].y))
         return i ;
   return MAXBONUS ;
}

// Новый бонус
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

// Двигать игрока
void movePlayer(int8_t dx, int8_t dy) {
    drawCharAt(playerx,playery,SPACE) ;
    playerx+=dx ;
    playery+=dy ;
    setColor(Green) ;
    drawCharAt(playerx,playery,HERO) ;
}

// Двигать врага по индексу
void moveEnemy(int16_t i, uint8_t dx, uint8_t dy) {
    drawCharAt(enemy[i].x,enemy[i].y,SPACE) ;
    uint16_t idx = getBonusIdxAt(enemy[i].x,enemy[i].y) ;
    if (idx!=MAXBONUS) {
      setColor(Blue) ;
      drawCharAt(bonuses[idx].x,bonuses[idx].y,BONUS_CHARS[bonuses[idx].t]) ;
    }
    enemy[i].x+=dx ;
    enemy[i].y+=dy ;
    setColor(Red) ;
    drawCharAt(enemy[i].x,enemy[i].y,ENEMY) ;
}

// Звуковые эффекты
inline void playBonusEffect() {
   if (!soundon) return ;
   playSound(057,020) ;
   playSound(0127,010) ;
}

inline void playGameOverEffect() {
   if (!soundon) return ;
   playSound(0100,047) ;
   playSound(060,077) ;
   playSound(040,0147) ;
}

void PrintMenu() {
    ClearScreen() ;
    setColor(Green) ;
    drawStringAt(2,3,GAMETITLE) ;
    drawStringAt(5,6,MENU_START) ;
    drawStringAt(5,7,MENU_SOUND) ;
    drawStringAt(5,8,MENU_JOY) ;
    drawStringAt(5,9,MENU_HELP) ;
    drawStringAt(5,10,MENU_EXIT) ;

    drawStringAt(POS_TEXT_SOUND_ON_OFF,7,soundon?TEXT_ON:TEXT_OFF) ;
    drawStringAt(19,8,joyon?TEXT_ON:TEXT_OFF) ;

    drawStringAt(2,13,TEXT_AUTHOR) ;
    drawStringAt(2,14,"github.com/tereshenkovav") ;
    drawStringAt(2,15,"/BkGccGame") ;
}

void PrintHelpAndWaitEnter() {
    ClearScreen() ;
    setColor(Green) ;
    drawStringAt(2,3,HELP0) ;
    drawStringAt(2,4,HELP1) ;
    drawStringAt(2,5,HELP2) ;
    drawStringAt(2,6,HELP3) ;
    setColor(Blue) ;
    drawStringAt(2,8,HELP4) ;
    drawStringAt(2,9,HELP5) ;
    drawStringAt(2,10,HELP6) ;
    setColor(Green) ;
    drawStringAt(2,12,HELP7) ;
    while (keyHolded()!=KEY_ENTER) ;
}

void MainGame() {
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
    drawStringAt(1,SIZEY+2,BONUS_SPEEDUP) ;
    drawStringAt(POS_TEXT_BONUS_SHIELD,SIZEY+2,BONUS_SHIELD) ;
    drawStringAt(POS_TEXT_BONUS_SCORE,SIZEY+2,BONUS_SCORE) ;

    playerx = 15 ;
    playery = 10 ;

    for (uint16_t i=0; i<MAXENEMY; i++)
      enemy[i].exist = 0 ;
    newEnemy() ;

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
    for (uint16_t i=0; i<MAXENEMY; i++)
      if (enemy[i].exist)
        drawCharAt(enemy[i].x,enemy[i].y,ENEMY) ;

    setColor(Blue) ;
    for (uint16_t i=0; i<MAXBONUS; i++)
      if (bonuses[i].t!=btNone)
        drawCharAt(bonuses[i].x,bonuses[i].y,BONUS_CHARS[bonuses[i].t]) ;

    uint16_t ticks_common = 0 ;
    uint16_t ticks_enemy = 0 ;
    uint16_t ticks_player = 0 ;
    uint16_t ticks_newenemy = 0 ;
    uint8_t left_bonus_speed = 0 ;
    uint8_t left_bonus_shield = 0 ;
    uint8_t lastkey = 0 ;

    T_enemy = MOVE_PERIOD_ENEMY ;
    T_player = MOVE_PERIOD_PLAYER ;
    T_newenemy = PERIOD_NEW_ENEMY ;

    setColor(Green) ;
    drawUIntAt(SCORE_POS_X,SIZEY+2,score) ;

    for (;;) {
       startTimer(03777) ; // 03777 - примерно 0.1 с

       // Проверка нажатия
       uint8_t key = keyHolded() ;

       // Дополнительная обработка джойстика
       if ((key==0)&&(joyon)) {
          uint16_t joykey = readWord(0177714) ;
          if ((joykey & 01)!=0) key=KEY_UP ;
          if ((joykey & 02)!=0) key=KEY_RIGHT ;
          if ((joykey & 04)!=0) key=KEY_DOWN ;
          if ((joykey & 010)!=0) key=KEY_LEFT ;
       }

       // Если новая клавиша нажата, не равная старой
       if ((key!=0)&&(key!=lastkey)) {
         ticks_player = 0 ;
         lastkey=key ;
       }
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
               playBonusEffect() ;
         }
       }
       if (ticks_enemy==0) { // Ограничения по тактам
        for (uint16_t i=0; i<MAXENEMY; i++)
         if (enemy[i].exist) {
         int8_t dx=0 ;
         int8_t dy=0 ;
         if (iabs(playerx-enemy[i].x)>iabs(playery-enemy[i].y)) {
           if (playerx<enemy[i].x) dx=-1 ;
           if (playerx>enemy[i].x) dx=1 ;
         }
         else {
           if (playery<enemy[i].y) dy=-1 ;
           if (playery>enemy[i].y) dy=1 ;
         }
         moveEnemy(i,dx,dy) ;
         if ((iabs(playerx-enemy[i].x)<2)&&(iabs(playery-enemy[i].y)<2)) {
           if (left_bonus_shield==0) {
             setColor(Red) ;
             drawStringAt(1,SIZEY+3,TEXT_GAMEOVER) ;
	     playGameOverEffect() ;
             goto Finish ;
           }
           else {
             score+=INC_SCORE_BY_KILLENEMY ;
             // Перерисовка игрока или затирание монстра, по ситуации
             if ((enemy[i].x==playerx)&&(enemy[i].y==playery)) {
               setColor(Green) ;
               drawCharAt(playerx,playery,HERO) ;
             }
             else
               drawCharAt(enemy[i].x,enemy[i].y,SPACE) ;
             enemy[i].exist=0 ;
             playBonusEffect() ;
           }
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
          drawStringAt(1,SIZEY+3,TEXT_GAMEOVER) ;
          break ;
       }
       ticks_common++ ;
       ticks_player++ ;
       ticks_enemy++ ;
       ticks_newenemy++ ;
       // Число тактов связано с FPS=10 и устанавливается в таймере
       if (ticks_common>10) ticks_common=0 ;
       if (ticks_player>T_player) ticks_player=0 ;
       if (ticks_enemy>T_enemy) ticks_enemy=0 ;
       if (ticks_newenemy>T_newenemy) {
          ticks_newenemy=0 ;
          setColor(Red) ;
          uint16_t idx = newEnemy() ;
          if (idx!=MAXENEMY)
            drawCharAt(enemy[idx].x,enemy[idx].y,ENEMY) ;
       }

       waitFrameEnd() ;
    }
Finish:
    while (keyHolded()!=KEY_ENTER) ;
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

    char key ;
    uint16_t rnd = 0 ;
    while (1) {
      PrintMenu() ;
      do {
        Randomize(rnd++) ;
        key = keyHolded() ;
      }
      while ((key<'0')||(key>'4')) ;
      if (key=='1') MainGame() ;
      if (key=='2') soundon=1-soundon ;
      if (key=='3') joyon=1-joyon ;
      if (key=='4') PrintHelpAndWaitEnter() ;
      if (key=='0') break ;
    }
    EMT_14() ;
}
