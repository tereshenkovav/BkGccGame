# Игра на Си для БК-0010-01 с использованием сборки gcc-pdp11

[![YooMoney donation](https://img.shields.io/badge/Donation-Yoo.money-blue.svg)](https://yoomoney.ru/to/41001497003495)
[![Website](https://img.shields.io/badge/Website-tav--developer.itch.io-29a662.svg)](https://tav-developer.itch.io)
[![License](https://img.shields.io/badge/License-MIT0-darkgray.svg)](#)
[![Tag](https://img.shields.io/github/v/tag/tereshenkovav/BkGccGame?color=00c2e8)](#)
[![Downloads](https://img.shields.io/github/downloads/tereshenkovav/BkGccGame/total?color=c87bff)](#)

Проект представляет собой простую игру для компьютера БК-0010, написанную
на Си с ассемблерными вставками и собираемую в бинарный файл
с помощью компилятора gcc для pdp-11.

Концепция игры - управляя движением игрока, уклоняться от врагов, 
зарабатывая очки за время выживания, подбирание бонусов и истребление врагов
при контакте, если активен бонус защиты. Игра не использует графические
примитивы, только псевдографику.

![BkGccGame](screen1.png) ![BkGccGame](screen2.png)

## Руководство по сборке

В каталоге `src` находится проект в виде файла исходного кода и скрипт сборки:

* build.bat - сборка по шагам без использования Make
* bkgccgame.c - код игры
* bk0010_rtl.c - код процедур, необходимых для разработки под БК-0010
* bk0010_rtl.h - заголовочный файл процедур, необходимых для разработки под БК-0010
* const_en.h - заголовочный файл с английскими текстами и константами позиционирования
* const_ru.h - заголовочный файл с русскими текстами и константами позиционирования

Сторонние файлы, необходимые для сборки, взяты из проекта [Digger](https://github.com/prcoder-1/digger-bk0010)

* a.out.ld - файл настроек для линкера
* crt0.s - модуль с функцией start, в нем убрана установка скролла и запись в прерывание 4 адреса start
* memory.s - функция memset для компилятор Си
* stdint.h - описатель типов данных для БК-0010

Для сборки также нужен файл `aout2bin.exe`, который можно получить из исходника
`aout2bin.c` в проекте Diggger
