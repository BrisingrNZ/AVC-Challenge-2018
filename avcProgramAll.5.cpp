#include <stdio.h>
#include "/home/pi/Desktop/MYLibrary/LibE101/E101.h"
#include <time.h>
#include <sys/time.h>

bool seenRed = false; // Q3+4
bool seenWhiteStrip = false; // Q2
double time1;
double time2 = 0;
int speed1 = 75; // Right motor
int speed2 = 175; // Left motor
int quadNum = 2;
double tempError = 100; // For correctTurn()
bool beginning3 = false; // For getError()
int correctLRTurn = 0; // Q3

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
		if (numWhitePixels < 50){
			return -1000000; // Lost line for Q2 & turn for Q3
		}
		else if (numWhitePixels > 240 && quadNum == 2){
			seenWhiteStrip = true;
		}
		/*else if (numWhitePixels > 200 && quadNum == 3){ // T-intersection for Q3
			return 1000000;
		}
		else if (numWhitePixels > 120 && quadNum == 3){ // Single turn for Q3
			if (error > 0){
//				printf("\nRight!");
				return 3000000;
			}
			else if (error < 0){
//				printf("\nLeft!");
				return -3000000;
			}
		}*/
		else if (numWhitePixels != 0){
			error /= numWhitePixels;
		}
	}
	else{
		error /= numWhitePixels;
	}
	return error;
}

/* Check to left and right for Q3 */
int checkLeftorRight(){ //return 1 = L; return 2 = R; return 0 = dead end
	int pixel;
	int col = 80;
	int maxWhite = 0;
	int minWhite = 0;
	int numWhitePixels = 0;
	int redPixel = 0;
	int bluePixel = 0;
	int greenPixel = 0;
	double whiteThreshold = 0;
	double error = 0;

	/* Get max and min white values */
	for(int i = 0; i < 240; i++){
		pixel = get_pixel(i, col, 3);
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

	/* Check white and black for pixels */
	for(int i = 0; i < 240; i++){
		pixel = get_pixel(i, col, 3);

		if (pixel > whiteThreshold){
			pixel = 1;
			numWhitePixels++;
		}
		else{
			pixel = 0;
		}
	}

	if (numWhitePixels > 50){
		return 1;
	}
	else{
		numWhitePixels = 0;
		col = 240;
	}

	/* Check white and black for pixels */
	for(int i = 0; i < 240; i++){
		pixel = get_pixel(i, col, 3);

		if (pixel > whiteThreshold){
			pixel = 1;
			numWhitePixels++;
		}
		else{
			pixel = 0;
		}
	}

	if (numWhitePixels > 50){
		return 2;
	}
	else{
		return 0;
	}
}

/* Correct turning */
void correctTurn(){
	tempError = 100;
	while (tempError > 1 || tempError < -1){
		take_picture();

		tempError = getError();
		printf("\nError = %f", tempError);
		if (correctLRTurn == 1){ // Adjust by turning right
			set_motor(1,100);
			set_motor(2,-100);
		}
		else if (correctLRTurn == 2){ // Adjust by turning left
			set_motor(1,-100);
			set_motor(2,100);
		}
	}

	set_motor(1,0);
	set_motor(2,0);
	sleep1(1,0);
	correctLRTurn = 0;
//	printf("Correct turn");

	return;
}

/* Do a 180 degree turn */
void oneEightyDegree(){
//	printf("180 Degrees");
	set_motor(1,150);
	set_motor(2,-255);
	sleep1(2,0);

	set_motor(1,0);
	set_motor(2,0);
	sleep1(0,250000);

	correctLRTurn = 1;
	correctTurn();

	return;
}

/* Quadrant 3 - 90 degree left turn */
void turnLeft(){
//	printf("\nGo Left!");

	set_motor(1,250);
	set_motor(2,-150);
	sleep1(0,700000);

	set_motor(1,0);
	set_motor(2,0);
	sleep1(0,500000);

	set_motor(1,-speed1);
	set_motor(2,-speed2);
	sleep1(0,500000);

	correctLRTurn = 1;
	correctTurn();

	return;
}

/* Quadrant 3 - 90 degree right turn */
void turnRight(){
//	printf("\nGo Right!");

	set_motor(1,-230);
	set_motor(2,170);
	sleep1(0,700000);

	set_motor(1,0);
	set_motor(2,0);
	sleep1(0,500000);

	set_motor(1,-speed1);
	set_motor(2,-speed2);
	sleep1(0,500000);

	correctLRTurn = 2;
	correctTurn();

	return;
}

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
			double kP = 1.5;

			dV = (int) (((double) error)*kP);

			set_motor(1, speed1-dV);
			set_motor(2, speed2+dV);
		}
	}
}

/* Navigate straight line maze until red strip reached */
void doQuad3(){
	struct timeval time;
	double error;

	gettimeofday(&time, 0);
	time1 = time.tv_sec+(time.tv_usec/1000000.0);

	/* Take picture approx. every 1/6 of a second & calculate error */
	if (true){//time1-time2 > 0.15){
		take_picture();
		error = getError();

		if (error == 2000000){ // Red strip reached
			set_motor(1,0);
			set_motor(2,0);
			sleep1(1,0);
			return;
		}
		else if (error == -1000000){ 
			int checkValue = checkLeftorRight();
			set_motor(1,0);
			set_motor(2,0);

			if (checkValue == 0){
				oneEightyDegree();
			}
			else if (checkValue == 1){
				set_motor(1,speed1);
				set_motor(2,speed2);
				sleep1(0,750000);
				turnLeft();
				sleep1(0,500000);
			}
			else if (checkValue == 2){
				set_motor(1,speed1);
				set_motor(2,speed2);
				sleep1(1,450000);
				turnRight();
				sleep1(0,500000);
			}
		}
		else{
			int dV;
			double kP = 1.25;

			dV = (int) (((double) error)*kP);

			set_motor(1, speed1-dV);
			set_motor(2, speed2+dV);
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
				quadNum = 3;

				set_motor(1,0);
				set_motor(2,0);
				sleep1(0,500000);

				set_motor(1,speed1);
				set_motor(2,speed2);
				sleep1(0,750000);

				correctLRTurn = 2;
				beginning3 = true;
				correctTurn();

				speed1 = 75;
				speed2 = 125;
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
