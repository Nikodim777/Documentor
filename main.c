#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <wchar.h>
#include <windows.h>

#define MAXLENBUF 256
#define MAXLENCOL 32 //Максимальное длина названия цвета
#define MAXLENHDR 32 //Масимальная длина текста заголовка
#define MAXLENLNG 4 //Максимальная длина строки названия языка
#define NUMCOLOURS 9
#define NUMLANGS 2
#define NUMTYPES 9 //Число типов документируемых объектов (функции, структуры и т.д.)

enum langs
{
    RUS = 0,
    EN = 1
};

enum colType //Перечисление хранящее типы объектов (меню, рамка таблицы и т.д.) для которых можно настроить свой цвет
{
    WARNING = 0,
    BACKGROUND = 1,
    SEPARATOR = 2,
    MENUBORDER = 3,
    MENUSELECT = 4,
    MENU = 5,
    TABLE1 = 6,
    TABLE2 = 7,
    TABLEBORDER = 8
};

enum objType //Перечисление хранящее типы документируемых объектов
{
    MODULES = 0,
    FUNCS = 1,
    GROUPFUNCS = 2,
    VARS = 3,
    STRUCTS = 4,
    ENUMS = 5,
    UNIONS = 6,
    MACRO = 7,
    GROUPMACRO = 8
};
#define FIRSTTYPE MODULES
#define LASTTYPE GROUPMACRO

struct elemList //Структура (часть связного списка) хранящяя информацию об одном документируемом объекте
{
    wchar_t *html;
    wchar_t name[MAXLENBUF];
    wchar_t modname[MAXLENBUF]; //Имя модуля которому принадлежит объект
    struct elemList* next;
};

struct elemList *beginList[NUMTYPES] = {NULL}, *curList[NUMTYPES] = {NULL};

enum langs curLang;
//Массив хранящий размеры строк HTML представления документируемых объектов разного типа
int sizesHtmlMembers[NUMTYPES] = {8192, 4096, 8192, 1024, 4096, 4096, 4096, 1024, 4096};


void newElem(enum objType type, wchar_t* name, wchar_t* modname)
{
    if(beginList[type] == NULL)
    {
        beginList[type] = (struct elemList*)malloc(sizeof(struct elemList));
        curList[type] = beginList[type];
    }
    else
    {
        curList[type]->next = (struct elemList*)malloc(sizeof(struct elemList));
        curList[type] = curList[type]->next;
    }
    curList[type]->html = (wchar_t*)malloc(sizesHtmlMembers[type] * sizeof(wchar_t));
    curList[type]->html[0] = '\0';
    wcscpy(curList[type]->name, name);
    wcscpy(curList[type]->modname, modname);
    curList[type]->next = NULL;
}

void addTableRow(enum objType type, wchar_t* col1, wchar_t* col2){
    wcscat(curList[type]->html, L"\t\t\t\t\t<tr>\n");
    wcscat(curList[type]->html, L"\t\t\t\t\t\t<td>");
    wcscat(curList[type]->html, col1);
    wcscat(curList[type]->html, L"</td>");
    wcscat(curList[type]->html, L"<td>");
    wcscat(curList[type]->html, col2);
    wcscat(curList[type]->html, L"</td>\n");
    wcscat(curList[type]->html, L"\t\t\t\t\t</tr>\n");
}

void setFunctionInfo(FILE* curFile, enum objType type){
    int flArg = 0, //Флаг наличия у функции аргументов
        status;
    wchar_t strBuf[MAXLENBUF], strBuf1[MAXLENBUF],strBuf2[MAXLENBUF];
    static wchar_t argumentLabel[NUMLANGS][MAXLENHDR] = {
        L"Аргументы:",
        L"Arguments:"
    };
    static wchar_t resultLabel[NUMLANGS][MAXLENHDR] = {
        L"Результат: ",
        L"Result: "
    };

    wcscat(curList[type]->html, L"\t\t\t<p>\n");
    while(fgetws(strBuf, MAXLENBUF, curFile))
    {
        if(wcsstr(strBuf, L"*/"))
        {
            if(flArg) //Если не аргументов, то не создавалась и таблица, и не нужны закрывающие теги
            {
                wcscat(curList[type]->html, L"\t\t\t\t</tbody>\n");
                wcscat(curList[type]->html, L"\t\t\t</table>\n\n");
            }
            break;
        }

        status = swscanf(strBuf, L"%*[ \t]\\%s", strBuf1); //Если в строке есть ключевое слово вида "\keyword" оно считывается
        if(status == 0) //Если простая строка информации
        {
            wcscat(curList[type]->html, L"\t\t\t\t");
            wcscat(curList[type]->html, strBuf);
            continue;
        }

        if(!wcscmp(strBuf1, L"param"))
        {
            if(flArg == 0) //Если первый аргумент
            {
                wcscat(curList[type]->html, L"\t\t\t</p>\n"); //Завершаем предыдущий параграф с общей информацией
                wcscat(curList[type]->html, L"\t\t\t<p>"); //Создаём таблицу аргументов
                wcscat(curList[type]->html, argumentLabel[curLang]);
                wcscat(curList[type]->html, L"</p>\n");
                wcscat(curList[type]->html, L"\t\t\t<table>\n");
                wcscat(curList[type]->html, L"\t\t\t\t<tbody>\n");
                flArg = 1;
            }
            status = swscanf(strBuf, L"%*[ \t]\\param %s %[^\0\n]", strBuf1, strBuf2);
            if(status == 2)
                addTableRow(type, strBuf1, strBuf2);
            else if(status == 1)
                addTableRow(type, strBuf1, L"");
        }
        else if(!wcscmp(strBuf1, L"result"))
        {
            if(flArg) //Если есть аргументы закрываем таблицу
            {
                wcscat(curList[type]->html, L"\t\t\t\t</tbody>\n");
                wcscat(curList[type]->html, L"\t\t\t</table>\n");
            }
            swscanf(strBuf, L"%*[ \t]\\result %[^\0\n]\n", strBuf1);
            wcscat(curList[type]->html, L"\t\t\t<p>");
            wcscat(curList[type]->html, resultLabel[curLang]);
            wcscat(curList[type]->html, strBuf1);
            wcscat(curList[type]->html, L"</p>\n");
            break;
        }
        else
            wprintf(L"Unknown keyword - %s\n", strBuf1);
    }
}

void setContentModule(FILE* curFile){
    wchar_t strBuf[MAXLENBUF];
    wcscpy(curList[MODULES]->name, curList[MODULES]->modname);
    while(fgetws(strBuf, MAXLENBUF, curFile))
        if(wcsstr(strBuf, L"*/"))
            break;
        else
        {
            wcscat(curList[MODULES]->html, L"\t\t\t\t");
            wcscat(curList[MODULES]->html, strBuf); //Добавляем строку общей информации к HTML представлению модуля
        }
}

void setContentVar(FILE* curFile){
    wchar_t strBuf[MAXLENBUF];

    wcscat(curList[VARS]->html, L"\t\t\t<p>\n");
    while(fgetws(strBuf, MAXLENBUF, curFile))
    {
        if(wcsstr(strBuf, L"*/"))
            break;

        wcscat(curList[VARS]->html, L"\t\t\t\t");
        wcscat(curList[VARS]->html, strBuf);
    }
    wcscat(curList[VARS]->html, L"\t\t\t</p>\n");
}

void setContentFunction(FILE* curFile){
    setFunctionInfo(curFile, FUNCS);
}

void setContentGroupFunction(FILE* curFile){
    int status;
    wchar_t strBuf[MAXLENBUF], strBuf1[MAXLENBUF], strBuf2[MAXLENBUF];
    static wchar_t listfuncsLabel[NUMLANGS][MAXLENHDR] = {
        L"Список функций:",
        L"List of functions:"
    };

    setFunctionInfo(curFile, GROUPFUNCS);

    while(fgetws(strBuf,MAXLENBUF,curFile)) //Переходим к началу группы функций
        if(wcsstr(strBuf, L"/*begingroup*/"))
            break;

    wcscat(curList[GROUPFUNCS]->html, L"\t\t\t<p>");
    wcscat(curList[GROUPFUNCS]->html, listfuncsLabel[curLang]);
    wcscat(curList[GROUPFUNCS]->html, L"</p>\n");
    wcscat(curList[GROUPFUNCS]->html, L"\t\t\t<table>\n");
    wcscat(curList[GROUPFUNCS]->html, L"\t\t\t\t<tbody>\n");
    while(fgetws(strBuf, MAXLENBUF, curFile))
    {
        if(wcsstr(strBuf, L"/*endgroup*/"))
            break;

        status = swscanf(strBuf, L"%[^;]; /*%[^*]*/", strBuf1, strBuf2);
        if(status == 2)
            addTableRow(GROUPFUNCS, strBuf1, strBuf2);
        else if(status == 1) //Если для конкретной функции нет описанин
            addTableRow(GROUPFUNCS, strBuf1, L"");
    }
    wcscat(curList[GROUPFUNCS]->html, L"\t\t\t\t</tbody>\n");
    wcscat(curList[GROUPFUNCS]->html, L"\t\t\t</table>\n\n");
}

void setContentStruct(FILE* curFile){
    int status;
    wchar_t strBuf[MAXLENBUF], strBuf1[MAXLENBUF], strBuf2[MAXLENBUF];
    static wchar_t structLabel[NUMLANGS][MAXLENHDR] = {
        L"Члены структуры:",
        L"Members of the struct:"
    };

    wcscat(curList[STRUCTS]->html, L"\t\t\t<p>\n");
    while(fgetws(strBuf, MAXLENBUF, curFile)){ //Добавление общей информации о структуре
        if(wcsstr(strBuf, L"*/"))
            break;

        wcscat(curList[STRUCTS]->html, L"\t\t\t\t");
        wcscat(curList[STRUCTS]->html, strBuf);
    }
    wcscat(curList[STRUCTS]->html, L"\t\t\t</p>\n");

    fgetws(strBuf, MAXLENBUF, curFile); //Пропуск строки с ключевым словом struct

    wcscat(curList[STRUCTS]->html, L"\t\t\t<p>");
    wcscat(curList[STRUCTS]->html, structLabel[curLang]);
    wcscat(curList[STRUCTS]->html, L"</p>\n");
    wcscat(curList[STRUCTS]->html, L"\t\t\t<table>\n");
    wcscat(curList[STRUCTS]->html, L"\t\t\t\t<tbody>\n");
    while(fgetws(strBuf, MAXLENBUF, curFile))
    {
        if(wcsstr(strBuf, L"}"))
            break;

        status = swscanf(strBuf, L"%[^;]; /*%[^*]*/", strBuf1, strBuf2); //Считывание инфы о члене структуры
        if(status == 2)
            addTableRow(STRUCTS, strBuf1, strBuf2);
        else if(status == 1)
            addTableRow(STRUCTS, strBuf1, L"");
    }
    wcscat(curList[STRUCTS]->html, L"\t\t\t\t</tbody>\n");
    wcscat(curList[STRUCTS]->html, L"\t\t\t</table>\n\n");
}

void setContentEnum(FILE* curFile){
    int status;
    wchar_t strBuf[MAXLENBUF], strBuf1[MAXLENBUF], strBuf2[MAXLENBUF];
    static wchar_t enumLabel[NUMLANGS][MAXLENHDR] = {
        L"Константы перечисления:",
        L"Constants of the enum:"
    };

    wcscat(curList[ENUMS]->html, L"\t\t\t<p>\n");
    while(fgetws(strBuf, MAXLENBUF, curFile)){
        if(wcsstr(strBuf, L"*/"))
            break;

        wcscat(curList[ENUMS]->html, L"\t\t\t\t");
        wcscat(curList[ENUMS]->html, strBuf);
    }
    wcscat(curList[ENUMS]->html, L"\t\t\t</p>\n");

    fgetws(strBuf, MAXLENBUF, curFile); //Пропуск строки с ключевым словом enum

    wcscat(curList[ENUMS]->html, L"\t\t\t<p>");
    wcscat(curList[ENUMS]->html, enumLabel[curLang]);
    wcscat(curList[ENUMS]->html, L"</p>\n");
    wcscat(curList[ENUMS]->html, L"\t\t\t<table>\n");
    wcscat(curList[ENUMS]->html, L"\t\t\t\t<tbody>\n");
    while(fgetws(strBuf, MAXLENBUF, curFile))
    {
        if(wcsstr(strBuf, L"}"))
            break;

        status = swscanf(strBuf, L"%*[ \t]%[a-zA-Z0-9_]%*[^/]/*%[^*]*/", strBuf1, strBuf2);
        if(status == 2)
            addTableRow(ENUMS, strBuf1, strBuf2);
        else if(status == 1)
            addTableRow(ENUMS, strBuf1, L"");
    }
    wcscat(curList[ENUMS]->html, L"\t\t\t\t</tbody>\n");
    wcscat(curList[ENUMS]->html, L"\t\t\t</table>\n\n");
}

void setContentUnion(FILE* curFile){
    int status;
    wchar_t strBuf[MAXLENBUF], strBuf1[MAXLENBUF], strBuf2[MAXLENBUF];
    static wchar_t unionLabel[NUMLANGS][MAXLENHDR] = {
        L"Члены объединения:",
        L"Members of the union:"
    };

    wcscat(curList[UNIONS]->html, L"\t\t\t<p>\n");
    while(fgetws(strBuf, MAXLENBUF, curFile)){
        if(wcsstr(strBuf, L"*/"))
            break;

        wcscat(curList[UNIONS]->html, L"\t\t\t\t");
        wcscat(curList[UNIONS]->html, strBuf);
    }
    wcscat(curList[UNIONS]->html, L"\t\t\t</p>\n");

    fgetws(strBuf, MAXLENBUF, curFile); //Пропуск строки с ключевым словом union

    wcscat(curList[UNIONS]->html, L"\t\t\t<p>");
    wcscat(curList[UNIONS]->html, unionLabel[curLang]);
    wcscat(curList[UNIONS]->html, L"</p>\n");
    wcscat(curList[UNIONS]->html, L"\t\t\t<table>\n");
    wcscat(curList[UNIONS]->html, L"\t\t\t\t<tbody>\n");
    while(fgetws(strBuf, MAXLENBUF, curFile))
    {
        if(wcsstr(strBuf, L"}"))
            break;

        status = swscanf(strBuf, L"%[^;]; /*%[^*]*/", strBuf1, strBuf2);
        if(status == 2)
            addTableRow(UNIONS, strBuf1, strBuf2);
        else if(status == 1)
            addTableRow(UNIONS, strBuf1, L"");
    }
    wcscat(curList[UNIONS]->html, L"\t\t\t\t</tbody>\n");
    wcscat(curList[UNIONS]->html, L"\t\t\t</table>\n\n");
}

void setContentMacro(FILE* curFile){
    wchar_t strBuf[MAXLENBUF];
    static wchar_t valueLabel[NUMLANGS][MAXLENHDR] = {
        L"Значение: ",
        L"Value: "
    };

    wcscat(curList[MACRO]->html, L"\t\t\t<p>\n");
    while(fgetws(strBuf, MAXLENBUF, curFile))
    {
        if(wcsstr(strBuf, L"*/"))
            break;

        wcscat(curList[MACRO]->html, L"\t\t\t\t");
        wcscat(curList[MACRO]->html, strBuf);
    }

    fwscanf(curFile, L"#define %*s %s\n", strBuf);
    wcscat(curList[MACRO]->html, L"\t\t\t\t<br>");
    wcscat(curList[MACRO]->html, valueLabel[curLang]);
    wcscat(curList[MACRO]->html, strBuf);
    wcscat(curList[MACRO]->html, L"\n\t\t\t</p>\n");
}

void setContentGroupMacro(FILE* curFile){
    int status;
    wchar_t strBuf[MAXLENBUF], strBuf1[MAXLENBUF], strBuf2[MAXLENBUF];
    static wchar_t listmacroLabel[NUMLANGS][MAXLENHDR] = {
        L"Список макросов:",
        L"List of macro:"
    };

    wcscat(curList[GROUPMACRO]->html, L"\t\t\t<p>\n");
    while(fgetws(strBuf,MAXLENBUF,curFile))
    {
        if(wcsstr(strBuf, L"*/"))
            break;

        wcscat(curList[GROUPMACRO]->html, L"\t\t\t\t");
        wcscat(curList[GROUPMACRO]->html, strBuf);
    }
    wcscat(curList[GROUPMACRO]->html, L"\t\t\t</p>\n");

    while(fgetws(strBuf,MAXLENBUF,curFile))
        if(wcsstr(strBuf, L"/*begingroup*/"))
            break;

    wcscat(curList[GROUPMACRO]->html, L"\t\t\t<p>");
    wcscat(curList[GROUPMACRO]->html, listmacroLabel[curLang]);
    wcscat(curList[GROUPMACRO]->html, L"</p>\n");
    wcscat(curList[GROUPMACRO]->html, L"\t\t\t<table>\n");
    wcscat(curList[GROUPMACRO]->html, L"\t\t\t\t<tbody>\n");
    while(fgetws(strBuf, MAXLENBUF, curFile))
    {
        if(wcsstr(strBuf, L"/*endgroup*/"))
            break;

        status = swscanf(strBuf, L"#define %s %[^\0\n]", strBuf1, strBuf2);
        if(status == 2)
            addTableRow(GROUPMACRO, strBuf1, strBuf2);
        else if(status == 1)
            addTableRow(GROUPMACRO, strBuf1, L"");
    }
    wcscat(curList[GROUPMACRO]->html, L"\t\t\t\t</tbody>\n");
    wcscat(curList[GROUPMACRO]->html, L"\t\t\t</table>\n\n");
}

//Функция анализирует файлы и заполняет связные списки
void analyzeFiles(LPWSTR* filenameList, int numFiles)
{
    FILE* curFile = NULL;
    int i, j, status;
    wchar_t strBuf[MAXLENBUF], strBuf1[MAXLENBUF], strBuf2[MAXLENBUF];
    char filename[MAXLENBUF];

    static wchar_t keywords[NUMTYPES][MAXLENBUF] = {
        L"file",
        L"function",
        L"funcgroup",
        L"var",
        L"struct",
        L"enum",
        L"union",
        L"macro",
        L"macrogroup"
    };
    //Массив функций заполнения HTML представления документируемых объектов различного типа
    void (*setContent[NUMTYPES])(FILE* curFile) = {setContentModule, setContentFunction, setContentGroupFunction,
                                                    setContentVar, setContentStruct, setContentEnum,
                                                    setContentUnion, setContentMacro, setContentGroupMacro};

    for(i = 0; i < numFiles; i++) //Цикл по документируемым файлам
    {
        wcstombs(filename, filenameList[i], sizeof(filename));
        curFile = fopen(filename, "r, ccs=UTF-8");
        if(curFile == NULL)
        {
            printf("%s - ", filename);
            perror("error opening the file");
            continue;
        }

        while(fgetws(strBuf,MAXLENBUF,curFile)) //Считывание файла построчно
        {
            status = swscanf(strBuf, L"/*\\%s %s", strBuf1, strBuf2); //Определение строки типа и имени документируемого объекта
            if(!status) //Если просто строка
                continue;

            for(j = FIRSTTYPE; j <= LASTTYPE; j++) //Цикл по ключевым словам
                if(!wcscmp(strBuf1, keywords[j])) //Если строковое представление ключевого слова совпадает с прочитанным
                {
                    newElem(j, strBuf2, filenameList[i]); //Создать новый элемент соответствующего типа
                    setContent[j](curFile); //Вызвать соответствующую функцию заполнения HTML представления для этого элемента
                    break;
                }
            if(j > GROUPMACRO)
                wprintf(L"Undefined lexem - %s\n",strBuf1);
        }

        status = fclose(curFile);
        if(status)
        {
            printf("%s - ", filename);
            perror("error closing the file");
        }
    }
}

//Создание элемента меню в HTML файле
void printLi(FILE* htmlFile, enum objType type, wchar_t* nameLi)
{
    struct elemList* iterList;
    fputws(L"\t\t\t<li onmouseover=\"this.getElementsByTagName('DIV')[0].style.display = 'block'\"\
 onmouseout=\"this.getElementsByTagName('DIV')[0].style.display = 'none'\">\n", htmlFile);
    fwprintf(htmlFile, L"\t\t\t\t<h4>%s</h4>\n", nameLi);
    fputws(L"\t\t\t\t<div>\n", htmlFile);
    for(iterList = beginList[type]; iterList != NULL; iterList = iterList->next)
        fwprintf(htmlFile, L"\t\t\t\t\t<a href=\"#%s\">%s</a>\n", iterList->name, iterList->name);
    fputws(L"\t\t\t\t</div>\n", htmlFile);
    fputws(L"\t\t\t</li>\n", htmlFile);
}

int main()
{
    int argc;
    LPWSTR* argv;
    //Получение строки аргументов типа wchar_t !!!!System dependence!!!!
    argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    setlocale(LC_ALL,"rus");

    FILE* configFile = NULL;
    FILE* htmlFile = NULL;
    FILE* cssFile = NULL;
    wchar_t strBuf[MAXLENLNG];
    static wchar_t langs[NUMLANGS][MAXLENLNG] = {
        L"RUS",
        L"ENG"
    };
    static wchar_t colors[NUMCOLOURS][MAXLENCOL] = {
        L"Maroon",
        L"SkyBlue",
        L"DarkCyan",
        L"LightSalmon",
        L"Chocolate",
        L"Coral",
        L"LightSalmon",
        L"DarkSalmon",
        L"Chocolate"
    };
    static wchar_t headers[NUMLANGS][NUMTYPES][MAXLENHDR] = {
        {
            L"Модули", L"Функции", L"Группы функций", L"Переменные", L"Структуры",
            L"Перечисления", L"Объединения", L"Макросы", L"Группы макросов"
        },
        {
            L"Modules", L"Functions", L"Groupfunctions", L"Vars",
            L"Structs", L"Enums", L"Unions", L"Macro", L"Groupmacro"
        }
    };
    static wchar_t backLabel[NUMLANGS][MAXLENHDR] = {
        L"Назад",
        L"Back"
    };
    static wchar_t moduleLabel[NUMLANGS][MAXLENHDR] = {
        L"модуля",
        L"of the module"
    };
    int i, status,
        flExist; //Флаг существования документируемых объектов определённого типа для определённого модуля

    struct elemList *iterList1, *iterList2;

    if(argc == 2 && !wcscmp(argv[1], L"/?"))
    {
        puts("Format: documentor nameproject listfiles");
        return 0;
    }
    else if(argc <= 2)
    {
        puts("Enter list filenames for processing");
        return 1;
    }

    configFile = fopen("config.txt", "r, ccs=UTF-8");
    if(configFile == NULL)
        puts("There are using standart css style");
    else
    {
        fwscanf(configFile, L"%*s\t%s\n", strBuf);
        for(i = 0; i < NUMLANGS; i++) //Определение языка
            if(!wcscmp(langs[i], strBuf))
            {
                curLang = i;
                break;
            }
        if(i >= NUMLANGS)
        {
            printf("Unknown language in config file!\n");
            curLang = 0;
        }

        fwscanf(configFile, L"%*[^\n]\n");
        for(i = 0; i < NUMCOLOURS; i++) //Считывание цветов из файла конфигурации
            fwscanf(configFile, L"%*s\t%s\n", colors[i]);

        fwscanf(configFile, L"%*[^\n]\n");
        for(i = FIRSTTYPE; i <= LASTTYPE; i++) //Считывание размеров блока HTML  для каждого типа объекта
            fwscanf(configFile, L"%*s%*[\t]%d\n", &sizesHtmlMembers[i]);

        status = fclose(configFile);
        if(status)
            perror("Error closing the config file");
    }

    cssFile = fopen("index.css", "w, ccs=UTF-8");
    if(cssFile == NULL)
    {
        perror("Error opening the css file");
        return 1;
    }

    //Заполнение css файла
    fwprintf(cssFile, L"body{\n\tbackground: %s\n}\n", colors[BACKGROUND]);
    fputws(L"nav > li > div > a,h2,h3,h4{\n\ttext-align: center;\n}\n", cssFile);
    fputws(L"nav{\n\
\tdisplay: flex;\n\
\tflex-direction: row;\n\
\tjustify-content: center;\n\
\talign-items: flex-start;\n\
\talign-content: space-between;\n}\n", cssFile);
    fwprintf(cssFile, L"nav > li{\n\
\tdisplay: block;\n\
\twidth: 20%%;\n\
\tbackground: %s;\n\
\tcursor: pointer;\n}\n", colors[MENU]);
    fwprintf(cssFile, L"nav > li:not(:last-child) > h4{\n\
\tborder-width: 0px medium 0px 0px;\n\
\tborder-style: groove;\n\
\tborder-color: %s;\n}\n", colors[MENUBORDER]);
    fputws(L"nav > li > div{\n\tdisplay: none;\n}\n", cssFile);
    fputws(L"nav > li > div > a , nav > li > div > a:active{\n\
\tdisplay: block;\n\
\tcolor: black;\n\
\ttext-decoration: none;\n}\n", cssFile);
    fwprintf(cssFile, L"nav > li > div > a:hover{\n\
\tbackground: %s;\n\
\ttext-decoration: underline;\n}\n", colors[MENUSELECT]);
    fwprintf(cssFile, L"hr{\n\
\tbackground: %s;\n\
\tborder-style: none;\n\
\tborder-radius: 3px;\n}\n", colors[SEPARATOR]);
    fputws(L"hr.type-sep{\n\theight: 3px;\n}\n", cssFile);
    fputws(L"hr.module-sep{\n\theight: 7px;\n}\n", cssFile);
    fputws(L"main > a,main > p,main > table{\n\
\tmargin-left: 10%;\n\
\tmargin-right: 10%;\n}\n", cssFile);
    fwprintf(cssFile, L"span{\n\
\tcolor: %s;\n\
\tfont-weight: bold;\n}\n", colors[WARNING]);
    fwprintf(cssFile, L"table{\n\
\tborder: 3px solid %s;\n\
\tborder-spacing: 0px;\n}\n", colors[TABLEBORDER]);
    fwprintf(cssFile, L"tr:nth-child(2n){\n\tbackground: %s;\n}\n", colors[TABLE1]);
    fwprintf(cssFile, L"tr:nth-child(2n+1){\n\tbackground: %s;\n}\n", colors[TABLE2]);
    fwprintf(cssFile, L"td:not(:last-child){\n\
\tborder-width: 0px 2px 0px 0px;\n\
\tborder-style: solid;\n\
\tborder-color: %s;\n}\n", colors[TABLEBORDER]);
    fputws(L"@media (max-width: 800px){\n", cssFile);
    fputws(L"\tnav{\n\
\t\tflex-direction: column;\n\
\t\talign-items: center;\n\t}\n", cssFile);
    fputws(L"\tnav > li{\n\t\twidth: 40%;\n\t}\n", cssFile);
    fwprintf(cssFile, L"\tnav > li:not(:last-child){\n\
\t\tborder-width: 0px 0px medium 0px;\n\
\t\tborder-style: groove;\n\
\t\tborder-color: %s;\n\t}\n", colors[MENUBORDER]);
    fputws(L"\tnav > li:not(:last-child) > h4{\n\t\tborder-style: none;\n\t}\n", cssFile);
    fputws(L"}\n", cssFile);


    status = fclose(cssFile);
    if(status)
        perror("Error closing the css file");

    analyzeFiles(&argv[2], argc - 2); //Заполнение связных списков

    htmlFile = fopen("index.html", "w, ccs=UTF-8");
    if(htmlFile == NULL)
    {
        perror("Error opening the html file");
        return 1;
    }

    fputws(L"<html>\n", htmlFile);
    fputws(L"\t<head>\n", htmlFile);
    fputws(L"\t\t<meta charset=\"utf-8\">\n", htmlFile);
    fputws(L"\t\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n", htmlFile);
    fwprintf(htmlFile, L"\t\t<title>%s</title>\n", argv[1]);
    fputws(L"\t\t<link rel=\"stylesheet\" href=\"index.css\">\n", htmlFile);
    fputws(L"\t</head>\n", htmlFile);
    fputws(L"\t<body>\n", htmlFile);
    fputws(L"\t\t<a name=\"nav\"></a>\n", htmlFile);
    fputws(L"\t\t<nav>\n", htmlFile);

    //Создание выпадающего меню
    for(i = FIRSTTYPE; i <= LASTTYPE; i++)
        if(beginList[i] != NULL)
            printLi(htmlFile, i, headers[curLang][i]);

    fputws(L"\t\t</nav>\n", htmlFile);
    fputws(L"\t\t<main>\n", htmlFile);

    //Создание тела документации
    for(iterList1 = beginList[MODULES]; iterList1 != NULL; iterList1 = iterList1->next) //Цикл по модулям
    {
        //Вывод инфы о модуле
        if(iterList1 != beginList[MODULES])
            fwprintf(htmlFile, L"\t\t\t<a name=\"%s\" href=\"#nav\">%s</a>\n", iterList1->name, backLabel[curLang]);
        fwprintf(htmlFile, L"\t\t\t<h2>%s</h2>\n", iterList1->name);
        fputws(L"\t\t\t<p>\n", htmlFile);
        fputws(iterList1->html, htmlFile);
        fputws(L"\t\t\t</p>\n", htmlFile);

        //Цикл по типам документируемых объектов для данного модуля
        for(i = FUNCS; i <= LASTTYPE; i++)
        {
            flExist = 0;
            //Проверка существования документируемых объектов данного типа для данного модуля
            for(iterList2 = beginList[i]; iterList2 != NULL; iterList2 = iterList2->next)
                if(!wcscmp(iterList1->name, iterList2->modname))
                {
                    flExist = 1;
                    break;
                }

            if(flExist)
            {
                fputws(L"\n\n\t\t\t<hr class=\"type-sep\">\n", htmlFile);
                fwprintf(htmlFile, L"\t\t\t<h3>%s %s</h3>\n", headers[curLang][i], moduleLabel[curLang]);
            }
            for(iterList2 = beginList[i]; iterList2 != NULL; iterList2 = iterList2->next) //Цикл по списку объектов данного типа
            {
                if(wcscmp(iterList1->name, iterList2->modname)) //Если объект не принадлежит модулю
                    continue;
                fwprintf(htmlFile, L"\t\t\t<a name=\"%s\" href=\"#nav\">%s</a>\n", iterList2->name, backLabel[curLang]);
                fwprintf(htmlFile, L"\t\t\t<h4>%s</h4>\n", iterList2->name);
                fputws(iterList2->html, htmlFile);
            }
        }

        if(iterList1->next != NULL)
            fputws(L"\n\n\n\t\t\t<hr class=\"module-sep\">\n\n\n\n", htmlFile);
    }

    fputws(L"\t\t</main>\n", htmlFile);
    fputws(L"\t</body>\n", htmlFile);
    fputws(L"</html>", htmlFile);

    status = fclose(htmlFile);
    if(status)
        perror("Error closing the html file");

    return 0;
}
