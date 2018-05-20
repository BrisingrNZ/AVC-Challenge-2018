#include <stdio.h>
#include "/home/pi/Desktop/MYLibrary/LibE101/E101.h"
#include <time.h>
#include <sys/time.h>

int quadrantNumber = 1;
int motor1Speed = 75; //right
int motor2Speed = 175; //left
bool seenRed = false;
bool seenWhiteStrip = false;
struct timeval time;
double time1;
double time2 = 0;

void doQuad1(){
	init();
	char serverName[15] = "130.195.6.196";
	int port = 1024;
	char message[24] = "Please";

	int connection = connect_to_server(serverName, port);
	int sent;
	if (connection == 0){
		sent = send_to_server(message);
	}
	int received;
	if (sent == 0){
		receive_from_server(message);
	}
	int sent2 = send_to_server(message);
	if (sent2 == 0){
		printf("Yay, I opened the gate!");
	}
	return;
}

/* Return error value for quadrant 2 */
double getQuad2Error(){
	int pixel;
	int row = 120;
	int maxWhite = 0;
	int minWhite = 0;
	int numWhitePixels = 0;
	double whiteThreshold = 0;
	double error = 0;

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

	/* Adjust error for width of line & check whether all black is seen */
	if (numWhitePixels > 240){
		return 1000000;
	}
	else if (numWhitePixels == 0){
		return -1000000;
	}
	else if (numWhitePixels != 0){
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
		error = getQuad2Error();

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
			double kP = 1.3;
			
			dV = (int) (((double) error)*kP);
			
			set_motor(1, speed1-dV);
			set_motor(2, speed2+dV);
		}
	}
}

/* Return error value for quadrant 3 */
double getQuad3Error(){
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

	return error;
}

/* Quadrant 3 & 4 - 90 degree left turn */
void turnLeft(){
	set_motor(1,100);
	set_motor(2,-150);
	sleep1(1,550000);
	set_motor(1,0);
	set_motor(2,0);
	sleep1(0,500000);
	return;
}

/* Quadrant 3 & 4 - 90 degree right turn */
void turnRight(){
	set_motor(1,-100);
	set_motor(2,150);
	sleep1(2,0);
	set_motor(1,0);
	set_motor(2,0);
	sleep1(0,500000);
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
		error = getQuad3Error();

		if (error == 2000000){
			return;
		}
		else if (error == -1000000){
			set_motor(1,-speed1);
			set_motor(2,-speed2);
		}
		else if (error != 1000000){
			if (error > 75){
				turnRight();
			}
			else if (error < -75){
				turnLeft();
			}
			else{
				int dV;
				double kP = 1.3;
				
				dV = (int) (((double) error)*kP);
				
				set_motor(1, speed1-dV);
				set_motor(2, speed2+dV);
			}
		}
		else{
			turnLeft();
		}
	}
}

void doQuad4(){
	redCheckQ4();

	if (!seenRed){

	}
	else{
		//GATE PROTOCOL
		seenRed = false;
	}

	return;
}

void redCheckQ4(){
	/* Take picture every 1/4 of a second & calculate */
	gettimeofday(&time, 0);
	time1 = time.tv_sec+(time.tv_usec/1000000.0);

	if (time1-time2 > 0.25){
		take_picture();

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

		gettimeofday(&time, 0);
		time2 = time.tv_sec+(time.tv_usec/1000000.0);
	}
}

void checkForwardSensor(){
	//if dist < x
		//Check left
			//Turn left
		//Check right
			//Turn right
}

void checkWalls(){
	//if right sensors have values
		//check equal and > x
	//else if right sensors have no values
		//check left sensors for values
			//check equal and > x
}

void gateProtocol(){
	bool seenGate = false;

	while (!seenGate){
		//look forward for gate value < x
			seenGate = true;
	}

	//while gate in front DO NOTHING
	//When gate not in front, move forward by exiting gateProtocol
}

int main(){
	init();

	while (true){
		/* Open first gate and move on to curved line */
		while (quadrantNumber==1){
			doQuad1();
			quadrantNumber = 2;
		}

		/* Navigate curved line until white strip reached */
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

		/* Navigate maze and deal with second gate */
		while (quadrantNumber==4){
			doQuad4();
		}
	}

	return 0;
}

//left - 1=100, 2=-150 (1,550000)
//right - 1=-100, 2=150 (2,0)