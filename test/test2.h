#ifndef ARRAY_H_INCLUDED
#define ARRAY_H_INCLUDED

/*\file
    Вот это модуль №2<br>
    <span>Не играйте со спичками!</span>
*/

#include <memory.h>

/*\macro MAXSIZE
    Максимальный размер
*/
#define MAXSIZE 4
/*\macro MINSIZE
    Минимальный размер
*/
#define MINSIZE 2

/*\var venture
    Имя компании
*/
char* venture;

/*\enum currency
    Перечисление хранящее тип валюты
*/
enum currency {
    ROUBLE, /* Рубль */
    DOLLAR, /* Доллар */
    EURO  /* Евро */
};

/*\function moneyprint
    Функция печатает деньги в указанной валюте в определённом количестве.
    \param value Требуемое колво денег.
    \param curr Требуемая валюта.
*/
void moneyprint(int value, enum currency curr);
/*Макросы печати рублей*/
#define ruprint(value) moneyprint((value),currency.ROUBLE);

/*\function Sum
    Функция подсчитывает сумму элементов массива
    \result Сумма элементов массива
*/
int Sum(void *mas);

/*\struct anyman
    Структура хранящяя инфу о мужике
*/
struct anyman{
    char name[25]; /* Имя */
    int life; /* Жизнь */
    float drunk; /* Выпито спирта в литрах */
};

/*\union foo
    Объединение хранящее представление числа в виде тетрады символов
*/
union foo{
    char str[MAXSIZE]; /* Число в виде строки */
    int num; /* Число */
};

#endif
