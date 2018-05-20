#include <stdio.h>
#include "/home/pi/Desktop/MYLibrary/LibE101/E101.h"
#include <time.h>
#include <sys/time.h>

bool seenRed = false;
bool seenWhiteStrip = false;
double time1;
double time2 = 0;
int speed1 = 75; //right motor
int speed2 = 175; //left motor
int quadNum = 3;
double tempError = 100;
bool beginning3 = false;
int checkLeftorRightTurn = 0;

/* Open first gate and move on to curved line */
void doQuad1(){
	char serverName[15] = "130.195.6.196";
	int port = 1024;
	char message[24] = "Please";

	int connection = connect_to_server(serverName, port);

	int sent;
	if (connection == 0){
		sent = send_to_server(message);
	}

	if (sent == 0){
		receive_from_server(message);
	}

	int sent2 = send_to_server(message);
	if (sent2 == 0){
		printf("Yay, I opened the gate!");
	}
	
	return;
}

/* Error calculations for quadrants 2 & 3 */
double getError(){
	int pixel;
	int row = 120;
	int maxWhite = 0;
	int minWhite = 0;
	int numWhitePixels = 0;
	int redPixel = 0;
	int bluePixel = 0;
	int greenPixel = 0;
	double whiteThreshold = 0;
	double error = 0;

	if (quadNum == 3){
		/* Check whether red line is reached */
		for (int i = 0; i < 320; i++){
			redPixel = get_pixel(row, i, 0);
			greenPixel = get_pixel(row, i, 1);
			bluePixel = get_pixel(row, i, 2);
			if (redPixel > 200 && greenPixel < 50 && bluePixel < 50){
				seenRed = true;
				return 2000000;
			}
		}
	}

	/* Get max and min white values */
	for(int i = 0; i < 320; i++){
		pixel = get_pixel(row, i, 3);
		if (i == 0){
			maxWhite = pixel;
			minWhite = pixel;
		}
		else{
			if (pixel > maxWhite){
				maxWhite = pixel;
			}
			else if (pixel < minWhite){
				minWhite = pixel;
			}
		}
	}

	/* Get threshold above which pixels are considered white */
	whiteThreshold = (maxWhite + minWhite) / 1.5;

	/* Check white and black for pixels & calculate error */
	for(int i = 0; i < 320; i++){
		pixel = get_pixel(row, i, 3);

		if (pixel > whiteThreshold){
			pixel = 1;
			numWhitePixels++;
		}
		else{
			pixel = 0;
		}

		error += (i-160)*pixel;
	}

	if (!beginning3){
		/* Adjust error for width of line & check whether all black/white is seen */
		if (numWhitePixels == 0){
			return -1000000;
		}
		else if (numWhitePixels > 240){
			return 1000000;
		}
		else if (numWhitePixels != 0){
			error /= numWhitePixels;
		}
	}
	else{
		error /= numWhitePixels;
	}

	return error;
}

/* Navigate curved line until white strip reached */
void doQuad2(){
	struct timeval time;
	double error;

	gettimeofday(&time, 0);
	time1 = time.tv_sec+(time.tv_usec/1000000.0);

	/* Take picture approx. every 1/6 of a second & calculate error */
	if (time1-time2 > 0.15){
		take_picture();
		error = getError();

		/* Change motor speed based on error calculation */
		if (error == -1000000){
			set_motor(1, -speed1);
			set_motor(2, -speed2);
		}
		else if (error == 1000000){
			seenWhiteStrip = true; //Move on to quadrant 3
		}
		else{
			int dV;
			double kP = 1.4;

			dV = (int) (((double) error)*kP);

			set_motor(1, speed1-dV);
			set_motor(2, speed2+dV);
		}
	}
}

/* Correct turning */
void correctTurn(){
	tempError = 100;
	while (tempError != 0){
		take_picture();

		tempError = (int) (getError());
		int tempSpeed1;
		int tempSpeed2;
		if (checkLeftorRightTurn == 1){
			tempSpeed1 = -75;
			tempSpeed2 = 75;
		}
		else if (checkLeftorRightTurn == 2){
			tempSpeed1 = 75;
			tempSpeed2 = -75;
		}

		set_motor(1,-tempSpeed1);
		set_motor(2,tempSpeed2);
	}
	checkLeftorRightTurn = 0;
	return;
}

/* Quadrant 3 & 4 - 90 degree left turn */
void turnLeft(){
	printf("\nGo Left!");
	set_motor(1,100);
	set_motor(2,-150);
	sleep1(0,750000);
	set_motor(1,0);
	set_motor(2,0);
	sleep1(0,500000);
	correctTurn();
	return;
}

/* Quadrant 3 & 4 - 90 degree right turn */
void turnRight(){
	printf("\nGo Right!");
	set_motor(1,-100);
	set_motor(2,150);
	sleep1(1,750000);
	set_motor(1,0);
	set_motor(2,0);
	sleep1(0,500000);
	correctTurn();
	return;
}

void doQuad3(){
	struct timeval time;
	double error;

	gettimeofday(&time, 0);
	time1 = time.tv_sec+(time.tv_usec/1000000.0);

	/* Take picture approx. every 1/6 of a second & calculate error */
	if (time1-time2 > 0.15){
		take_picture();
		error = getError();
		printf("\nError = %f", error);

		if (error == 2000000){
			return;
		}
		else if (error == -1000000){
			set_motor(1,-speed1);
			set_motor(2,-speed2);
		}
		else if (error != 1000000){
			if (error > 75){
				set_motor(1,speed1);
				set_motor(2,speed2);
				sleep1(0,250000);
				checkLeftorRightTurn = 1;
				turnRight();
			}
			else if (error < -75){
				set_motor(1,speed1);
				set_motor(2,speed2);
				sleep1(0,250000);
				checkLeftorRightTurn = 2;
				turnLeft();
			}
			else{
				int dV;
				double kP = 1.4;

				dV = (int) (((double) error)*kP);

				set_motor(1, speed1-dV);
				set_motor(2, speed2+dV);
			}
		}
		else{
			set_motor(1,speed1);
			set_motor(2,speed2);
			sleep1(0,500000);
			turnLeft();
		}
	}
}

void doQuad4(){
	//DO NOTHING YET
	printf("\nI Should Do Stuff");
}

int main(){
	init();

	while (true){
		/* Open first gate and move on to curved line */
		while (quadNum == 1){
			doQuad1();
			quadNum = 2;
		}

		/* Navigate curved line until white strip reached */
		while (quadNum == 2){
			doQuad2();

			if (seenWhiteStrip){
				printf("\nDo The Thing");
				quadNum = 3;

				set_motor(1,0);
				set_motor(2,0);
				sleep1(0,500000);

				set_motor(1,speed1);
				set_motor(2,speed2);
				sleep1(0,750000);

				checkLeftorRightTurn = 1;
				beginning3 = true;
				correctTurn();
			}
		}

		/* Navigate straight lines until red strip reached */
		while (quadNum == 3){
			beginning3 = false;
			doQuad3();

			if (seenRed){
				seenRed = false;
				quadNum = 4;
			}
		}

		/* Navigate maze and deal with second gate */
		while (quadNum == 4){
			doQuad4();
		}
	}

	return 0;
}

//NOTE:
	//turn left - 1=100, 2=-150 (1,550000)
	//turn right - 1=-100, 2=150 (2,0)
