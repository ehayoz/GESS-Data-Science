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

string inputs = "inputs.txt"; ///Source file # we can add as many as we want, #1 file is the master file
const int arlen = 10; /// array length, setting longer than true length causes random values to be written.
const int quelen = 10; ///number of question + 1 for timestamp
const int nquest = 14; ///number of question sets per DataSet
const int gpspos = 8; ///Position of the Latitude column


struct Line ///time, HR, EDA. Temp, Sound, Dust, Wifi, Lon, Lat
{
    double data[arlen]; /// set mac column size;
};
struct tLine ///Input file names
{
    string data[2];
};
struct DataSet
{
    tLine* inputnames;
    Line* Startzeile;
    Line* Fragen;
    int Zeilenzahl;
    int Spaltenzahl;
    double stdev[arlen];
    double mean[arlen];
};
double stod(string s);
DataSet readfile(string filename, int columns);
void printData(DataSet Sample);
void writeDtoF(DataSet Sample, string filename);

double moodcoefficient(DataSet Sample);

double getsum(Line* Sample, int line, int column);
double getmean(Line* Sample, int line, int column);
double getstdev(Line* Sample, int line, int column);
bool chkeql(double Data1, double Data2, double deviation);
bool chkmap(Line Sample1, Line Sample2, double deviation);

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
        Set[i] = readfile(target.inputnames[i].data[0], arlen); /// reading file
    }

    ///Analyzation start

    //getsum(Set[1].Startzeile, Set[1].Zeilenzahl, 2);



    ///Analyzation end



    for(int i = 0; i < target.Zeilenzahl; ++i)
    {
        printData(Set[i]); /// printing on the cmd window
        writeDtoF(Set[i], target.inputnames[i].data[1]); /// writing comma-separated to file
    }
    cout << endl << target.Zeilenzahl << " files written\n";
    pauseoutput();



    /// delete arrays
    for(int i = 0; i < target.Zeilenzahl; ++i)
    {
        delete Set[i].Startzeile; /// deleting Pointer, has to be done for every Dataset.
        delete Set[i].inputnames;
        delete Set[i].Fragen;
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

    Line* Zeile = new Line[filelength]; /// for data
    Line* qZeile = new Line[nquest]; /// for cellphone questions
    tLine* tZeile = new tLine[filelength]; /// for input files

    Result.inputnames = tZeile;
    Result.Startzeile = Zeile;
    Result.Fragen = qZeile;

    int n = 0; ///actual questionentry
    double previous;

    ///read data to array
    for(int i= 0; i< filelength; ++i)
    {
        getline(fin, line);

        int field_start = 0;
        int field_end = 0;

        if(filename == inputs)
        {
            for(int j = 0; j<columns; ++j)/// get input filenames
            {
                field_end = line.find(',',field_end);

                dump = line.substr(field_start, field_end - field_start);
                field_start = ++field_end;
                tZeile[i].data[j]= dump.c_str();
            }
        }
        else
        {
            for(int j = 0; j<columns; ++j)///get data sets
            {
                field_end = line.find(',',field_end);

                dump = line.substr(field_start, field_end - field_start);
                field_start = ++field_end;
                Zeile[i].data[j]= stod(dump);
            }


            ///question result reading
            field_end = line.find(',',field_end);
            dump = line.substr(field_start, field_end - field_start);
            field_start = ++field_end;

            if((n == 0 || previous != stod(dump)) && n<nquest) ///if number is equal to previous entry skip
            {
                qZeile[n].data[0]= Zeile[i].data[0];
                qZeile[n].data[1]= previous = stod(dump);

                for(int j = 2; j<quelen; ++j) /// get questions
                {
                    field_end = line.find(',',field_end);
                    dump = line.substr(field_start, field_end - field_start);
                    field_start = ++field_end;
                    qZeile[n].data[j]= stod(dump);
                }
                ++n;
            }
            ///question result reading end*/
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

    strs << endl << "===========Questions===========\n\n";

    for(int i = 0; i < nquest; ++i)
    {
        for(int j = 0; j<quelen; ++j)
        {
            strs << setprecision(7) << Sample.Fragen[i].data[j];
            if(j < quelen-1) strs << ",";
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
        pauseoutput();
        return;
    }
    fout << str;
    fout.close();

    cout << "File " << filename <<" successfully closed! -- " << Sample.Zeilenzahl << " Lines written!\n";
}

double stod(string s) {
    double d;
    if ( ! (istringstream(s) >> d) ) d = 0;
    return d;
}


double moodcoefficient(DataSet Sample)
{
    ///To be done
    return true;
}

double getsum(Line* Sample, int line, int column) /// give linepointer, number of lines and column to sum up
{
    double sum = 0;
    for(int i = 0; i < line; ++i)
    {
        sum += Sample[i].data[column];
    }
    return sum;
}

double getmean(Line* Sample, int line, int column) /// give linepointer, number of lines and column to derivate the mean
{
    double sum = getsum(Sample, line, column);
    sum /= line;
    return sum;
}

double getstdev(Line* Sample, int line, int column) /// give linepointer, number of lines and column to derivate the standard deviation
{
    double mean = getmean(Sample, line, column);
    double sqsum = 0;
    for(int i = 0; i< line; ++i) sqsum += pow((Sample[i].data[column]-mean),2);
    sqsum /= line;
    sqsum = pow(sqsum, 0.5);
    return sqsum;
}

bool chkeql(double Data1, double Data2, double deviation)/// give linepointer, the line and column to
{
    double diffrence = Data1 - Data2;
    if(diffrence<1) diffrence *= -1;
    if(diffrence<deviation) return true;
    else return false;
}

bool chkmap(Line Sample1, Line Sample2, double deviation)/// give to lines to compare their location
{
    double difx = Sample1.data[gpspos] - Sample2.data[gpspos];
    double dify = Sample1.data[gpspos+1] - Sample2.data[gpspos+1];
    difx *= difx;
    dify *= dify;
    deviation *= deviation;
    if((difx+dify)<deviation) return true;
    else return false;
}
