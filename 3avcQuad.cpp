#include <stdio.h>
#include "/home/pi/Desktop/MYLibrary/LibE101/E101.h"
#include <time.h>
#include <sys/time.h>

bool seenRed = false; // Q3+4
double time1;
double time2 = 0;
int speed1 = 75; // Right motor
int speed2 = 175; // Left motor
int quadNum = 3;
double tempError = 100; // For correctTurn()
bool beginning3 = false; // For getError()
int checkLeftorRightTurn = 0; // Q3

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
		else if (numWhitePixels > 120){
			if (error > 0){
				return 3000000;
			}
			else if (error < 0){
				return -3000000;
			}
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

/* Quadrant 3 - 90 degree left turn */
void turnLeft(){
	printf("\nGo Left!");
	set_motor(1,100);
	set_motor(2,-150);
	sleep1(0,750000);
	set_motor(1,0);
	set_motor(2,0);
	sleep1(0,500000);
	checkLeftorRightTurn = 2;
	correctTurn();
	return;
}

/* Quadrant 3 - 90 degree right turn */
void turnRight(){
	printf("\nGo Right!");
	set_motor(1,-100);
	set_motor(2,150);
	sleep1(1,750000);
	set_motor(1,0);
	set_motor(2,0);
	sleep1(0,500000);
	checkLeftorRightTurn = 1;
	correctTurn();
	return;
}

/* Navigate straight line maze until red strip reached */
void doQuad3(){
	struct timeval time;
	double error;

	gettimeofday(&time, 0);
	time1 = time.tv_sec+(time.tv_usec/1000000.0);

	/* Take picture approx. every 1/6 of a second & calculate error */
	if (time1-time2 > 0.15){
		take_picture();
		error = getError();

		if (error == 2000000){ // Red strip reached
			return;
		}
		else if (error == -1000000){ // Do a 180 degree turn at dead-end
			set_motor(1,-100);
			set_motor(2,150);
			sleep1(5,250000);
			correctTurn();
		}
		else if (error != 1000000){
			if (error == 3000000){
				set_motor(1,speed1);
				set_motor(2,speed2);
				sleep1(0,250000);
				turnRight();
			}
			else if (error == -3000000){
				set_motor(1,speed1);
				set_motor(2,speed2);
				sleep1(0,250000);
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

int main(){
	init();

	while (true){
		beginning3 = false;
		doQuad3();
	}

	return 0;
}