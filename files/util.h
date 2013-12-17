#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Definiciones para la impresion con formato */

#define RESET   	"\033[0m"			/* Reset styles */
#define BOLD 		"\e[1m"				/* Bold */
#define UNDERLINE 	"\e[4m"				/* Underline */
#define BLACK   	"\033[30m"      	/* Black */
#define RED     	"\033[31m"      	/* Red */
#define GREEN   	"\033[32m"      	/* Green */
#define YELLOW  	"\033[33m"      	/* Yellow */
#define BLUE    	"\033[34m"      	/* Blue */
#define MAGENTA 	"\033[35m"      	/* Magenta */
#define CYAN    	"\033[36m"      	/* Cyan */
#define WHITE   	"\033[37m"      	/* White */
#define BOLDBLACK   "\033[1m\033[30m"   /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"   /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"   /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"   /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"   /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"   /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"   /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"   /* Bold White */

/* Imprime un texto en negrita */
void bold( char * string );

/* Imprime un texto subrayado */
void underline ( char * string );

/* Imprime un mensaje rojo en negrita */
void print_alert( char * string );

/* Imprime en verde y negrita */
void print_success ( char * string );

/* Imprime en amarillo y negrita */
void print_warning( char * string );

/* Imprime en cyan y negrita */
void print_notice( char * string );

/* Devuelve un numero random entre min y max */
int random_number(int min_num, int max_num);

#endif