#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <ctime>
#include <chrono>
#include <climits>
#include <cmath>
#include <stdlib.h>
#include <windows.h>
#include <iomanip>



using namespace std;

string inFileName = "Data_all.csv"; //Source file #1 we can add as many as we want

struct Line ///time, HR, EDA. Temp, Sound, Dust, Wifi, Lon, Lat
{
    double data[9];
};
struct DataSet
{
    Line* Startzeile;
    int Zeilenzahl;
    int Spaltenzahl;
};
DataSet readfile(string filename, int columns);
void printData(DataSet Sample);
void writeDtoF(DataSet Sample, string filename);
void pauseoutput(void){cout << "\nPress Enter to continue\n"; cin.get();}

int main()
{
    DataSet Set1 = readfile(inFileName, 9); /// reading file
    //printData(Set1); /// printing on the cmd window
    writeDtoF(Set1, "test.csv"); /// writing comma-separated to file
    pauseoutput();

    /// delete array
    delete Set1.Startzeile; /// deleting Pointer, has to be done for every Dataset.
    return 0;
}

DataSet readfile(string filename, int columns)
{
    ///Open file
    ifstream fin;
    fin.open(filename);
    string line;
    string dump;
    DataSet Result;
    Result.Spaltenzahl = columns;

    /// get #line and reset the getline position
    int filelength;
    for (filelength = 0; std::getline(fin, line); filelength++)
        ;
    Result.Zeilenzahl = filelength;
    fin.clear();
    fin.seekg(0, ios::beg);

    /// create Line array of size filelength

    Line* Zeile = new Line[filelength];
    Result.Startzeile = Zeile;

    ///read data to array
    for(int i= 0; i< filelength; ++i)
    {
        getline(fin, line);

        int field_start = 0;
        int field_end = 0;

        for(int j = 0; j<columns; ++j)
        {
            field_end = line.find(',',field_end);

            dump = line.substr(field_start, field_end - field_start);
            field_start = ++field_end;
            Zeile[i].data[j]= atof(dump.c_str());
        }
    }
    fin.close();
    return Result;
}

void printData(DataSet Sample)
{
    system("color A");
    ostringstream strs;
    for(int i = 0; i < Sample.Zeilenzahl; ++i)
    {
        for(int j = 0; j<Sample.Spaltenzahl; ++j)
        {
            strs << setprecision(7) << Sample.Startzeile[i].data[j];
            if(j < Sample.Spaltenzahl-1) strs << ",";
        }
        strs << endl;
    }
    string str = strs.str();
    cout << str;
}

void writeDtoF(DataSet Sample, string filename)
{
    system("color A");
    ostringstream strs;
    for(int i = 0; i < Sample.Zeilenzahl; ++i)
    {
        for(int j = 0; j<Sample.Spaltenzahl; ++j)
        {
            strs << setprecision(11) << Sample.Startzeile[i].data[j];
            if(j < Sample.Spaltenzahl-1) strs << ",";
        }
        strs << endl;
    }
    string str = strs.str();

    ///Schreiben des files
    ofstream fout;
    fout.open(filename.c_str(), ios::trunc);
    if (!fout.is_open ())
    {
        cout << "Fehler beim Oeffnen von " << filename << "! Keine Daten geschrieben!\n";
    }
    fout << str;
    fout.close();

    cout << "File " << filename <<" successfully closed!\n" << Sample.Zeilenzahl << " Lines written!\n";
}
