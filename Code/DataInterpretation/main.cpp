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

string inputs = "inputs.txt"; //Source file #1 we can add as many as we want

struct Line ///time, HR, EDA. Temp, Sound, Dust, Wifi, Lon, Lat
{
    double data[9]; /// set mac column size;
};
struct tLine ///Input file names
{
    string data[2];
};
struct DataSet
{
    tLine* inputnames;
    Line* Startzeile;
    int Zeilenzahl;
    int Spaltenzahl;
};
DataSet readfile(string filename, int columns);
void printData(DataSet Sample);
void writeDtoF(DataSet Sample, string filename);
void pauseoutput(void)
{
    cout << "\nPress Enter to continue\n";
    cin.get();
}

int main()
{
    DataSet target = readfile(inputs, 2); ///getting input target files, needs to be read as string. Maybe target names as well.

    DataSet* Set = new DataSet[target.Zeilenzahl]; ///Creating #Dataset pointers

    for(int i = 0; i < target.Zeilenzahl; ++i)
    {
        Set[i] = readfile(target.inputnames[i].data[0], 9); /// reading file
        ///Analyzation start

        ///Analyzation end

        //printData(Set1); /// printing on the cmd window
        writeDtoF(Set[i], target.inputnames[i].data[1]); /// writing comma-separated to file
    }
    cout << endl << target.Zeilenzahl << " files written\n";
    pauseoutput();



    /// delete arrays
    for(int i = 0; i < target.Zeilenzahl; ++i)
    {
        delete Set[i].Startzeile; /// deleting Pointer, has to be done for every Dataset.
        delete Set[i].inputnames;
    }
    delete[] Set;

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
    tLine* tZeile = new tLine[filelength]; /// for input files
    Result.Startzeile = Zeile;
    Result.inputnames = tZeile;

    ///read data to array
    for(int i= 0; i< filelength; ++i)
    {
        getline(fin, line);

        int field_start = 0;
        int field_end = 0;

        if(filename == inputs) for(int j = 0; j<columns; ++j)
            {
                field_end = line.find(',',field_end);

                dump = line.substr(field_start, field_end - field_start);
                field_start = ++field_end;
                tZeile[i].data[j]= dump.c_str();
            }
        else for(int j = 0; j<columns; ++j)
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

    cout << "File " << filename <<" successfully closed! -- " << Sample.Zeilenzahl << " Lines written!\n";
}
