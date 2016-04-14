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

/**Most important setting, adjust to Datasamples*/

string inputs = "inputs.txt"; ///Source file # we can add as many as we want, #1 file is the master file
const int stdarlen = 9; /// length of normal Datasets, maxarlen to store additional Data
const int maxarlen = 11; /// max array length take care to not write more data
const int tmaxc = 3; ///mac text columns
const int quelen = 4; ///number of question + 1 for line in which it first occured
const int nquest = 14; ///number of question sets per DataSet


const int gpspos = 7; ///Position of the Latitude column
const int range = 50; ///conGPSdep Data Lines to be checked before and after actual point
const double precision = 0.00003; ///conGPSdep Prcision radius
const int digits = 1000000;///number of GPSdigits as 10 to the power of digits


struct Line ///time, HR, EDA. Temp, Sound, Dust, Wifi, Lon, Lat, ..., *moodcoefficient, *stresslevel
{
    double data[maxarlen]; /// set max column size;
};
struct tLine ///Input file names
{
    string data[tmaxc];
};
struct DataSet
{
    tLine* inputnames; ///contains text lines
    Line* Startzeile; /// contains pointer to first line of DataSet
    Line* Fragen; /// contains pointer to first QuestionSet
    int Zeilenzahl; /// contains #lines from Startzeile
    int Spaltenzahl; /// contains number of initialized columns, important to write to file
    double stdev[maxarlen]; /// contains stdev of specified row
    double mean[maxarlen]; /// contains mean of specified row
};
double stod(string s);
DataSet readfile(string filename, int columns);
void printData(DataSet Sample);
void writeDtoF(DataSet Sample, string filename);
DataSet copDat(const DataSet Sample);
DataSet addDat(const DataSet Sample1, const DataSet Sample2);
DataSet divideDat(const DataSet Sample, int nummean);

void moodcoefficient(DataSet& Sample);
void normalize(DataSet& Sample);
DataSet conGPSdep(const DataSet master, const DataSet slave);

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
    ///runtime detection
    clock_t t1,t2;
    t1=clock();

    ///initialize program

    DataSet target = readfile(inputs, 2); ///getting input target files, needs to be read as string. Maybe target names as well.

    DataSet* Set = new DataSet[target.Zeilenzahl]; ///Creating #Dataset pointers
    DataSet* GPSset = new DataSet[target.Zeilenzahl];

    for(int i = 0; i < target.Zeilenzahl; ++i)
    {
        Set[i] = readfile(target.inputnames[i].data[0], stdarlen); /// reading file
    }

    ///Analyzation start

    for(int i = 0; i< target.Zeilenzahl; ++i) /// for every file itself
    {
        moodcoefficient(Set[i]); ///moodcoefficient
        normalize(Set[i]);
        GPSset[i] = conGPSdep(Set[0],Set[i]);
    }

    //getsum(Set[1].Startzeile, Set[1].Zeilenzahl, 2);



    ///Analyzation end



    for(int i = 0; i < target.Zeilenzahl; ++i)
    {
        ///printData(Set[i]); /// printing on the cmd window
        writeDtoF(Set[i], target.inputnames[i].data[1]); /// writing comma-separated to file
        writeDtoF(GPSset[i], target.inputnames[i].data[2]); /// writing comma-separated to file
    }




    /// delete arrays
    for(int i = 0; i < target.Zeilenzahl; ++i)
    {
        delete Set[i].Startzeile; /// deleting Pointer, has to be done for every Dataset.
        delete Set[i].inputnames;
        delete Set[i].Fragen;
        delete GPSset[i].Startzeile; /// deleting Pointer, has to be done for every Dataset.
        delete GPSset[i].inputnames;
        delete GPSset[i].Fragen;
    }
    delete[] Set;
    delete[] GPSset;

    t2=clock();
    float diff ((float)t2-(float)t1);
    cout << endl << target.Zeilenzahl << " files written -- time needed: " << diff/1000 << " s";
    pauseoutput();
    return 0;
}


///Write/Read functions

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
    double previous = 0; ///Placeholder to save previus question tag

    ///read data to array
    for(int i= 0; i< filelength; ++i)
    {
        getline(fin, line);

        int field_start = 0;
        int field_end = 0;

        if(filename == inputs)
        {
            ///outputfilname specifcation in file
            /*for(int j = 0; j<columns; ++j)/// get input filenames
            {
                field_end = line.find(',',field_end);

                dump = line.substr(field_start, field_end - field_start);
                field_start = ++field_end;
                tZeile[i].data[j]= dump;//.c_str();
            }*/
            ///auto output naming
            field_end = line.find(',',field_end);
            dump = line.substr(field_start, field_end - field_start);
            field_start = ++field_end;

            tZeile[i].data[0]= dump;
            tZeile[i].data[1]= tZeile[i].data[0];
            for(int n = 0; n<4; ++n)
            {
                tZeile[i].data[1].pop_back();
            }
            tZeile[i].data[2]= tZeile[i].data[1];

            tZeile[i].data[1]= tZeile[i].data[1] + "_norm.csv";
            tZeile[i].data[2]= tZeile[i].data[2] + "_gps.csv";

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
                //qZeile[n].data[0]= Zeile[i].data[0]; /// question timestamp of first occurence
                qZeile[n].data[0]= i; /// question, linenumber of first occurence
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

DataSet copDat(const DataSet Sample) /// copy datasets
{
    DataSet result;
    result.Fragen = new Line[nquest]; /// for cellphone questions
    result.Startzeile = new Line[Sample.Zeilenzahl]; /// for data
    result.inputnames = new tLine[Sample.Zeilenzahl]; /// for input files
    result.Zeilenzahl = Sample.Zeilenzahl;
    result.Spaltenzahl = Sample.Spaltenzahl;

    for(int i = 0; i< Sample.Spaltenzahl; ++i)
    {
        result.mean[i] = Sample.mean[i];
        result.stdev[i] = Sample.stdev[i];
    }


    for(int i = 0; i< Sample.Zeilenzahl; ++i)
    {
        for(int j = 0; j< Sample.Spaltenzahl; ++j)
        {
            result.Startzeile[i].data[j] = Sample.Startzeile[i].data[j];
        }
        for(int j = 0; j< tmaxc; ++j)
        {
           result.inputnames[i].data[j] = Sample.inputnames[i].data[j];
        }
    }
    for(int i = 0; i< nquest; i++)
    {
        for(int j = 0; j< quelen; j++)
        {
            result.Fragen[i].data[j] = Sample.Fragen[i].data[j]; /// for cellphone questions
        }
    }
    return result;
}

DataSet addDat(const DataSet Sample1, const DataSet Sample2) /// add up to Datasets stdev and mean are lost equal spalten and zeilen zahl requiered
{
    DataSet result = copDat(Sample1);
    if(result.Zeilenzahl != Sample2.Zeilenzahl || result.Spaltenzahl != Sample2.Spaltenzahl)
    {
        cout <<"\nSummierfehler, Zeilen-und/oder Spaltenzahl nicht identisch, Ergebnis gleich Sample1\n";
        return result;
    }

    for(int i = 0; i< Sample2.Spaltenzahl; ++i)
    {
        result.mean[i] = 0;
        result.stdev[i] = 0;
    }


    for(int i = 0; i< Sample2.Zeilenzahl; ++i)
    {
        for(int j = 0; j< Sample2.Spaltenzahl; ++j)
        {
            result.Startzeile[i].data[j] += Sample2.Startzeile[i].data[j];
        }
        for(int j = 0; j< tmaxc; ++j)
        {
           result.inputnames[i].data[j] += Sample2.inputnames[i].data[j];
        }
    }
    for(int i = 0; i< nquest; i++)
    {
        for(int j = 0; j< quelen; j++)
        {
            result.Fragen[i].data[j] += Sample2.Fragen[i].data[j]; /// for cellphone questions
        }
    }
    return result;
}

DataSet divideDat(const DataSet Sample, double nummean) /// divide Dataset by double stdev and mean are lost
{
    DataSet result = copDat(Sample);

    for(int i = 0; i< Sample.Spaltenzahl; ++i)
    {
        result.mean[i] = 0;
        result.stdev[i] = 0;
    }


    for(int i = 0; i< Sample.Zeilenzahl; ++i)
    {
        for(int j = 0; j< Sample.Spaltenzahl; ++j)
        {
            result.Startzeile[i].data[j] /= nummean;
        }
    }
    for(int i = 0; i< nquest; i++)
    {
        for(int j = 0; j< quelen; j++)
        {
            result.Fragen[i].data[j] /= nummean; /// for cellphone questions
        }
    }
    return result;
}



///Helper functions

double stod(string s)
{
    double d;
    if ( ! (istringstream(s) >> d) ) d = 0;
    return d;
}



///Analyse/Manipulate functions

void moodcoefficient(DataSet& Sample) /// calculate mood
{
    for(int i = 0; i<quelen; ++i) Sample.Fragen[i].data[quelen] = 0;

    for(int i = 0; i<nquest ; ++i) ///verrechnung der Fragen zu moodcoefficient
    {
        for(int j = 2; j<quelen; ++j)
        {
            Sample.Fragen[i].data[quelen] += Sample.Fragen[i].data[j];
        }
        Sample.Fragen[i].data[quelen] /= (quelen);
    }
    for(int i = 0; i < nquest; ++i) ///schreiben der mmodcoefficients, type Euler forward
    {
        for(int j = Sample.Fragen[i].data[0]; j < Sample.Zeilenzahl; ++j)
        {
            Sample.Startzeile[j].data[stdarlen] = Sample.Fragen[i].data[quelen];
        }
    }
    Sample.Spaltenzahl = stdarlen + 1; ///mark additional column
}

void normalize(DataSet& Sample) ///normalize DataSets
{
    for(int i = 1; i< Sample.Spaltenzahl; ++i)
    {
        Sample.mean[i] = getmean(Sample.Startzeile, Sample.Zeilenzahl, i);
        Sample.stdev[i] = getstdev(Sample.Startzeile,Sample.Zeilenzahl,i);
        for(int j = 0; j < Sample.Zeilenzahl; ++j)
        {
            if(i != gpspos && i != (gpspos+1)) Sample.Startzeile[j].data[i] /= Sample.mean[i];
        }
    }
}

DataSet conGPSdep(const DataSet master, const DataSet slave) /// convert Time ordered Data to location ordered Data
{
    DataSet gpsordered = copDat(master);
    //writeDtoF(gpsordered, "test.csv");

    for(int i = 0; i<gpsordered.Zeilenzahl; ++i)
    {
        int mini = i+range;
        int maxi = 0;

        for(int j = i-range; j<slave.Zeilenzahl; ++j)
        {
            if(j<0) j = 0; ///protection from array underflow
            if((j-i)> range) break;
            if(chkmap(gpsordered.Startzeile[i], slave.Startzeile[j], precision))
            {
                if(mini > j) mini = j; ///first occurence of true
                maxi = j; ///last occurence of true
            }
        }
        cout << mini << "    " << maxi <<endl; ///clue on accuracy

        ///evaluaktion from mini to max
        for(int j = 0; j< gpsordered.Spaltenzahl; ++j)
        {
            double meandump = 0;
            if(j != gpspos && j != (gpspos+1))///To keep GPS Mastercoordinates
            {
                for(int k = mini; k <= maxi; ++k)
                {
                    meandump += slave.Startzeile[k].data[j];
                }
                meandump /= (maxi-mini);
                gpsordered.Startzeile[i].data[j] = meandump;
            }
        }
    }
    //writeDtoF(gpsordered, "test.csv");
    return gpsordered;
}



///Math functions

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



///check functions

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
    difx *= digits;
    dify *= digits;
    deviation *= digits;

    deviation *= deviation;
    difx *= difx;
    dify *= dify;
    //cout << difx << " -- " << dify << " -- " << deviation<<endl;/// accuracy control
    if((difx+dify)<=deviation) return true;
    else return false;
}
