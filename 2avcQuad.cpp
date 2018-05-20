#include <stdio.h>
#include "/home/pi/Desktop/MYLibrary/LibE101/E101.h"
#include <time.h>
#include <sys/time.h>

int main(){
	init();

	bool seenRed = false;
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
	struct timeval time;
	double time1;
	double time2 = 0;
	int motor1Speed = 75;
	int motor2Speed = 175;

	while (!seenRed){
		error = 0;
		numWhitePixels = 0;
		
		gettimeofday(&time, 0);
		time1 = time.tv_sec+(time.tv_usec/1000000.0);

		if (time1-time2>0.25){
			take_picture();

			for (int i = 0; i<320; i++){
				redPixel = get_pixel(row, i, 0);
				greenPixel = get_pixel(row, i, 1);
				bluePixel = get_pixel(row, i, 2);
				if (redPixel > 200 && greenPixel < 50 && bluePixel < 50){
					break;
				}
			}

			for(int i =0; i<320; i++){
				pixel = get_pixel(row, i, 3);
				if (i==0){
					maxWhite = pixel;
					minWhite = pixel;
				}
				else{
					if (pixel>maxWhite){
						maxWhite = pixel;
					}
					else if (pixel<minWhite){
						minWhite = pixel;
					}
				}
			}

			whiteThreshold = (maxWhite + minWhite) / 1.5;

			for(int i=0; i<320; i++){
				pixel = get_pixel(row, i, 3);

				if(pixel>whiteThreshold){
					pixel = 1;
					numWhitePixels++;

				}else{
					pixel = 0;
				}

				error += (i-160)*pixel;
			}

			int dV;
			double kP = 1.3;
			if (numWhitePixels!=0){
				error /= numWhitePixels;
				
				dV = (int) (((double) error)*kP);
				
				set_motor(1, motor1Speed-dV);
				set_motor(2, motor2Speed+dV);				
			}
			else{
				set_motor(1,-motor1Speed);
				set_motor(2,-motor2Speed);
			}

			gettimeofday(&time, 0);
			time2 = time.tv_sec+(time.tv_usec/1000000.0);
		}
	}
	return 0;
}

