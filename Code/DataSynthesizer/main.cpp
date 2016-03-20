#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <chrono>
#include <climits>

using namespace std;
using namespace std::chrono;

// ABKLÄREN:
// - TIME OF BIOFEEDBACK DOES NOT MATCH WITH GPS AND SENSORBOARD TIME

/*
Device				Abbreviation
Wasp City			wc
GPS					gps
Meshlium Scanner	ms
Biofeedback			bf
*/

// Measurement Period [s]
float f_bf = 1; // HR only with 1Hz!
float f_gps = 1;
float f_wc = 0.4;
float f_ms = 0.016;

ifstream fin[3];
int N = (sizeof(fin)/sizeof(fin[0]));
ofstream fout;

void init();
void check();
void concat();
void cleanup();
string getLAT(string);
string getLON(string);
string getSound(string);
string getDust(string);
string readLine(int, int);
double time2stamp(int);
long getMinTime(bool);

int main(int argc, char** argv) {
	init();
	check();
	concat();
	
	cleanup();
	return 0;
}

void concat() {
	long minTime = getMinTime(true);
	long maxTime = getMinTime(false);
	long baseMinTime = stol(readLine(0,0)) + 3000; // tweak because of different times
	cout << (maxTime - baseMinTime)*f_bf << endl; cin.get();
	// Get to "common" time
	// Base is HR
	for(int m = (minTime - baseMinTime)*f_bf + 2; m < (maxTime - baseMinTime)*f_bf; m++) {
		fout.precision(15);
		fout << stol(readLine(0,0)) + m/f_bf << ',' << endl;
	}
	
	
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
	
	return line.substr(field_start, field_end - field_start);
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
	
	return line.substr(field_start, field_end - field_start);
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

void init() {
  fin[0].open("HR.csv");
  fin[1].open("GPS.csv");
  fin[2].open("WaspCity.csv");
  fout.open("concat.txt");
}

void check() {
	// Check File Input
	string line;
	int n, n_max;
	cout << "Frequencies based on File Input (if frequencies differ from what they should, the devices did not log data for the same time span):\n";
	for(int i = 0; i < N; i++) {
		// Count lines
		for (n = 0; std::getline(fin[i], line); n++)
    		;
				
		// Check if number of lines match with device frequencies
		switch(i) {
			case 0: n_max = n;
					cout << "Heartrate: " << endl;
					break;
			case 1: cout << "GPS: " << endl;
					break;
			case 2: cout << "WaspCity: " << endl;
					break;
		}
		
    	cout << (float) n/n_max*f_bf << endl;
    }
    cout << endl;
}

void cleanup() {
  fout.close();
  for(int i = 0; i < N; i++)
  	fin[i].close();
}

double time2stamp(int n_i, int m) {
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

long getMinTime(bool min) {
	if(min) {
		long T_min = 0;
		long T_i[N];
	
		T_i[0] = stol(readLine(0,0));	// HR
		T_i[1] = time2stamp(1,0);		// GPS
		T_i[2] = time2stamp(2,0+1);		// WaspCity
		
		cout << "Start Times:" << endl;
		for(int i = 0; i < N; i++) {
			cout << T_i[i] << endl;
			if(T_i[i] > T_min)
				T_min = T_i[i];
		}
		cout << endl;
		return T_min;
	}
	else {
		long T_max = LONG_MAX;
		long T_i[N];
	
		// Count lines
		int n;
		string line;
		for (n = 0; std::getline(fin[0], line); n++)
			;
		T_i[0] = stol(readLine(0,0)) + n*f_bf;
		T_i[1] = time2stamp(1,-1);
		T_i[2] = time2stamp(2,-1);
		
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
