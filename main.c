
#include <avr/io.h>
#include "lcd.h"
#include "avr.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>

#define MY_NULL 47

enum mode {standard, military};
enum verticalPosition {top, bottom};
enum AmPm {am, pm};
	
struct Time{
	int month;
	int day;
	int year;
	int hours;
	int minutes;
	int seconds;
	enum mode clockMode;// = standard;
	enum AmPm ampm;
};
// converts to actually number
unsigned char get_value(int key){
	switch(key){
		case 1:
			return 1;
		case 2:
			return 2;
		case 3:
			return 3;
		case 5:
			return 4;
		case 6:
			return 5;
		case 7:
			return 6;
		case 9:
			return 7;
		case 10:
			return 8;
		case 11:
			return 9;
		case 14:
			return 0;
		case 4:
			return 65; // capital A
 		case 8:
			return 66; // capital B
		case 16:
			return 68; // capital D
		default:  
			return MY_NULL;
	}
}

int is_pressed(int r, int c){
	
	//##### NC ######
	DDRC= 0; // set everything to input
	PORTC = 0; // set to N/C
	
	//####  write weak 1 to c+4 ####
	SET_BIT(PORTC, c+4);
	
	// ######  write strong 0 to r #######
	SET_BIT(DDRC, r);
	CLR_BIT(PORTC, r); 
	
	
	// check if button is pressed
	if(GET_BIT(PINC, c+4))
		return 0;
	return 1; 
}

unsigned char get_key(){
	int r,c;
	for(r=0; r<4;++r){
		for(c=0; c<4;++c){
			if(is_pressed(r,c)){
				return get_value((r*4)+c+1);
			}
		}
	}
	return get_value(0);
}
		
void promptDelay(char* message){
	clr_lcd();
	puts_lcd2(message);
	wait_avr(1500);
}

char waitForKeyPress(){
	char c = MY_NULL;
	while(c == MY_NULL){
		c = get_key();
		wait_avr(1);
	}
	return c;
}

enum mode setMode(){
	char c = MY_NULL;
	promptDelay("Enter Mode");
	puts_lcd2("A: Standard time");
	pos_lcd(1,0);
	puts_lcd2("B: Military time");
	
	while(c != 'A' && c != 'B'){
		c = get_key();
		wait_avr(1);
	}
	
	if (c == 'B') return military;
	else return standard;
}


void printTime(struct Time t){
	char buf[17];
	char * tempAMPM = NULL;
	sprintf(buf,"%02d/%02d/%04d",t.month, t.day, t.year);
	clr_lcd();
	pos_lcd(0,3);
	puts_lcd2(buf);
	
	if(t.clockMode == standard){
		pos_lcd(1,3);
		tempAMPM = (t.ampm == am) ? "AM" : "PM";
		sprintf(buf,"%02d:%02d:%02d %s", t.hours, t.minutes, t.seconds, tempAMPM);
		puts_lcd2(buf);
	}else if(t.clockMode == military){
		pos_lcd(1,4);
		sprintf(buf,"%02d:%02d:%02d", t.hours, t.minutes, t.seconds);
		puts_lcd2(buf);
	}
}
	
		
int digitPrompt(int place, int row, unsigned char min, unsigned char max, struct Time t, enum verticalPosition vp){
	char buf[17];
	unsigned char c = MY_NULL;
	char * tempAMPM = NULL; 
	clr_lcd();
	if(vp == top){
		pos_lcd(0,3);
		sprintf(buf,"%02d/%02d/%04d",t.month, t.day, t.year);
		puts_lcd2(buf);
		pos_lcd(1, place+3);
		puts_lcd2("^");
	}else if(vp == bottom){
		if(t.clockMode == standard){
			pos_lcd(0, place+2);
			puts_lcd2("v");
			pos_lcd(1,2);
			tempAMPM = (t.ampm == am) ? "AM" : "PM";
			sprintf(buf,"%02d:%02d:%02d %s", t.hours, t.minutes, t.seconds, tempAMPM);
			puts_lcd2(buf);
		}else if(t.clockMode == military){
			pos_lcd(0, place+4);
			puts_lcd2("v");
			pos_lcd(1,4);
			sprintf(buf,"%02d:%02d:%02d", t.hours, t.minutes, t.seconds);
			puts_lcd2(buf);
		}
	}
		
	
	while(1){
		c  = get_key();
		if(c>= min && c<= max)
			return c; 
		wait_avr(1);
	}
}
		

void my_wait(){
	wait_avr(400);
}

unsigned char isLeapYear(int year){
	if(year%4 != 0) return 0;
	else if(year % 100 != 0) return 1;
	else if(year % 400 != 0) return 0; 
	else return 1;
}

int getMonthRange(struct Time t){
	switch(t.month){
		case(1):
		case(3):
		case(5):
		case(7):
		case(8):
		case(10):
		case(12): 
			return 31;
		case(4):
		case(6):
		case(9):
		case(11):
			return 30; 
		case(2):
			if(isLeapYear(t.year)) return 29;
			else return 28;
	}
	return 0; 
}

enum AmPm getAmPm(){
	int n = MY_NULL;
	clr_lcd();
	pos_lcd(0, 5);
	puts_lcd2("AM/PM?");
	pos_lcd(1, 3); 
	puts_lcd2("A:AM  B:PM");
	
	while(1){
		n = get_key();
		if(n == 65) return am;
		if(n == 66) return pm;
		
		wait_avr(1); 
	}
}

struct Time setTime(enum mode cMode){
	int temp = 0;
	 
	 
	struct Time t;
	t.month = 0; 
	t.day = 0; 
	t.year = 0; 
	t.hours = 0; 
	t.minutes = 0;
	t.seconds = 0;  
	t.ampm = am;
	t.clockMode = cMode;
	
	
	
	promptDelay("Enter MM/DD/YYYY");
	wait_avr(10);
	//int place, int row, unsigned char min, unsigned char max, struct Time t, enum verticalPosition vp
	
	// get year
	t.year = digitPrompt(6, 0, 0, 9, t, top) * 1000;
	my_wait();
	temp = digitPrompt(7, 0, 0, 9, t, top) * 100;
	t.year += temp;
	my_wait();
	temp = digitPrompt(8, 0, 0, 9, t, top) * 10;
	t.year += temp;
	my_wait();
	temp = digitPrompt(9, 0, 0, 9,t , top);
	t.year += temp;
	my_wait();
	
	// get month
	t.month = digitPrompt(0, 0, 0, 1, t, top) * 10;
	my_wait();
	int high; 
	int low; 
	if(t.month == 10){
		high = 2;
		low = 0; 
	}else{
		high = 9; 
		low  = 1; 
	}
	temp = digitPrompt(1, 0, low, high, t,top);
	t.month += temp;
	my_wait();
	
	int monthMax = getMonthRange(t);
	// get day
	int tensDigit = monthMax/10;
	t.day = digitPrompt(3, 0, 0, tensDigit, t, top) * 10;
	my_wait();
	int onesDigit = monthMax - ((monthMax/10)*10);
	
	if(t.day == 10){
		onesDigit = 9;
	}else if(t.day == 20 && t.month != 2){
		onesDigit = 9;
	}
	wait_avr(10);
	temp = digitPrompt(4, 0, 0, onesDigit, t, top);
	t.day += temp;
	my_wait();
	
	if(t.clockMode == standard){
		t.ampm = getAmPm();
		wait_avr(10);
		promptDelay("Enter HH:MM:SS");
		wait_avr(10);
		
		t.hours = digitPrompt(0, 1, 0, 1, t, bottom) * 10;
		my_wait();
		temp = digitPrompt(1, 1, 0, 9, t, bottom);
		t.hours += temp;
		my_wait();
		
	}else{
		promptDelay("Enter HH:MM:SS");
		wait_avr(10);
		t.hours = digitPrompt(0, 1, 0, 2, t, bottom) * 10;
		my_wait();
		int high =0; 
		int low = 0; 
		if(t.hours == 10 || t.hours == 0){
			high = 9;
			low = 0; 
		}else if(t.hours == 20){
			high = 3; 
			low  = 0;
		}
		temp = digitPrompt(1, 1, low, high, t, bottom);
		t.hours += temp;
		my_wait();
		
	}
	
	t.minutes = digitPrompt(3, 1, 0, 6, t, bottom) *10;
	my_wait();
	temp = digitPrompt(4, 1, 0, 9, t, bottom);
	t.minutes += temp;
	my_wait();
	
	
	t.seconds = digitPrompt(6, 1, 0, 6, t, bottom) *10;
	my_wait();
	temp = digitPrompt(7, 1, 0, 9, t, bottom);
	t.seconds += temp;
	my_wait();
		
	return t;
}


int main(void){
	char c = MY_NULL;
	int monthMax = 0; 
	struct Time time;
	time.clockMode = standard; 
	time.month = 0; time.day= 0; time.year = 0; 
	time.hours = 0; time.minutes = 0; time.seconds = 0; 
	time.ampm = am;
	 
	ini_avr();
	ini_lcd();
	
	time.clockMode = setMode(); 
	time = setTime(time.clockMode);
	wait_avr(1000);	
	
	printTime(time);
	
	while(1){
		wait_avr(999); 
		++time.seconds;
		
		if(time.seconds > 59){
			time.seconds = 0; 
			++time.minutes;
		}
		
		
		if(time.minutes > 59){
			time.minutes = 0; 
			++time.hours; 
		}
		
		if(time.clockMode == military){
			if(time.hours > 23){
				time.hours = 0; 
				++time.day;
			}
		}else if(time.clockMode == standard){
			if(time.hours > 12){
				time.hours = 1;
				
			}else if(time.hours > 11 && time.ampm == pm && time.minutes == 0 && time.seconds == 0){
					++time.day;
					time.ampm = am; 
			}else if(time.hours > 11 && time.ampm == am && time.minutes == 0 && time.seconds == 0){
					time.ampm = pm;
			}
			
			}
		
		
		monthMax = getMonthRange(time); 
		if(time.day > monthMax){
			++time.month;
			time.day = 1; 
		}
		
		if(time.month > 12){
			time.month = 1; 
			++time.year;
		}
		
		printTime(time);
	
		c = get_key();
		if(c == 'd'){
			wait_avr(10);
			break;
		}else if(c == 65){
			if(time.ampm == pm){
				time.hours += 12;
				time.clockMode = military;
			}else if(time.ampm == am){
				time.clockMode = military;
			}
			wait_avr(10);
			
		}else if(c == 66){
			if(time.hours > 12){
				time.hours -= 12; 
				time.ampm = pm;
				time.clockMode = standard;
			}else if(time.hours <=12){
				time.ampm = am ;
				time.clockMode = standard; 
			}
			wait_avr(10);
		}
		
		wait_avr(5);
	}
	
	return 0; 
}
