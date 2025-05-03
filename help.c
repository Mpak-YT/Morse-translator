//*      22
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
//#include <malloc.h>
#include <stdbool.h>
#include <locale.h>
/*#include <pthread.h>*/

#define HZ_MIN 37
#define HZ_MAX 32767
#define HZ_GRAD 1091
#define DURATION_MS_MIN 0
#define DURATION_MS_MAX 3000
#define DURATION_MS_GRAD 100
//***********************
#define SHIFT 70
#define SPEED_NORMAL 1
#define SPEED_MULTI_FAST 1.15
#define SPEED_MULTI_SLOW 0.25
#define SPEED_MAX 10*SHIFT
#define SPEED_MIN 1
//****************************
#define NUMBER_OF_RU 32
#define NUMBER_OF_ENG 26
#define NUMBER_OF_SYMBOLS 24
#define MORSE_ERROR '#'
#define MAX_MORSE_CODE 256
#define MAX_MORSE_SYMBOLS 8
#define MORSE_SYMBOLS ".- \t"
#define SYMBOLS "0123456789.,:;|\'\"-/?!@\t+"
#define RU  "????????????????????????????????" SYMBOLS
#define ENG "ABCDEFGHIJKLMNOPQRSTUVWXYZ" SYMBOLS
//*******************PSEUDOGRAPHICS***********************
#define VERT_1 179
#define VERT_2 186
#define VERT_2_LEFT_1 182
#define VERT_2_RIGHT_1 199
#define HORIZ_1 196
#define HORIZ_2 205
#define HORIZ_2_UP_1 207
#define HORIZ_2_DOWN_1 209
#define CROSS_1 197
#define UP_RIGHT_2 201
#define UP_LEFT_2 187
#define DOWN_RIGHT_2 200
#define DOWN_LEFT_2 188
#define BLOCK 219
#define SQUARE 254
#define LENGHT_PART_LINE 18
#define LENGHT_LINE 21
//*******************KEY_CODE*****************************
#define KEY_ENTER 13
#define KEY_BACKSPACE 8
#define KEY_PG_UP 72
#define KEY_PG_DOWN 80
#define KEY_PG_RIGHT 77
#define KEY_PG_LEFT 75
//*******************DURATION*****************************
#define DIT_IN_DAH 3
#define DIT_IN_BETWEEN_D 1
#define DIT_IN_BETWEEN_LETTER 3
#define DIT_IN_BETWEEN_WORD 7

struct morse{
    struct code{
        unsigned int morse_code:8;
    }*codes;
    char *symbols;  
};

struct settings{
    short int hz;         //* 37 <= HZ <= 32767
    short int dit_duration; //* 0 <=    <= 3000ms
    bool language;          //* 1- eng, 0 -ru
};

bool secure_input_symbols(char symbol, char *symbols);
void caps_lock(bool new_state);
//************************************PSEUDOGRAPHICS************************************
void line(int pos, int max);
//**************************************PRINT_MENU**************************************
int print_menu(bool language, char* (*txt)(int, bool), char* (*hint)(int, bool), int max_number);
//*****************************************TEXT*****************************************
char* text_menu(int number, bool language);
char* text_settings_menu(int number, bool language);
char* text_language_menu(int number,  bool language);
char* text_translator_menu(int number, bool language);
char* get_settings_hint(int choise, bool language);
void hello(bool language);


//**********************INTERACTION***********************
int interaction_value(short int *value, int min, int max, int shift);
int interaction_menu(int *option, int max);
void change_speed(int key, int early_key, float *speed);
//************************IN_MENU*************************
void print_all_morse(struct morse morse, struct settings settings);
//**********************SETTINGS**************************
void language_selection(struct morse *morse, bool *language);
void input_value(short int *hz, short int min, short int max, short int grad);
//************************MENU****************************
void settings_menu(struct morse *morse, struct settings *settings);
void menu(struct morse *morse, struct settings *settings);
void translator_menu(struct morse morse, struct settings settings);
//*********************FOR_TRANSLATOR*********************
char* input_str(char *symbols);
void handle_caps_lock_beep(bool caps_state, int hz, int duration);
void encoder(struct morse morse, char* str, short int dit_duration, short int hz);
void decoder(struct morse morse, char* str);
int ep(int num, int degree);
void print_morse(unsigned char byte, short int dit_duration, short int hz);







void main(){
    setlocale(LC_ALL, "ru_RU.CP866"); // Set locale to Russian CP866
    SetConsoleCP(866); // Set console input code page to 866
    SetConsoleOutputCP(866); // Set console output code page to 866
    struct morse morse;
    struct settings settings;
    settings.dit_duration=300;
    settings.hz = 800;
    language_selection(&morse, &settings.language);
    hello(settings.language);
    menu(&morse, &settings);
    free(morse.symbols);
    free(morse.codes);
}





//********************************************************
//**********************INTERACTION***********************
//********************************************************
int interaction_value(short int *value, int min, int max, int shift) {
    int key = getch();
    if (key == KEY_ENTER)
        return 0;
    if (key == 0 || (key > 223 && key < 256)) {
        key = getch();
        switch (key) {
            case KEY_PG_LEFT:
                *value = (*value - shift < min) ? min : *value - shift;
                return key;
            case KEY_PG_RIGHT:
                *value = (*value + shift > max) ? max : *value + shift;
                return key;
        }      
    }
    return 1;
}

int interaction_menu(int *option, int max) {
    int key = getch();
    if (key == KEY_ENTER)
        return 0;
    if (key == 0 || (key > 223 && key < 256)) {
        key = getch();
        switch (key) {
            case KEY_PG_UP:
                *option = (*option - 1 < 0) ? max : *option - 1;
                break;
            case KEY_PG_DOWN:
                *option = (*option + 1 > max) ? 0 : *option + 1;
                break;
        }
    }
    return 1;
}





//********************************************************
//************************IN_MENU*************************
//********************************************************
void print_all_morse(struct morse morse, struct settings settings){
    system("cls");
    printf("\n");
    for(int i=0; morse.symbols[i]!='\0'; i++){
        printf("%c  <->  ", morse.symbols[i]);
        print_morse(morse.codes[i].morse_code, 0, 0);
        printf("\t\t");
        if((i+1)%3==0)
            printf("\n");
    }
    getch();
}




void caps_lock(bool new_state) {
    if ((GetKeyState(VK_CAPITAL) & 0x0001) != new_state) {
        keybd_event(VK_CAPITAL, 0, KEYEVENTF_EXTENDEDKEY, 0);
        keybd_event(VK_CAPITAL, 0, KEYEVENTF_KEYUP, 0);
    }
}
//********************************************************
//************************FOR_TRANSLATOR******************
//********************************************************
char* input_str(char *symbols) {
    system("cls");
    char* str = (char*)calloc(1, sizeof(char));
    int i = 0;
    while (1) {
        caps_lock(1);
        char symbol = getch();
        if (symbol == KEY_ENTER) {
            return (i == 0) ? NULL : str;
        }
        if (symbol == KEY_BACKSPACE && i > 0) {
            str[--i] = '\0';
            str = (char*)realloc(str, (i + 1) * sizeof(char));
            system("cls");
            printf("%s", str);
            continue;
        }
        if (symbol == 0) {
            getch();
            continue;
        }
        if (secure_input_symbols(symbol, symbols) == 0) {
            str = (char*)realloc(str, (i + 2) * sizeof(char));
            str[i++] = symbol;
            str[i] = '\0';
            system("cls");
            printf("%s", str);
        }
    }
}

void encoder(struct morse morse, char* str, short int dit_duration, short int hz) {
    printf("\n\n");
    caps_lock(0);
    if (str != NULL) {
        for (int i = 0; str[i] != '\0'; i++) {
            if (str[i] == ' ') {
                printf("     ");
                Sleep(DIT_IN_BETWEEN_WORD * dit_duration);
                continue;
            }
            int j = 0;
            while (str[i] != morse.symbols[j])
                j++;
            print_morse(morse.codes[j].morse_code, dit_duration, hz);
            if (str[i + 1] != ' ' && str[i + 1] != '\t')
                Sleep(DIT_IN_BETWEEN_LETTER * dit_duration);
        }
        free(str);
    }
}

void decoder(struct morse morse, char* str) {
    printf("\n\n");
    if (str != NULL) {
        for (int i = 0; str[i] != '\0'; i++) {
            if (str[i] == ' ')
                continue;
            if (str[i] == '\t') {
                printf(" ");
                continue; 
            }
            int j = 0;
            short int code = 0;
            short int max = MAX_MORSE_CODE;
            while (str[i] != ' ' && str[i] != '\t' && str[i] != '\0') {           
                if (str[i] == '.') {
                    j++;
                    i++;
                    max /= 2;
                } 
                else {
                    code += max;
                    j++;
                    i++;
                    max /= 2;
                }
            }
            if (j >= MAX_MORSE_SYMBOLS) {
                printf("\r");
                continue;
            }
            i--;
            int flag = j;
            while (j <= MAX_MORSE_SYMBOLS) {
                code /= 2;
                j++;
            }
            code += ep(2, flag);
            int k = 0;
            while (morse.symbols[k-1]!='\t') {
                if (code == morse.codes[k].morse_code) {
                    printf("%c", morse.symbols[k]);
                    break;
                }
                k++;
            }
        }
        free(str);
    }
}

int ep(int num, int degree){
    long double z = num;
    if (degree == 0) return 1;
    for (; degree > 1; z *= num, degree--);
    return z;
}

bool secure_input_symbols(char symbol, char *symbols){
    for (int i = 0; *(symbols+i)!='\0';i++)
        if(symbol==*(symbols+i)||symbol==' ')
            return 0;
    return 1;
}

void handle_caps_lock_beep(bool caps_state, int hz, int duration) {
    caps_lock(caps_state);
    Beep(hz, duration);
    Sleep(duration);
    caps_lock(!caps_state);
}

void print_morse(unsigned char byte, short int dit_duration, short int hz){
    int k=0;
    do k++; while (byte>>(MAX_MORSE_SYMBOLS-k) == 0);
    k++;
    for(;k<=MAX_MORSE_SYMBOLS; k++){
        if(byte>>(MAX_MORSE_SYMBOLS-k) & 0b1){
            printf("-");
            handle_caps_lock_beep(1, hz, DIT_IN_DAH*dit_duration);         
        }     
        else{
            printf(".");
            handle_caps_lock_beep(1, hz, dit_duration);        
        }
        Sleep(DIT_IN_BETWEEN_D*dit_duration);
    }
    printf(" ");
}




//********************************************************
//**********************SETTINGS**************************
//********************************************************
void language_selection(struct morse *morse, bool *language){
    morse->symbols = NULL;
    morse->codes = NULL;
    int choice = print_menu(*language, text_language_menu, NULL, 1);
    if(choice==0){
        morse->symbols = (char*)realloc(morse->symbols, sizeof(RU));
        morse->symbols = RU;
        morse->codes = (struct code *)realloc(morse->codes, (NUMBER_OF_RU+NUMBER_OF_SYMBOLS)*sizeof(struct code));
        {    //*first 1 is not readed, start reading after first 1
        (morse->codes)[0].morse_code = 0b00000101, /*  ?  */
        (morse->codes)[1].morse_code = 0b00011000, /*  ?  */
        (morse->codes)[2].morse_code = 0b00001011, /*  ?  */
        (morse->codes)[3].morse_code = 0b00001110, /*  ?  */
        (morse->codes)[4].morse_code = 0b00001100, /*  ?  */
        (morse->codes)[5].morse_code = 0b00000010, /*  ?  */
        (morse->codes)[6].morse_code = 0b00010001, /*  ?  */
        (morse->codes)[7].morse_code = 0b00011100, /*  ?  */
        (morse->codes)[8].morse_code = 0b00000100, /*  ?  */
        (morse->codes)[9].morse_code = 0b00010111, /*  ?  */
        (morse->codes)[10].morse_code = 0b00001101, /*  ?  */
        (morse->codes)[11].morse_code = 0b00010100, /*  ?  */
        (morse->codes)[12].morse_code = 0b00000111, /*  ?  */
        (morse->codes)[13].morse_code = 0b00000110, /*  ?  */
        (morse->codes)[14].morse_code = 0b00001111, /*  ?  */
        (morse->codes)[15].morse_code = 0b00010110, /*  ?  */
        (morse->codes)[16].morse_code = 0b00001010, /*  ?  */
        (morse->codes)[17].morse_code = 0b00001000, /*  ?  */
        (morse->codes)[18].morse_code = 0b00000011, /*  ?  */
        (morse->codes)[19].morse_code = 0b00001001, /*  ?  */
        (morse->codes)[20].morse_code = 0b00010010, /*  ?  */
        (morse->codes)[21].morse_code = 0b00010000, /*  ?  */
        (morse->codes)[22].morse_code = 0b00011010, /*  ?  */
        (morse->codes)[23].morse_code = 0b00011110, /*  ?  */
        (morse->codes)[24].morse_code = 0b00011111, /*  ?  */
        (morse->codes)[25].morse_code = 0b00011101, /*  ?  */
        (morse->codes)[26].morse_code = 0b01011010, /*  ?  */
        (morse->codes)[27].morse_code = 0b00011011, /*  ?  */
        (morse->codes)[28].morse_code = 0b00011001, /*  ?  */
        (morse->codes)[29].morse_code = 0b00100100, /*  ?  */
        (morse->codes)[30].morse_code = 0b00010011, /*  ?  */
        (morse->codes)[31].morse_code = 0b00010101, /*  ?  */
        (morse->codes)[32].morse_code = 0b00111111, /*  '0'   */
        (morse->codes)[33].morse_code = 0b00101111, /*  '1'   */
        (morse->codes)[34].morse_code = 0b00100111, /*  '2'   */
        (morse->codes)[35].morse_code = 0b00100011, /*  '3'   */
        (morse->codes)[36].morse_code = 0b00100001, /*  '4'   */
        (morse->codes)[37].morse_code = 0b00100000, /*  '5'   */
        (morse->codes)[38].morse_code = 0b00110000, /*  '6'   */
        (morse->codes)[39].morse_code = 0b00111000, /*  '7'   */
        (morse->codes)[40].morse_code = 0b00111100, /*  '8'   */
        (morse->codes)[41].morse_code = 0b00111110, /*  '9'   */
        (morse->codes)[42].morse_code = 0b01111111, /*  '.'   */
        (morse->codes)[43].morse_code = 0b01010101, /*  ','   */
        (morse->codes)[44].morse_code = 0b01111000, /*  ':'   */
        (morse->codes)[45].morse_code = 0b00110101, /*  ';'   */
        (morse->codes)[46].morse_code = 0b01101101, /*  '|'   */
        (morse->codes)[47].morse_code = 0b01011110, /*  '\''  */
        (morse->codes)[48].morse_code = 0b01010010, /*  '"'   */
        (morse->codes)[49].morse_code = 0b01100001, /*  '-'   */
        (morse->codes)[50].morse_code = 0b00110010, /*  '/'   */
        (morse->codes)[51].morse_code = 0b01001100, /*  '?'   */
        (morse->codes)[52].morse_code = 0b01110011, /*  '!'   */
        (morse->codes)[53].morse_code = 0b01011010, /*  '@'   */
        (morse->codes)[54].morse_code = 0b00110001, /*  '\t'  */
        (morse->codes)[55].morse_code = 0b00101010; /*  '+'  */
        }
        *language=0;
    }
    else{
        morse->symbols = (char*)realloc(morse->symbols, sizeof(ENG));
        morse->symbols = ENG;
        morse->codes = (struct code *)realloc(morse->codes, (NUMBER_OF_ENG+NUMBER_OF_SYMBOLS)*sizeof(struct code));
        {    //*first 1 is not readed, start reading after first 1
        (morse->codes)[0].morse_code = 0b00000101, /*  A  */
        (morse->codes)[1].morse_code = 0b00011000, /*  B  */
        (morse->codes)[2].morse_code = 0b00011010, /*  C  */
        (morse->codes)[3].morse_code = 0b00001100, /*  D  */
        (morse->codes)[4].morse_code = 0b00000010, /*  E  */
        (morse->codes)[5].morse_code = 0b00010010, /*  F  */
        (morse->codes)[6].morse_code = 0b00001110, /*  G  */
        (morse->codes)[7].morse_code = 0b00010000, /*  H  */
        (morse->codes)[8].morse_code = 0b00000100, /*  I  */
        (morse->codes)[9].morse_code = 0b00010111, /*  J  */
        (morse->codes)[10].morse_code = 0b00001101, /*  K  */
        (morse->codes)[11].morse_code = 0b00010100, /*  L  */
        (morse->codes)[12].morse_code = 0b00000111, /*  M  */
        (morse->codes)[13].morse_code = 0b00000110, /*  N  */
        (morse->codes)[14].morse_code = 0b00001111, /*  O  */
        (morse->codes)[15].morse_code = 0b00010110, /*  P  */
        (morse->codes)[16].morse_code = 0b00011101, /*  Q  */
        (morse->codes)[17].morse_code = 0b00001010, /*  R  */
        (morse->codes)[18].morse_code = 0b00001000, /*  S  */
        (morse->codes)[19].morse_code = 0b00000011, /*  T  */
        (morse->codes)[20].morse_code = 0b00001001, /*  U  */
        (morse->codes)[21].morse_code = 0b00010001, /*  V  */
        (morse->codes)[22].morse_code = 0b00001011, /*  W  */
        (morse->codes)[23].morse_code = 0b00011001, /*  X  */
        (morse->codes)[24].morse_code = 0b00011011, /*  Y  */
        (morse->codes)[25].morse_code = 0b00011100, /*  Z  */
        (morse->codes)[26].morse_code = 0b00111111, /*  '0'   */
        (morse->codes)[27].morse_code = 0b00101111, /*  '1'   */
        (morse->codes)[28].morse_code = 0b00100111, /*  '2'   */
        (morse->codes)[29].morse_code = 0b00100011, /*  '3'   */
        (morse->codes)[30].morse_code = 0b00100001, /*  '4'   */
        (morse->codes)[31].morse_code = 0b00100000, /*  '5'   */
        (morse->codes)[32].morse_code = 0b00110000, /*  '6'   */
        (morse->codes)[33].morse_code = 0b00111000, /*  '7'   */
        (morse->codes)[34].morse_code = 0b00111100, /*  '8'   */
        (morse->codes)[35].morse_code = 0b00111110, /*  '9'   */
        (morse->codes)[36].morse_code = 0b01111111, /*  '.'   */
        (morse->codes)[37].morse_code = 0b01010101, /*  ','   */
        (morse->codes)[38].morse_code = 0b01111000, /*  ':'   */
        (morse->codes)[39].morse_code = 0b00110101, /*  ';'   */
        (morse->codes)[40].morse_code = 0b01101101, /*  '|'   */
        (morse->codes)[41].morse_code = 0b01011110, /*  ' ' '  */
        (morse->codes)[42].morse_code = 0b01010010, /*  '"'   */
        (morse->codes)[43].morse_code = 0b01100001, /*  '-'   */
        (morse->codes)[44].morse_code = 0b00110010, /*  '/'   */
        (morse->codes)[45].morse_code = 0b01001100, /*  '?'   */
        (morse->codes)[46].morse_code = 0b01110011, /*  '!'   */
        (morse->codes)[47].morse_code = 0b01011010, /*  '@'   */
        (morse->codes)[48].morse_code = 0b00110001; /*  '\t'  */
        (morse->codes)[49].morse_code = 0b00101010; /*  '+'  */
        }
        *language=1;
    }
}    

void input_value(short int *value, short int min, short int max, short int grad){
    float speed=SPEED_NORMAL*SHIFT;
    int early_key = 0;
    while(1){
        system("cls");
        printf("%d", min);
        for(int i=0; i<max/grad-5;i++, printf(" "));
        printf("%d\n", max);
        for(int i=0; i<max/grad+1; i++){
            if(*value/grad==i){
                printf("%c", BLOCK);
                continue;
            }
            if(i<*value/grad)
                printf("\033[32m%c\033[0m", SQUARE);
            else
                printf("%c", HORIZ_1);
        } 
        printf("\n\t    %d", *value); 
        int key = interaction_value(value, min, max, speed);
        if(key==0)
            break;      
        change_speed(key, early_key, &speed);
        early_key = key;
    }
}

void change_speed(int key, int early_key, float *speed){
    if((early_key == key && (key==KEY_PG_RIGHT)||(key==KEY_PG_LEFT))|| early_key==0)
        *speed*=SPEED_MULTI_FAST;
    else
        *speed*=SPEED_MULTI_SLOW;
    if(*speed<SPEED_MIN)
        *speed=SPEED_MIN; 
    if(*speed>SPEED_MAX)
        *speed=SPEED_MAX; 
}



//********************************************************
//************************MENU****************************
//********************************************************
void settings_menu(struct morse *morse, struct settings *settings){
    while(1){
        switch (print_menu(settings->language, text_settings_menu, get_settings_hint, 3)){
            case 0:
                input_value(&(settings->hz), HZ_MIN, HZ_MAX, HZ_GRAD);
                Beep(settings->hz, settings->dit_duration);
                break;
            case 1:
                input_value(&(settings->dit_duration), DURATION_MS_MIN, DURATION_MS_MAX, DURATION_MS_GRAD);
                Beep(settings->hz, settings->dit_duration);
                break;
            case 2:                
                language_selection(morse, &(settings->language));
                break;
            case 3:
                return;
        }
    }
}

void menu(struct morse *morse, struct settings *settings){
    while(1){
        switch (print_menu(settings->language, text_menu, NULL, 3)){
            case 0:
                translator_menu(*morse, *settings);
                break;
            case 1:
                print_all_morse(*morse, *settings);
                break;
            case 2:
                settings_menu(morse, settings);
                break;
            case 3:
                return;
        }
    }
}

void translator_menu(struct morse morse, struct settings settings){
    while(1){
        switch (print_menu(settings.language, text_translator_menu, NULL, 2)){
            case 0:
                encoder(morse, input_str(morse.symbols), settings.dit_duration, settings.hz);
                getch();
                break;
            case 1:
                decoder(morse, input_str(MORSE_SYMBOLS));
                getch();
                break;
            case 2:
                return;
        }
    }
}




//************************************PSEUDOGRAPHICS************************************
//**************************************PRINT_MENU**************************************
//*****************************************TEXT*****************************************
void line(int pos, int max){ 
    printf("%c", (pos==0)?UP_RIGHT_2:(pos==max)? DOWN_RIGHT_2:VERT_2_RIGHT_1);
    for(int i=0; i<LENGHT_LINE;i++){
        if(i==LENGHT_PART_LINE){
            printf("%c", (pos==0)?HORIZ_2_DOWN_1:(pos==max)?HORIZ_2_UP_1:CROSS_1);
            continue;
        }     
        printf("%c", (pos==0||pos==max)?HORIZ_2:HORIZ_1);
    }
    printf("%c%c", (pos==0)?UP_LEFT_2:(pos==max)? DOWN_LEFT_2:VERT_2_LEFT_1, (pos==0)?'\n':(pos==max)? '\0':'\n');
}
/**************************************************************************************/
int print_menu(bool language, char* (*txt)(int, bool), char* (*hint)(int, bool), int max_number){
    int choise = 0;
    while (1){
        system("cls");
        for(int number = 0; number<max_number+1; number++){
            line(number, max_number+1);
            printf("%c", VERT_2);
            printf((choise == number)?"\033[42;1m %s \033[0m%c\033[32;1m<-\033[0m":" %s %c  ", txt(number, language), VERT_1);
            printf("%c\n", VERT_2);
        }
        line(max_number, max_number);
        if(hint!=NULL && hint(choise, language)!=NULL)
            printf("%s", hint(choise, language));
        if(!interaction_menu(&choise, max_number))
            return choise;
    }
}
/**************************************************************************************/
char* text_menu(int number, bool language){
    switch(number){
        case 0: return(language) ? "TRANSLATOR      " : 
                                   "??????????      ";
        case 1: return(language) ? "ALL MORSE CODE  " :
                                   "??? ?????? ?????";
        case 2: return(language) ? "SETTINGS        " : 
                                   "?????????       ";
        case 3: return(language) ? "EXIT            " : 
                                   "?????           ";
    }
}

char* text_settings_menu(int number, bool language){
    switch(number){
        case 0: return(language) ? "HZ              " : 
                                   "??              ";
        case 1: return(language) ? "DURATION        " : 
                                   "????????????    ";
        case 2: return(language) ? "LANGUAGE        " : 
                                   "????            ";
        case 3: return(language) ? "EXIT            " : 
                                   "?????           ";
    }
}

char* text_language_menu(int number,  bool language){
    switch(number){
        case 0: return "??????? ????    "; 
        case 1: return "ENGLISH LANGUAGE";
    }
}

char* text_translator_menu(int number, bool language){
    switch(number){
        case 0: return(language) ? "INTO MORSE      " : 
                                   "?? ?????        ";
        case 1: return(language) ? "FROM MORSE      " :
                                   "? ?????         ";
        case 2: return(language) ? "EXIT            " : 
                                   "?????           ";
    }
}

char* get_settings_hint(int choise, bool language) {
    switch (choise) {
        case 0:
            return language ? "\nThe Morse code translator will also produce sound for the auditory transmission of Morse code.\nYou can adjust the frequency of this sound in hertz." : "\n?????? ?? ??? ? ?? ??? ????????? ??? ??? ?????? ????? ???? ???.\n?? ????? ??????? ????? ??? ??? ? ?????.";
        case 1:
            return language ? "\nThe Morse code translator will also produce sound for the auditory transmission of Morse code.\nYou can adjust the duration of the shortest signal (dit) in milliseconds." : "\n?????? ?? ??? ??? ??? ????????? ??? ??? ?????? ????? ???? ???.\n?? ????? ??????? ???????? ???? ?????? ????? (??) ? ?????????.";
        default:
            return NULL;
    }
}

void hello(bool language){
    system("cls");
    printf("%s", (language)?"\n\nHello! This programm can encrypt typed text in morse code and vice versa.\n\
Before starting, it is recommended to go to settings.\nAll menu control using arrow keys and Enter button.\nPress any key to continue":
"\n\n????! ?? ????? ????? ?????? ???????? ??? ?? ????? ??? ? ???????.\n\
??? ????? ?????????? ?????? ???????.\n?? ??????? ???? ? ??????? ?????? ? ??????? ? ?????? Enter\n?????? ???? ???????, ??? ????????");
    getch();
}
