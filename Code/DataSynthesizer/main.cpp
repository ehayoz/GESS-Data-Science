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
[Timestamp],[HR],[BVP],[EDA],[TempBF],[Sound],[Dust],[TempEN],[RH],[Light],[#WiFi],[LON],[LAT],[Greenery],[S No.],[Q1],[Q2],[Q3],[Q4],[Q5],[Q6],[Q7],[Q8],[Q9],[Q10],[Q11],[Q12]
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
float f_bvp = 64;
float f_hobo = 1;

int epochFix = 3600;    // because conversion on our computer generate an error of 1h

ifstream fin[9];
const int N = (sizeof(fin)/sizeof(fin[0]));
ofstream fout[2];
int No = (sizeof(fout)/sizeof(fout[0]));
long Tmin_i[N];
float green_prev = 0;
int progress = 0;
int r_vu = 100;
int b_vu = 0;
long minTime;
long maxTime;
long minQuestTime;
long maxQuestTime;

void init();
void concat();
void cleanup();
string getHR();
string getEDA();
string getTempBF();
string getLAT(string);
string getLON(string);
string getSound(string);
string getDust(string);
string getTempEN(string);
string getRH(string);
string getLight(string);
string getQuestion(string);
string readLine(int, int);
double time2stamp1(int,int);
double time2stamp2(int,int);
double time2stamp3(int,int);
double time2stamp4(int,int);
long checkMinTime(bool);
long getMinQuestTime(bool);
double WGStoCHx(double, double);
double WGStoCHy(double, double);
double DecToSexAngle(double);
double ToSexAngle(double);
float getGreen(double, double);
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
  fin[6].open("data/2016-04-08-1250_status.csv");   // TODO allgemein formulieren
  fin[7].open("data/BVP.csv");
  fin[8].open("data/Temp_RH_Light_10_11.csv");
  fout[0].open("data/Data_all.csv");
  fout[1].open("data/GPS_only.csv");
  for(int i = 0; i < No; i++) {
    fout[i].setf(ios::fixed,ios::floatfield);
    fout[i].precision(4);
  }
  image.save("img/map_chk.bmp");
}

void cleanup() {
  for(int i = 0; i < N; i++)
  	fin[i].close();
  for(int i = 0; i < No; i++)
  	fout[i].close();
}

void concat() {
    // Check if all times overlap
    minTime = checkMinTime(true);
    maxTime = checkMinTime(false);
    if(minTime >= maxTime)
        cout << "Error: Start Time >= End Time. Check your input data and restart program." << endl << endl;
    // But we will take the survey time as the reference:
	minTime = getMinQuestTime(true);
	maxTime = getMinQuestTime(false);

	cout << "Press Enter to start concatenating the data and evaluate greenery";
	//cin.get();
	cout << "Loading...";
	int m1 = 0 + 1; // [FIX, +1]
	int m2 = 0 + 1; // fix empty line
	int m3 = 0 + 1;
	int m3_prev = 1;
	int m4 = 0 + 3;
	int m5 = 0;
	int n_device = 0;
	bool first = true;
	int progress_prev = -1;

	double CHx;
	double CHy;
	double WGSx;
	double WGSy;
	image.save("img/map_chk.bmp");

	for(double t = minTime; t <= maxTime; t += 1/f_hr) {
            //if(t >= 1460123239)// 1460123239
            //    break;
		progress = ceil((double)(t-minTime)/(maxTime - minTime)* 100);
		if(progress != progress_prev) {
			// Progress Bar, only cout if new progress to save time
			system("CLS");
			cout << "Generating Data Files. " << "Progress: " << progress << "%";
			progress_prev = progress;
		}

		// is for [Sound],[Dust],
		while(time2stamp1(2,m2) < t) {
			m2++;
		}
		// is for [LON],[LAT]
		while(time2stamp1(1,m1) < t) {
			m1++;
		}
		// is for [WiFi]
		while(time2stamp2(3,m3) < t) {
			m3++;
		}
		if(m3 != m3_prev && !first)	// only refresh n_device if there is a new measure
		{
			n_device = m3 - m3_prev - 2;	// substract 2 because of our devices
			//cout << "m3=" << m3 << endl << "m3_prev=" << m3_prev << endl << "n_device=" << n_device << endl; cin.get();
		}

		if(n_device < 0)
			n_device = 0;

        // is for [TempEN],[RH],[Light]
        while(time2stamp4(8,m4) < t) {
            m4++;
        }
        // is for [Survey]
        while(time2stamp3(6,m5) < t) {
            if(time2stamp3(6,m5 + 1) < t)
               m5++;
            else
                break;
        }
        //cin.get();

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

		// FOUT [Timestamp],[HR],[BVP],[EDA],[TempBF],
		fout[0] << t << ',' << readLine(0, (t-Tmin_i[0])*f_hr+2) << ',' << readLine(7, (t-Tmin_i[7])*f_bvp+2) << ',' << readLine(4, (t-Tmin_i[4])*f_eda+2) << ',' << readLine(5, (t-Tmin_i[5])*f_temp+2) << ','
		// FOUT [Sound],[Dust],
			 << getSound(readLine(2,m2)) << ',' << getDust(readLine(2,m2)) << ','
        // FOUT [TempEN],[RH],[Light],
             << getTempEN(readLine(8,m4)) << ',' << getRH(readLine(8,m4)) << ',' << getLight(readLine(8,m4)) << ','
		// FOUT [#WiFi]
			 << n_device << ','
	    // FOUT [LON],[LAT]
			 // << getLON(readLine(1,m1)) << ',' << getLAT(readLine(1,m1)) << endl;
			 << CHx << ',' << CHy << ','
        // FOUT [Greenery]
			 //<< (float) round(getGreen(CHx,CHy)*10000)/10000
			 << ','
        // FOUT [S No],[Q1],[Q2],[Q3],[Q4],[Q5],[Q6],[Q7],[Q8],[Q9],[Q10],[Q11],[Q12]
             << getQuestion(readLine(6,m5)) << endl;

	    // FOUT [LON],[LAT]
		fout[1] << CHx << ',' << CHy << endl;

		// RGB value updater, from R -> B
		r_vu -= (float)((t-minTime)/(maxTime-minTime))*100;
		b_vu += (float)((t-minTime)/(maxTime-minTime))*100;

		m3_prev = m3;
		first = false;
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

string getTempEN(string line) {
	int field_start = 0;
	int field_end = 0;
	// find "starting" comma
	for(int i = 0; i < 2; i++)
		field_start = line.find(',',field_start+1) + 1;

	// find "ending" comma
	field_end = line.find(',',field_start+1);

    return line.substr(field_start, field_end - field_start);
}
// problem: 2841,04/08/16 01:47:20 PM,12.775,46.367,2321.8,,,
string getRH(string line) {
	int field_start = 0;
	int field_end = 0;
	// find "starting" comma
	for(int i = 0; i < 3; i++)
		field_start = line.find(',',field_start+1) + 1;

	// find "ending" comma
	field_end = line.find(',',field_start+1);

    return line.substr(field_start, field_end - field_start);
}

string getLight(string line) {
	int field_start = 0;
	int field_end = 0;
	// find "starting" comma
	for(int i = 0; i < 4; i++)
		field_start = line.find(',',field_start+1) + 1;

	// find "ending" comma
	field_end = line.find(',',field_start+1);

    return line.substr(field_start, field_end - field_start);
}

string getQuestion(string line) {
	int field_start = 0;
	//int field_end = 0;
	// find "starting" comma
	for(int i = 0; i < 1; i++)
		field_start = line.find(',',field_start+1) + 1;

	// find "ending" comma
	//field_end = line.find(',',field_start+1);

    return line.substr(field_start);
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
	for(int i = 0; i < 1; i++)
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
//	if(n_i == 3 && m == -1) {
//            cout << field_start << endl;
//            cout << "Entries:\n";
//            cout << line << endl;
//        cout << stoi(line.substr(field_start+11,2)) << endl;
//        cout << stoi(line.substr(field_start+14,2)) << endl;
//        cout << stoi(line.substr(field_start+17,2)) << endl;
//        cout << stoi(line.substr(field_start+2,2)) + 100 << endl;
//        cout << stoi(line.substr(field_start+5,2)) - 1 << endl;
//        cout << stoi(line.substr(field_start+8,2)) << endl;
//    cout << "----\n";
//	}

    //cout << referenceDateComponent.tm_hour << endl; cin.get();
	time_t readDate = mktime(&referenceDateComponent);
	// Print string to check
	//char buff[20];
	//strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&referenceDate));
	//cout << buff << endl;

	// Conversion Time to UNIX-Timestamp
  	double seconds = difftime(readDate,referenceDate) + 31*24*3600; // fix month

	return seconds;
}


double time2stamp3(int n_i, int m) {
	// only for files with the following format:
	// 2016-04-08 13:23:30.435916, ...

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
	//for(int i = 0; i < 2; i++)
	//	field_start = line.find('"',field_start+1);
	//field_start++;

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
	//cout << referenceDateComponent.tm_year << endl; cin.get();
    //cout << referenceDateComponent.tm_mon << endl; cin.get();
    //cout << referenceDateComponent.tm_mday << endl; cin.get();
    //cout << referenceDateComponent.tm_hour << endl; cin.get();
    //cout << referenceDateComponent.tm_min << endl; cin.get();
    //cout << referenceDateComponent.tm_sec << endl; cin.get();

	// Print string to check
	//char buff[20];
	//strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&referenceDate));
	//cout << buff << endl;

	// Conversion Time to UNIX-Timestamp
  	double seconds = difftime(readDate,referenceDate) + 31*24*3600; // fix month
  	//cout.precision(10);
  	//cout << seconds << endl; cin.get();

	return seconds;
}

double time2stamp4(int n_i, int m) {
	// only for files with the following format:
	// 1898,04/08/16 01:31:37 PM,13.642,45.415,4868.2,,, , ...

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
	for(int i = 0; i < 1; i++)
		field_start = line.find(',',field_start+1);
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
	referenceDateComponent.tm_min = stoi(line.substr(field_start+12,2));
	referenceDateComponent.tm_sec = stoi(line.substr(field_start+15,2));
	referenceDateComponent.tm_year = stoi(line.substr(field_start+6,2)) + 100;
	referenceDateComponent.tm_mon = stoi(line.substr(field_start+0,2)) - 1;
	referenceDateComponent.tm_mday = stoi(line.substr(field_start+3,2));
	if(line.substr(field_start+18,2) == "PM") {
        referenceDateComponent.tm_hour = stoi(line.substr(field_start+9,2)) + 12;
	}
    else
        referenceDateComponent.tm_hour = stoi(line.substr(field_start+9,2));

	time_t readDate = mktime(&referenceDateComponent);

	// Print string to check
	//char buff[20];
	//strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&referenceDate));
	//cout << buff << endl;

	// Conversion Time to UNIX-Timestamp
  	double seconds = difftime(readDate,referenceDate) + 31*24*3600; // fix month
  	//cout.precision(10);
  	//cout << seconds << endl; cin.get();

	return seconds;
}

long checkMinTime(bool min) {
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
		Tmin_i[0] = stol(readLine(0,0)) + epochFix;	// HR
		Tmin_i[1] = time2stamp1(1,0+1);		// GPS
		Tmin_i[2] = time2stamp1(2,0+1);		// WaspCity
		Tmin_i[3] = time2stamp2(3,0+1);		// WiFi Scanner
		Tmin_i[4] = stol(readLine(4,0)) + epochFix;		// EDA
		Tmin_i[5] = stol(readLine(5,0)) + epochFix;		// TEMP
		Tmin_i[6] = time2stamp3(6,0);       // Survey
		Tmin_i[7] = stol(readLine(7,0)) + epochFix;    // BVP
		Tmin_i[8] = time2stamp4(8,0+3+6);   // HOBO

		cout << "Order of the times below:" << endl
		<< "HR\nGPS\nWaspCity\nWiFi\nEDA\nTEMP\nSurvey\nBVP\nHOBO\n\n";
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
		T_i[0] = stol(readLine(0,0)) + N_T[0]/f_hr + epochFix;
		T_i[1] = time2stamp1(1,-1);
		T_i[2] = time2stamp1(2,-1);
		T_i[3] = time2stamp2(3,-1);
		T_i[4] = stol(readLine(4,0)) + N_T[4]/f_eda + epochFix;		// EDA
		T_i[5] = stol(readLine(5,0)) + N_T[5]/f_temp + epochFix;		// TEMP
		T_i[6] = time2stamp3(6,-1);       // Survey
		T_i[7] = stol(readLine(7,0)) + N_T[7]/f_bvp + epochFix;
		T_i[8] = time2stamp4(8,-1);

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

long getMinQuestTime(bool min) {
    if(min)
        return time2stamp3(6,0);       // Survey
    else
        return time2stamp3(6,-1);       // Survey
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

float getGreen(double CHx, double CHy) {
    // CHx ~ 600000, CHy ~ 200000
    if(CHx == 0 && CHy == 0)    // no GPS data available
        return green_prev;

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
                        return green_prev;
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
                    else {
                        // no green pixel
                        greenMat[y][x] = false;
                    }
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
                // Mark walk, progressive
                //unsigned char r = image(y, x, 0, 0);
                //unsigned char g = image(y, x, 0, 1);
                //unsigned char b = image(y, x, 0, 2);
                const float color[] = {0,0,230};

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
    green_prev = (float)pixMatch/pixTot;
	return green_prev;
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

