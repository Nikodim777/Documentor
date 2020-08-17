#ifndef ARRAY_H_INCLUDED
#define ARRAY_H_INCLUDED

/*\file
    f - означает работу с типом float.<br>
    d - означает работу с типом double.<br>
    e - означает работу с типом double в экспоненциальном виде.<br>
    c - означает работу с типом complex.<br>
    d - означает работу с либой структурой данных.
*/

#include <memory.h>

/*\var largeSize_tp
    Беззнаковый тип для задания очень больших размеров
*/
typedef unsigned long long largeSize_tp;


/*\funcgroup printFuncs
    Функции служат для вывода значений базовых типов.<br>
    Можно создать аналогичные собзственные функции для вывода значений любого типа.<br>
    Пример_функции_вывода_элемента<br>
    void printDbl(const void *arg)<br>
    {<br>
        printf("%.6f\t", *(double*)arg);<br>
    }<br>
    \param arg Указатель на значение которое нужно вывести.
*/
/*begingroup*/
void printInt(const void *arg);
void printFlt(const void *arg);
void printDbl(const void *arg);
void printExp(const void *arg);
void printCmplx(const void *arg);
/*endgroup*/
/*\function aprint
    Функция предназначена для форматированного вывода одномерного или двумерного массива.
    \param title Указатель на строку заголовка печатаемого массива.
    \param print Указатель на функцию печатающую элемент определённого типа.
*/
void aprint(void *mas, const size_t szEl, const largeSize_tp size1, const largeSize_tp size2, const char *title,
            void (*print)(const void *arg));
/*\macrogroup printMacro
    Макросы для печати массива определённого типа
*/
/*begingroup*/
#define aprinti(mas,size1,size2,title) aprint((mas),sizeof(int),(size1),(size2),(title),printInt);
#define aprintf(mas,size1,size2,title) aprint((mas),sizeof(float),(size1),(size2),(title),printFlt);
#define aprintd(mas,size1,size2,title) aprint((mas),sizeof(double),(size1),(size2),(title),printDbl);
#define aprinte(mas,size1,size2,title) aprint((mas),sizeof(double),(size1),(size2),(title),printExp);
#define aprintc(mas,size1,size2,title) aprint((mas),sizeof(double complex),(size1),(size2),(title),printCmplx);
/*endgroup*/

/*\funcgroup fillFuncs
    Функции служат для заполнения массива обределённым образом.<br>
    step - заполняет целыми числами по порядку.<br>
    stepTwc - то же но с двойным шагом.<br>
    Функции с суффиксом Rnd - заполняют случайными числами.
    Можно создать аналогичные собственные функции.
    Пример_функции_изменения_элемента
    void setRnd(void* masEl, largeSize_tp num)<br>
    {<br>
        *(int*)masEl = rand() % 100;<br>
    }
    \param masEl Указатель на элемент массива значение которого нужно изменить.
    \param num Порядковый номер элемента.
    \result Ничего не возвращает
*/
/*begingroup*/
void step(void* masEl, largeSize_tp num);
void stepTwc(void* masEl, largeSize_tp num);
void setRnd(void* masEl, largeSize_tp num);
void setDblRnd(void* masEl, largeSize_tp num);
/*endgroup*/

/*\enum checkRes
    Перечисление хранящее результат соответствия элементов массива некоторому условию
*/
enum checkRes {
    RES_OK = 1, /* Соответствует */
    RES_NOT = 0, /* Не соответствует */
};

/*\enum typeAppr
    Перечисление хранящее тип соответствия элементов массива некоторому условию
*/
enum typeAppr {
    ALL_APPR, /* Все элементы соответствуют */
    ANY_APPR, /* Хотя бы один элемент соответствуют */
    CNT_APPR  /* Определённое число элементов соответствуют */
};

/*\function Appropriate
    Функция принимает в качестве параметров указатель на элемент массива
    и возвращает RES_NOT в случае несоответствия и RES_OK в случае соответствия.
    \param type Тип соответствия.
    \param checkEl Указатель на функцию проверки соответствия элемента массива определённому условию.
    \result Возвращает или признак соответствия/несоответствия или число соответствующих элементов в зависимости от типа соответствия
*/
int Appropriate(void *mas, const size_t szEl, const largeSize_tp size, enum typeAppr type,
                enum checkRes (*checkEl)(const void *masEl));

/*\struct fighter
    Структура хранящяя инфу о юните
*/
struct fighter{
    char name[25]; /* Имя */
    int life; /* Жизнь */
    float damage; /* Урон */
};

/*\union magic
    Объединение хранящее представление числа в виде тетрады символов
*/
union magic{
    char str[4]; /* Число в виде строки */
    int num; /* Число */
};

#endif
