#include <stdio.h>
#include "/home/pi/Desktop/MYLibrary/LibE101/E101.h"
#include <time.h>

int quadrantNumber = 2;
int motor1Speed = 75; //right
int motor2Speed = 175; //left
bool seenRed = false;
bool seenWhiteStrip = false;
struct timeval time;
double time1;
double time2 = 0;

void doQuad2(){
	/* Take picture every 1/4 of a second & calculate */
	gettimeofday(&time, 0);
	time1 = time.tv_sec+(time.tv_usec/1000000.0);

	if (time1-time2 > 0.25){
		take_picture();
		double getError = doLineCalculations();

		if (seenRed){
			printf("ERROR - Seen red in quadrant 2!!");
			break;
		}
		else{
			if (getError == -1000000){
				set_motor(1, -motor1Speed);
				set_motor(2, -motor2Speed);
			}
			else if (getError != 1000000){
				int dV;
				double kP = 1.3;
				error /= numWhitePixels;
				
				dV = (int) (((double) error)*kP);
				
				set_motor(1, motor1Speed-dV);
				set_motor(2, motor2Speed+dV);
			}
			else{
				seenWhiteStrip = true;
			}
		}

		gettimeofday(&time, 0);
		time2 = time.tv_sec+(time.tv_usec/1000000.0);
	}

	return;
}

void doQuad3(){
	/* Take picture every 1/4 of a second & calculate */
	gettimeofday(&time, 0);
	time1 = time.tv_sec+(time.tv_usec/1000000.0);

	if (time1-time2 > 0.25){
		take_picture();
		double getError = doLineCalculations();

		if (seenRed){
			break;
		}
		else{
			if (getError == -1000000){
				set_motor(1, -motor1Speed);
				set_motor(2, -motor2Speed);
			}
			else if (getError != 1000000){
				if (getError > 75){
					turnRight();
				}
				else if (getError < -75){
					turnLeft();
				}
				else{
					int dV;
					double kP = 1.3;
					error /= numWhitePixels;
					
					dV = (int) (((double) error)*kP);
					
					set_motor(1, motor1Speed-dV);
					set_motor(2, motor2Speed+dV);
				}
			}
			else{
				turnLeft();
			}
		}

		gettimeofday(&time, 0);
		time2 = time.tv_sec+(time.tv_usec/1000000.0);
	}

	return;
}

void turnLeft(){
	set_motor(1,100);
	set_motor(2,-150);
	sleep1(1,550000);
	set_motor(1,0);
	set_motor(2,0);
	sleep1(0,500000);
	return;
}

void turnRight(){
	set_motor(1,-100);
	set_motor(2,150);
	sleep1(2,0);
	set_motor(1,0);
	set_motor(2,0);
	sleep1(0,500000);
	return;
}

/* Return error value */
double doLineCalculations(){
	int pixel;
	int row = 120;
	int maxWhite = 0;
	int minWhite = 0;
	double whiteThreshold = 0;
	double error = 0;
	int numWhitePixels = 0;
	int redPixel = 0;
	int bluePixel = 0;
	int greenPixel = 0;

	/* Check whether red line is reached */
	for (int i = 0; i < 320; i++){
		redPixel = get_pixel(row, i, 0);
		greenPixel = get_pixel(row, i, 1);
		bluePixel = get_pixel(row, i, 2);
		if (redPixel > 200 && greenPixel < 50 && bluePixel < 50){
			seenRed = true;
			break;
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

	if (maxWhite < 100){
		return -1000000;
	}
	else if (minWhite > 150){
		return 1000000;
	}
	else {
		whiteThreshold = (maxWhite + minWhite) / 1.5; //Sets threshold to determine white or black pixel

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
			if (numWhitePixels != 0){
				error /= numWhitePixels;
			}
			else{
				return -1000000;
			}
		}

		return error;
	}
}

int main(){
	init();

	while (true){
		while (quadrantNumber==2){
			doQuad2();

			if (seenWhiteStrip){
				quadrantNumber = 3;
			}
		}

		/* Navigate straight lines until red strip reached */
		while (quadrantNumber==3){
			doQuad3();

			if (seenRed){
				quadrantNumber = 4;
				seenRed = false;
			}
		}
	}

	return 0;
}

//left - 1=100, 2=-150 (1,550000)
//right - 1=-100, 2=150 (2,0)