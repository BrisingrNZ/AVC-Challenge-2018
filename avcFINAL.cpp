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
int quadNum = 1;
double tempError = 100; // For correctTurn()
bool beginning3 = false; // For getError()
int correctLRTurn = 0; // Q3
int frontCentreSensor; // Analog 4
int frontLeftSensor; // Analog 0
int frontRightSensor; // Analog 3
int rearLeftSensor; // Analog 1
int rearRightSensor; // Analog 2

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
			if (redPixel > 150 && bluePixel < 50){
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
		printf("\nNWP = %d", numWhitePixels);
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
		if (correctLRTurn == 1){ // Adjust by turning left
			set_motor(1,100);
			set_motor(2,-100);
		}
		else if (correctLRTurn == 2){ // Adjust by turning right
			set_motor(1,-100);
			set_motor(2,100);
		}
	}

	set_motor(1,0);
	set_motor(2,0);
	sleep1(1,0);
	correctLRTurn = 0;

	return;
}

/* Do a 180 degree turn */
void oneEightyDegree(){
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

/* Check if wall in front for Q4 */
void checkInFront(){
	frontCentreSensor = read_analog(4);

	if (frontCentreSensor > 200 && frontCentreSensor < 600){
		set_motor(1,speed1);
		set_motor(2,speed2);
		sleep1(0,750000);

		frontLeftSensor = read_analog(0);
		frontRightSensor = read_analog(3);
		if (frontLeftSensor > 900){ //turn left
			set_motor(1,175);
			set_motor(2,-200);
			sleep1(1,0);

			set_motor(1,speed1);
			set_motor(2,speed2);
			sleep1(0,500000);

			set_motor(1,0);
			set_motor(2,0);
			sleep1(0,250000);

			rearLeftSensor = read_analog(1);
			frontLeftSensor = read_analog(0);
			while (rearLeftSensor > 900 && frontLeftSensor < 900){
				set_motor(1,-50);
				set_motor(2,150);

				rearLeftSensor = read_analog(1);
				frontLeftSensor = read_analog(0);
			}

			while (rearLeftSensor < 900 && frontLeftSensor > 900){
				set_motor(1,75);
				set_motor(2,-125);

				rearLeftSensor = read_analog(1);
				frontLeftSensor = read_analog(0);
			}
		}
		else if (frontRightSensor > 900){ //turn right
			set_motor(1,-175);
			set_motor(2,255);
			sleep1(1,250000);

			set_motor(1,speed1);
			set_motor(2,speed2);
			sleep1(1,500000);

			set_motor(1,-175);
			set_motor(2,255);
			sleep1(1,250000);

			set_motor(1,speed1+50);
			set_motor(2,speed2);
			sleep1(1,0);

			set_motor(1,0);
			set_motor(2,0);
			sleep1(0,250000);

			rearRightSensor = read_analog(2);
			frontRightSensor = read_analog(3);
			while(rearRightSensor > 900 && frontRightSensor < 900){
				set_motor(1,75);
				set_motor(2,-125);

				rearRightSensor = read_analog(2);
				frontRightSensor = read_analog(3);
			}

			while (rearRightSensor < 900 && frontRightSensor > 900){
				set_motor(1,-50);
				set_motor(2,150);

				rearRightSensor = read_analog(2);
				frontRightSensor = read_analog(3);
			}
		}
	}
}

/* Check sensors for Q4 */
void checkSensors(){
	frontRightSensor = read_analog(3);
	rearRightSensor = read_analog(2);
	frontLeftSensor = read_analog(0);
	rearLeftSensor = read_analog(1);

	if (frontRightSensor < 900 && rearRightSensor < 900 && frontLeftSensor < 900 && rearLeftSensor < 900){ //forward
		set_motor(1,20);
		set_motor(2,255);
	}
	else if (frontLeftSensor > 900){
		set_motor(1,speed1*2); //angle left
	}
	else if (frontRightSensor > 900){ //angle right
		set_motor(1,speed1-30);
		set_motor(2,speed2*2);
	}
}

/* Check for red tape for Q4 */
void gateProtocol(){
	take_picture();
	int row = 120;
	bool firstGateCheck = false;
	bool secondGateCheck = false;
	int redPixel;
	int greenPixel;
	int bluePixel;

	for (int i = 0; i < 320; i++){
		redPixel = get_pixel(row, i, 0);
		greenPixel = get_pixel(row, i, 1);
		bluePixel = get_pixel(row, i, 2);
		if (redPixel > 200 && greenPixel < 50 && bluePixel < 50){
			seenRed = true;
		}
	}

	if (seenRed){
		set_motor(1,0);
		set_motor(2,0);

		while (!firstGateCheck){ //Wait until gate closes in front
			frontCentreSensor = read_analog(0);
			if (frontCentreSensor > 50 && frontCentreSensor < 700){
				firstGateCheck = true;
			}
		}

		while (!secondGateCheck){ //Wait until gate opens again
			frontCentreSensor = read_analog(0);
			if (frontCentreSensor < 50){
				secondGateCheck = true;
			}
		}

		set_motor(1,speed1);
		set_motor(2,speed2);
		sleep1(3, 0);
	}
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
	if (true){
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
	checkInFront();
	checkSensors();
	gateProtocol();
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

				set_motor(1,speed1);
				set_motor(2,speed2);
				sleep1(1,0);
			}
		}

		/* Navigate maze and deal with second gate */
		while (quadNum == 4){
			doQuad4();
		}
	}

	return 0;
}