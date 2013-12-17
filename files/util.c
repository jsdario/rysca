#include "util.h"

void bold( char * string ) {

	printf( BOLD "%s" RESET, string );
	return;
}

void underline ( char * string ) {

	printf( UNDERLINE "%s" RESET, string );
	return;
}

void print_notice ( char * string ) {

	printf( BOLDCYAN "%s" RESET, string );
	return;
}

void print_warning ( char * string ) {

	printf( BOLDYELLOW "%s" RESET, string );
	return;	
}

void print_alert ( char * string ) {

	printf( BOLDRED "%s" RESET, string );
	return;
}

void print_success ( char * string ) {

	printf( BOLDGREEN "%s" RESET, string );
	return;
}

int random_number(int min_num, int max_num) {

	
	int result=0,low_num=0,hi_num=0;
	if(min_num<max_num)	{
		low_num=min_num;
        hi_num=max_num+1; // this is done to include max_num in output.
    }else {
        low_num=max_num+1;// this is done to include max_num in output.
        hi_num=min_num;
    }
    srand(time(NULL));
    result = (rand()%(hi_num-low_num))+low_num;
    return result;
}