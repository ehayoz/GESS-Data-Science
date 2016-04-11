#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <chrono>
#include <climits>
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>
#include <vector>

// Include CImg library file and use its main namespace
#include "CImg.h"
using namespace cimg_library;

using namespace std;
using namespace std::chrono;


/*
Information:
The Data_all.csv output file contains its data in the following order:
[Timestamp],[HR],[EDA],[TEMP],[Sound],[Dust],[#WiFi],[LON],[LAT],[Greenery]
(Greenery is in the interval [0,1])
The GPS_only.csv output file contains its data in the following order:
[LON],[LAT]

Device				Abbreviation
Wasp City			wc
GPS					gps
Meshlium Scanner	ms
Heartrate			hr
Electrodermal Act.  eda
Temperature			temp
*/

// Measurement Period [s]
float f_hr = 1; // HR only with 1Hz!
float f_eda = 4;
float f_temp = 4;
float f_gps = 1;
float f_wc = 0.4;
float f_ms = 0.016;

ifstream fin[6];
const int N = (sizeof(fin)/sizeof(fin[0]));
ofstream fout[2];
int No = (sizeof(fout)/sizeof(fout[0]));
long Tmin_i[N];
int counter = 0;
long minTime;
long maxTime;

void init();
void check();
void concat();
void cleanup();
string getHR();
string getEDA();
string getTemp();
string getLAT(string);
string getLON(string);
string getSound(string);
string getDust(string);
string readLine(int, int);
double time2stamp1(int,int);
double time2stamp2(int,int);
long getMinTime(bool);
double WGStoCHx(double, double);
double WGStoCHy(double, double);
double DecToSexAngle(double);
double ToSexAngle(double);
double getGreen(double, double);
int stoi(string);
long stol(string);
double stod(string);

CImg<unsigned char> image("img/map.bmp");
CImg<unsigned char> image_chk("img/map_chk.bmp");
int image_h = image.height();
int image_w = image.width();
vector< vector<bool> > greenMat(image_h, vector<bool>(image_w));


int main(int argc, char** argv) {

	init();
	concat();
	//check();

	cleanup();
	return 0;
}

void init() {
  fin[0].open("data/HR.csv");
  fin[1].open("data/GPS.csv");
  fin[2].open("data/WaspCity.csv");
  fin[3].open("data/WifiScan.csv");
  fin[4].open("data/EDA.csv");
  fin[5].open("data/TEMP.csv");
  fout[0].open("data/Data_all.csv");
  fout[1].open("data/GPS_only.csv");
  for(int i = 0; i < No; i++)
	 fout[i].precision(15);
  image.save("img/map_chk.bmp");
}

void check() {
	// Check File Input
	string line;
	int n, n_max;
	cout << "Frequencies based on File Input (if frequencies differ from what they should, the devices did not log data for the same time span):\n\n";
	for(int i = 0; i < N; i++) {
		// Count lines
		for (n = 0; std::getline(fin[i], line); n++)
    		;

		// Check if number of lines match with device frequencies
		switch(i) {
			case 0: n_max = n;
					cout << "HR: " << endl;
					break;
			case 1: cout << "GPS: " << endl;
					break;
			case 2: cout << "WaspCity: " << endl;
					break;
			case 3: cout << "WiFi Scanner: " << endl;
					break;
			case 4: cout << "EDA: " << endl;
					break;
			case 5: cout << "TEMP: " << endl;
					break;
		}
		cout << n << endl;
    	cout << (float) n/n_max*f_hr << endl;
    }
    cout << endl;
}

void cleanup() {
  for(int i = 0; i < N; i++)
  	fin[i].close();
  for(int i = 0; i < No; i++)
  	fout[i].close();
}

void concat() {
	minTime = getMinTime(true);
	maxTime = getMinTime(false);
	if(minTime >= maxTime)
        cout << "Error: Start Time >= End Time. Check your input data and restart program." << endl;
	cout << "Press Enter to start concatenating the data and evaluate greenery";
	cin.get();
	int m1 = 0 + 1; // [FIX, +1]
	int m2 = 0 + 1; // fix empty line
	int m3 = 0 + 1;
	int m3_prev = 1;
	int n_device = 0;
	int progress = 0;
	int progress_prev = 0;

	double CHx;
	double CHy;
	double WGSx;
	double WGSy;

	image.save("img/map_chk.bmp");

	//long t = minTime;

	// Get to "common" time
	// Base is HR (has to be specified in advance)

	for(double t = minTime; t < maxTime; t += 1/f_hr) {
		progress = ceil((double)(t-minTime)/(maxTime - minTime)* 100);
		if(progress != progress_prev) {
			// Progress Bar, only cout if new progress to save time
			system("CLS");
			cout << "Generating Data Files. " << "Progress: " << progress << "%";
			progress_prev = progress;
		}

		// is for [Sound],[Dust],
		while(time2stamp1(2,m2) <= t) {
			m2++;
		}
		// is for [LON],[LAT]
		while(time2stamp1(1,m1) <= t) {
			m1++;
		}
		// is for [WiFi]
		while(time2stamp2(3,m3) <= t) {
			m3++;
		}

		if(m3 != m3_prev)	// only refresh n_device if there is a new measure
		{
			n_device = m3 - m3_prev - 2;	// substract 2 because of our devices
			//cout << "m3=" << m3 << endl << "m3_prev=" << m3_prev << endl << "n_device=" << n_device << endl; cin.get();
		}

		if(n_device < 0)
			n_device = 0;

		// Conversion WGS to CH
		if(getLON(readLine(1,m1)).compare("") == 0)
			CHx = CHy = 0;
		else {
//            if ( ! (istringstream(getLAT(readLine(1,m1))) >> WGSx) ) WGSx = 0;
//            if ( ! (istringstream(getLON(readLine(1,m1))) >> WGSy) ) WGSy = 0;
//			CHx = WGStoCHy(WGSx , WGSy); // Convention: x and y reversed!
//			CHy = WGStoCHx(WGSx , WGSy);
            CHx = WGStoCHy(stod(getLAT(readLine(1,m1))) , stod(getLON(readLine(1,m1)))); // Convention: x and y reversed!
			CHy = WGStoCHx(stod(getLAT(readLine(1,m1))) , stod(getLON(readLine(1,m1))));
		}

		// FOUT [Time],[HR],[EDA],[Temp],						[FIX,-1]									[FIX,-1]
		fout[0] << t << ',' << readLine(0, (t-Tmin_i[0])*f_hr+2-1) << ',' << readLine(4, (t-Tmin_i[4])*f_eda+2-1) << ',' << readLine(5, (t-Tmin_i[5])*f_temp+2) << ','
		// FOUT [Sound],[Dust],
			 << getSound(readLine(2,m2)) << ',' << getDust(readLine(2,m2)) << ','
		// FOUT [WiFi]
			 << n_device << ','
	    // FOUT [LON],[LAT]
			 // << getLON(readLine(1,m1)) << ',' << getLAT(readLine(1,m1)) << endl;
			 << CHx << ',' << CHy << ','
        // FOUT [Green]
			 << getGreen(CHx,CHy) << endl;

	    // FOUT [LON],[LAT]
		fout[1] << CHx << ',' << CHy << endl;

		m3_prev = m3;
	}
	cout  << " ...Success!" << endl << endl;
    CImgDisplay main_disp(image_chk,"Check Image");
	image_chk.save("img/map_chk.bmp");
	while (!main_disp.is_closed())
        main_disp.wait();
}

string readLine(int n_i, int m) {
	// Reset getline pointer
	fin[n_i].clear();
	fin[n_i].seekg(0, ios::beg);
	string line;

	if(m == -1) {
		// get last line
    	while (fin[n_i] >> std::ws && std::getline(fin[n_i], line)) // skip empty lines
        	;
	}
	else {
		for(int i = 0; i <= m; i++) {
			std::getline(fin[n_i], line);
		}
	}

	return line;
}

string getLAT(string line) {
	// LAT: entry after 13th comma
	// LON: entry after 15th comma
	int field_start = 0;
	int field_end = 0;

	// find "starting" comma
	for(int i = 0; i < 13; i++)
		field_start = line.find(',',field_start+1);
	field_start++;

	// find "ending" comma
	for(int i = 0; i < 14; i++)
		field_end = line.find(',',field_end+1);

	line = line.substr(field_start, field_end - field_start);
	if(line.length() > 5) {
		line.erase(4,1);
		line.insert(2,".");
	}

	return line;
}

string getLON(string line) {
	// LAT: entry after 13th comma
	// LON: entry after 15th comma
	int field_start = 0;
	int field_end = 0;

	// find "starting" comma
	for(int i = 0; i < 15; i++)
		field_start = line.find(',',field_start+1);
	field_start++;

	// find "ending" comma
	for(int i = 0; i < 16; i++)
		field_end = line.find(',',field_end+1);


	line = line.substr(field_start, field_end - field_start);
	if(line.length() > 5) {
		line.erase(5,1);
		line.insert(3,".");
	}

	return line;
}

string getDust(string line) {
	int field_start = 0;
	int field_end = 0;

	// find "starting" comma
	for(int i = 0; i < 3; i++)
		field_start = line.find(':',field_start+1);
	field_start += 2;

	// find "ending" comma
	field_end = line.find('mg',field_end) - 2;

	return line.substr(field_start, field_end - field_start);
}

string getSound(string line) {
	int field_start = 0;
	int field_end = 0;

	// find "starting" comma
	field_start = line.find(':',field_start+1) + 2;

	// find "ending" comma
	field_end = line.find('dB',field_end) - 2;

	return line.substr(field_start, field_end - field_start);
}

double time2stamp1(int n_i, int m) {
	// only for files with the following format:
	// 2016_03_14_15_32_19_681, ...

	// n_i: Which fin
	// m: Which line, starting from 0

  	// Reference Date (01.01.1970)
	struct tm referenceDateComponent = {0};
	referenceDateComponent.tm_hour = 0;
	referenceDateComponent.tm_min = 0;
	referenceDateComponent.tm_sec = 0;
	referenceDateComponent.tm_year = 70;
	referenceDateComponent.tm_mon = 1;
	referenceDateComponent.tm_mday = 1;
	time_t referenceDate = mktime(&referenceDateComponent);

	// Ignore empty lines
	/*string line = "";
	while(line.size() < 10) {
		std::getline(fin[n_i], line);
	}*/

	string line = readLine(n_i,m);

	// Read File Date
//	int tmp;
//	int field_start = 0;
//	if ( ! (istringstream(line.substr(field_start+11,2)) >> tmp) ) tmp = 0;
//	referenceDateComponent.tm_hour = tmp;
//	if ( ! (istringstream(line.substr(field_start+14,2)) >> tmp) ) tmp = 0;
//	referenceDateComponent.tm_min = tmp;
//	if ( ! (istringstream(line.substr(field_start+17,2)) >> tmp) ) tmp = 0;
//	referenceDateComponent.tm_sec = tmp;
//	if ( ! (istringstream(line.substr(field_start+2,2)) >> tmp) ) tmp = 0;
//	referenceDateComponent.tm_year = tmp + 100;
//	if ( ! (istringstream(line.substr(field_start+5,2)) >> tmp) ) tmp = 0;
//	referenceDateComponent.tm_mon = tmp - 1;
//	if ( ! (istringstream(line.substr(field_start+8,2)) >> tmp) ) tmp = 0;
//	referenceDateComponent.tm_mday = tmp;
//	time_t readDate = mktime(&referenceDateComponent);
	referenceDateComponent.tm_hour = stoi(line.substr(11,2));
	referenceDateComponent.tm_min = stoi(line.substr(14,2));
	referenceDateComponent.tm_sec = stoi(line.substr(17,2));
	referenceDateComponent.tm_year = stoi(line.substr(2,2)) + 100;
	referenceDateComponent.tm_mon = stoi(line.substr(5,2)) - 1;
	referenceDateComponent.tm_mday = stoi(line.substr(8,2));
	time_t readDate = mktime(&referenceDateComponent);

	// Print string to check
	//char buff[20];
	//strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&referenceDate));
	//cout << buff << endl;

	// Conversion Time to UNIX-Timestamp
  	double seconds = difftime(readDate,referenceDate) + 31*24*3600; // fix month

	return seconds;
}

double time2stamp2(int n_i, int m) {
	// only for files with the following format:
	// 14802,"2016-03-14 15:35:56", ...

	// n_i: Which fin
	// m: Which line, starting from 0

  	// Reference Date (01.01.1970)
	struct tm referenceDateComponent = {0};
	referenceDateComponent.tm_hour = 0;
	referenceDateComponent.tm_min = 0;
	referenceDateComponent.tm_sec = 0;
	referenceDateComponent.tm_year = 70;
	referenceDateComponent.tm_mon = 1;
	referenceDateComponent.tm_mday = 1;
	time_t referenceDate = mktime(&referenceDateComponent);

	// Ignore empty lines
	/*string line = "";
	while(line.size() < 10) {
		std::getline(fin[n_i], line);
	}*/

	string line = readLine(n_i,m);

	int field_start = 0;
	// find "starting" apostroph
	for(int i = 0; i < 2; i++)
		field_start = line.find('"',field_start+1);
	field_start++;

	// Read File Date
//	int tmp;
//	if ( ! (istringstream(line.substr(field_start+11,2)) >> tmp) ) tmp = 0;
//	referenceDateComponent.tm_hour = tmp;
//	if ( ! (istringstream(line.substr(field_start+14,2)) >> tmp) ) tmp = 0;
//	referenceDateComponent.tm_min = tmp;
//	if ( ! (istringstream(line.substr(field_start+17,2)) >> tmp) ) tmp = 0;
//	referenceDateComponent.tm_sec = tmp;
//	if ( ! (istringstream(line.substr(field_start+2,2)) >> tmp) ) tmp = 0;
//	referenceDateComponent.tm_year = tmp + 100;
//	if ( ! (istringstream(line.substr(field_start+5,2)) >> tmp) ) tmp = 0;
//	referenceDateComponent.tm_mon = tmp - 1;
//	if ( ! (istringstream(line.substr(field_start+8,2)) >> tmp) ) tmp = 0;
//	referenceDateComponent.tm_mday = tmp;
//	time_t readDate = mktime(&referenceDateComponent);
	referenceDateComponent.tm_hour = stoi(line.substr(field_start+11,2));
	referenceDateComponent.tm_min = stoi(line.substr(field_start+14,2));
	referenceDateComponent.tm_sec = stoi(line.substr(field_start+17,2));
	referenceDateComponent.tm_year = stoi(line.substr(field_start+2,2)) + 100;
	referenceDateComponent.tm_mon = stoi(line.substr(field_start+5,2)) - 1;
	referenceDateComponent.tm_mday = stoi(line.substr(field_start+8,2));
	time_t readDate = mktime(&referenceDateComponent);

	// Print string to check
	//char buff[20];
	//strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&referenceDate));
	//cout << buff << endl;

	// Conversion Time to UNIX-Timestamp
  	double seconds = difftime(readDate,referenceDate) + 31*24*3600; // fix month

	return seconds;
}

long getMinTime(bool min) {
	if(min) {
		long T_min = 0;
//        long tmp;
//        if ( ! (istringstream(readLine(0,0)) >> tmp) ) tmp = 0;
//		Tmin_i[0] = tmp;	// HR
//		Tmin_i[1] = time2stamp1(1,0);		// GPS
//		Tmin_i[2] = time2stamp1(2,0+1);		// WaspCity
//		Tmin_i[3] = time2stamp2(3,0+1);		// WiFi Scanner
//        if ( ! (istringstream(readLine(4,0)) >> tmp) ) tmp = 0;
//		Tmin_i[4] = tmp;		// EDA
//        if ( ! (istringstream(readLine(5,0)) >> tmp) ) tmp = 0;
//		Tmin_i[5] = tmp;		// TEMP
		Tmin_i[0] = stol(readLine(0,0));	// HR
		Tmin_i[1] = time2stamp1(1,0+1);		// GPS
		Tmin_i[2] = time2stamp1(2,0+1);		// WaspCity
		Tmin_i[3] = time2stamp2(3,0+1);		// WiFi Scanner
		Tmin_i[4] = stol(readLine(4,0));		// EDA
		Tmin_i[5] = stol(readLine(5,0));		// TEMP

		cout << "Order of the times below:" << endl
		<< "HR\nGPS\nWaspCity\nWiFi\nEDA\nTEMP\n\n";
		cout << "Start Times:" << endl;
		for(int i = 0; i < N; i++) {
			cout << Tmin_i[i] << endl;
			if(Tmin_i[i] > T_min)
				T_min = Tmin_i[i];
		}
		cout << endl;
		return T_min;
	}
	else {
		long T_max = LONG_MAX;
		long T_i[N];
		long N_T[N];

		// Count lines
		int n;
		string line;
		for (int i = 0; i < N; i++) {
			for (n = 0; std::getline(fin[i], line); n++)
				;
			N_T[i] = n;
		}

//		long tmp;
//        if ( ! (istringstream(readLine(0,0)) >> tmp) ) tmp = 0;
//		T_i[0] = tmp + N_T[0]*f_hr;
//		T_i[1] = time2stamp1(1,-1);
//		T_i[2] = time2stamp1(2,-1);
//		T_i[3] = time2stamp2(3,-1);
//        if ( ! (istringstream(readLine(4,0)) >> tmp) ) tmp = 0;
//		T_i[4] = tmp + N_T[4]*f_eda;		// EDA
//        if ( ! (istringstream(readLine(5,0)) >> tmp) ) tmp = 0;
//		T_i[5] = tmp + N_T[0]*f_temp;		// TEMP
		T_i[0] = stol(readLine(0,0)) + N_T[0]*f_hr;
		T_i[1] = time2stamp1(1,-1);
		T_i[2] = time2stamp1(2,-1);
		T_i[3] = time2stamp2(3,-1);
		T_i[4] = stol(readLine(4,0)) + N_T[4]*f_eda;		// EDA
		T_i[5] = stol(readLine(5,0)) + N_T[0]*f_temp;		// TEMP

		cout << "End Times:" << endl;
		for(int i = 0; i < N; i++) {
			cout << T_i[i] << endl;
			if(T_i[i] < T_max)
				T_max = T_i[i];
		}
		cout << endl;
		return T_max;
	}
}

// Convert WGS lat/long (° dec) to CH x
double WGStoCHx(double lat, double lng) {
	// Converts dec degrees to sex seconds
	lat = ToSexAngle(lat);
	lng = ToSexAngle(lng);

	// Auxiliary values (% Bern)
	double lat_aux = (lat - 169028.66) / 10000;
	double lng_aux = (lng - 26782.5) / 10000;

	// Process X
	double x = ((200147.07 + (308807.95 * lat_aux)
			+ (3745.25 * pow(lng_aux, 2)) + (76.63 * pow(lat_aux,
			2))) - (194.56 * pow(lng_aux, 2) * lat_aux))
			+ (119.79 * pow(lat_aux, 3));

	return x;
}

// Convert WGS lat/long (° dec) to CH y
double WGStoCHy(double lat, double lng) {
	// Converts dec degrees to sex seconds
	lat = ToSexAngle(lat);
	lng = ToSexAngle(lng);

	// Auxiliary values (% Bern)
	double lat_aux = (lat - 169028.66) / 10000;
	double lng_aux = (lng - 26782.5) / 10000;

	// Process Y
	double y = (600072.37 + (211455.93 * lng_aux))
			- (10938.51 * lng_aux * lat_aux)
			- (0.36 * lng_aux * pow(lat_aux, 2))
			- (44.54 * pow(lng_aux, 3));

	return y;
}

// Convert decimal angle (degrees) to sexagesimal angle (seconds)
double DecToSexAngle(double dec) {
	int deg = (int) floor(dec);
	int min = (int) floor((dec - deg) * 60);
	double sec = (((dec - deg) * 60) - min) * 60;

	return sec + min*60.0 + deg*3600.0;
}

double ToSexAngle(double val) {
    double deg = floor(val);
    double min = 100 * (val - deg);
    return min*60 + deg*3600;
}

double getGreen(double CHx, double CHy) {
    // CHx ~ 600000, CHy ~ 200000
    if(CHx == 0 && CHy == 0)    // no GPS data available
        return -1;

    // Rescale: 334 Pixel ~ 90 Meters
    double m2p = 334./90.;
    // Calibrate Pixels with CH1903, x and y reversed
    int px = round((CHx - 680759)*m2p) + 63;
    int py = round((-CHy + 247883)*m2p) + 63;

    //cout << "px=" << px << endl << "py=" << py;
    int x;
    int y;
    int r = 25*m2p; // Radius of circle [m]
    float rbRatio;
    float rAmount = 1.06;
    float bAmount = 1.22;
    int pixTot = 0;
    int pixMatch = 0;

    // run over a 1st time to detect green pixels
    for(int i = -r; i <= r; i++)
        for(int j = -r; j <= r; j++) {
                if((i*i + j*j) < (r*r)) {
                    x = px + j;
                    y = py + i;
                    if((x < 0) || (y < 0) || (x >= image_h) || (y >= image_w))  // Pixel outside image
                        return -1;
                    // Calculate Greenery
                    unsigned char r = image(y, x, 0, 0);
                    unsigned char g = image(y, x, 0, 1);
                    unsigned char b = image(y, x, 0, 2);

                    // Rule for detecting green of trees, grass etc.
                    if((r > 10 && g > 10 && b > 10) &&
                        (rAmount*r < g && bAmount*b < g)) {
                            // obviously green pixel
                            greenMat[y][x] = true;
                        }
                    else
                        // no green pixel
                        greenMat[y][x] = false;
                }
        }

    // run over a 2nd time to wipe "lonely" green pixels out => more accuracy
    int n_k = 3;
    for(int i = -r+n_k; i < r-n_k; i++)
        for(int j = -r+n_k; j < r-n_k; j++) {
            x = px + j;
            y = py + i;
            for(int k = 1; k < n_k+1; k++) {
                if((i*i + j*j) < (r*r)) {
                    if(greenMat[y][x] && (greenMat[y-k][x] || greenMat[y][x+k] || greenMat[y+k][x] || greenMat[y][x-k]))    // one neighbour pixel is green as well
                        break;
                    else if(k == n_k && greenMat[y][x]) // false alarm
                    {
                        greenMat[y][x] = false;
                        // Check false alarm
                        //const float color[] = {255.0,0.0,0.0};
                        //image.draw_point(i,j,color);
                    }
                }
            }
    }

    // run over a 3rd time to calculate green
    for(int i = -r; i <= r; i++)
        for(int j = -r; j <= r; j++) {
            x = px + j;
            y = py + i;
            if((i*i + j*j) < (r*r)) {
                // sum up #pixels of circle
                pixTot++;
                // Mark walk
                const float color[] = {0.0,0.0,255.0};
                image_chk.draw_point(y,x,color);
                if(greenMat[y][x]) {
                    // sum up #pixels which are green
                    pixMatch++;
                    const float color[] = {255.0,0.0,0.0};
                    // Mark green pixels along walk
                    //image_chk.draw_point(y,x,color);
                }
            }
    }

/*
    // TEST GREEN CALCULATION (iteration over whole file)
    // run over a 1st time to detect green pixels
    for(int i = 0; i < image_w; i++)
        for(int j = 0; j < image_h; j++) {
                // Calculate Greenery
                unsigned char r = image(i, j, 0, 0);
                unsigned char g = image(i, j, 0, 1);
                unsigned char b = image(i, j, 0, 2);

                // Rule for detecting green of trees, grass etc.
                if((r > 10 && g > 10 && b > 10) &&
                    (rAmount*r < g && bAmount*b < g)) {
                        // obviously green pixel
                        greenMat[j][i] = true;
                    }
                else
                    // no green pixel
                    greenMat[j][i] = false;
        }

    // run over a 2nd time to wipe "lonely" green pixels out => more accuracy
    int n_k = 3;
    for(int i = n_k; i < image_w-n_k; i++)
    for(int j = n_k; j < image_h-n_k; j++) {
            for(int k = 1; k < n_k+1; k++) {
                if(greenMat[j][i] && (greenMat[j-k][i] || greenMat[j][i+k] || greenMat[j+k][i] || greenMat[j][i-k]))    // one neighbour pixel is green as well
                    break;
                else if(k == n_k && greenMat[j][i]) // false alarm
                {
                    greenMat[j][i] = false;
                    //cout << endl << "False Alarm";
            //const float color[] = {255.0,0.0,0.0};
            //image.draw_point(i,j,color);
                }
            }
    }

    // run over a 3rd time to calculate green
    for(int i = 1; i < image_w-1; i++)
    for(int j = 1; j < image_h-1; j++) {
        // sum up #pixels of circle
        pixTot++;
        if(greenMat[j][i]) {
            // sum up #pixels which are green
            pixMatch++;
            const float color[] = {255.0,0.0,0.0};
            image.draw_point(i,j,color);
        }
    }
*/
	return (double)pixMatch/pixTot;
}

int stoi(string s) {
    int i;
    if ( ! (istringstream(s) >> i) ) i = 0;
    return i;
}

long stol(string s) {
    long l;
    if ( ! (istringstream(s) >> l) ) l = 0;
    return l;
}

double stod(string s) {
    double d;
    if ( ! (istringstream(s) >> d) ) d = 0;
    return d;
}


//double WGStoCHx(double lat, double lng) {
//	//converts WGS84 coordinates (lat,long) to the Swiss coordinates. See http://geomatics.ladetto.ch/ch1903_wgs84_de.pdf
//	//The elevation is supposed to be above sea level, so it does not require any conversion
//	//lat and long must be decimal (and they will be converted to seconds)
//	const double phi_p = (lat*3600. - 169028.66) / 10000.;
//	const double lambda_p = (lng*3600. - 26782.5) / 10000.;
//
//	double east_out = 600072.37
//		+ 211455.93	* lambda_p
//		- 10938.51	* lambda_p * phi_p
//		- 0.36		* lambda_p * (phi_p*phi_p)
//		- 44.54		* (lambda_p*lambda_p*lambda_p);
//
//	return east_out;
//}
//
//double WGStoCHy(double lat, double lng) {
//	//converts WGS84 coordinates (lat,long) to the Swiss coordinates. See http://geomatics.ladetto.ch/ch1903_wgs84_de.pdf
//	//The elevation is supposed to be above sea level, so it does not require any conversion
//	//lat and long must be decimal (and they will be converted to seconds)
//	const double phi_p = (lat*3600. - 169028.66) / 10000.;
//	const double lambda_p = (lng*3600. - 26782.5) / 10000.;
//
//	double north_out = 200147.07
//		+ 308807.95	* phi_p
//		+ 3745.25	* (lambda_p*lambda_p)
//		+ 76.63		* (phi_p*phi_p)
//		- 194.56	* (lambda_p*lambda_p) * phi_p
//		+ 119.79	* (phi_p*phi_p*phi_p);
//
//	return north_out;
//}

