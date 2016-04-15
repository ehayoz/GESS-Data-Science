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
const int stdarlen = 14; /// length of normal Datasets, maxarlen to store additional Data
const int maxarlen = 16; /// max array length take care to not write more data
const int tmaxc = 3; ///max text columns
const int quelen = 14; ///number of question + 1 for line in which it first occured
const int nquest = 14; ///number of question sets per DataSet


const int gpspos = 11; ///Position of the Latitude column
const int range = 50; ///conGPSdep Data Lines to be checked before and after actual point
const double precision = 3; ///conGPSdep Prcision radius
const int digits = 1;///number of GPSdigits as 10 to the power of digits


struct Line ///A[Timestamp],B[HR],C[BVP],D[EDA],E[TempBF],F[Sound],G[Dust],H[TempEN],I[RH],J[Light],K[#WiFi],L[LON],M[LAT],N[Greenery], O*moodcoefficient, P*stresslevel
{
    double data[maxarlen]; /// set max column size;
};

class DataSet
{
public:
    struct tLine ///Input file names
    {
        string data[tmaxc];
    };

    tLine* inputnames; ///contains text lines
    Line* Startzeile; /// contains pointer to first line of DataSet
    Line* Fragen; /// contains pointer to first QuestionSet [S No.],[Q1],[Q2],[Q3],[Q4],[Q5],[Q6],[Q7],[Q8],[Q9],[Q10],[Q11],[Q12]
    int Zeilenzahl; /// contains #lines from Startzeile
    int Spaltenzahl; /// contains number of initialized columns, important to write to file
    double stdev[maxarlen]; /// contains stdev of specified row
    double mean[maxarlen]; /// contains mean of specified row


    void printData(void);
    void readfile(string filename, int columns);
    void writeDtoF(string filename);

    void copDat(const DataSet Sample);
    DataSet operator+(const DataSet Sample) const;
    DataSet operator*(const double num) const;

    void normalize(void);
    void moodcoefficient(void);
    DataSet conGPSdep(const DataSet master) const;

    long double getsum(const int column) const;
    double getmean(const int column) const;
    double getstdev(const int column) const;
    //bool chkeql(double Data1, double Data2, double deviation);
    //bool chkmap(Line Sample1, Line Sample2, double deviation);


};

double stod(string s);

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

    DataSet target;
    target.readfile(inputs, 2); ///getting input target files, needs to be read as string. Maybe target names as well.

    DataSet* Set = new DataSet[target.Zeilenzahl]; ///Creating #Dataset pointers
    DataSet* GPSset = new DataSet[target.Zeilenzahl];

    for(int i = 0; i < target.Zeilenzahl; ++i)
    {
        Set[i].readfile(target.inputnames[i].data[0], stdarlen); /// reading file
    }

    ///Analyzation start

    for(int i = 0; i< target.Zeilenzahl; ++i) /// for every file itself
    {
        Set[i].moodcoefficient(); ///moodcoefficient
        Set[i].normalize();
        GPSset[i] = Set[i].conGPSdep(Set[0]);
    }

    //getsum(Set[1].Startzeile, Set[1].Zeilenzahl, 2);



    ///Analyzation end



    for(int i = 0; i < target.Zeilenzahl; ++i)
    {
        //Set[i].printData(); /// printing on the cmd window
        Set[i].writeDtoF(target.inputnames[i].data[1]); /// writing comma-separated to file
        GPSset[i].writeDtoF(target.inputnames[i].data[2]); /// writing comma-separated to file
    }




    /// delete arrays
    /*for(int i = 0; i < target.Zeilenzahl; ++i)
    {
        delete Set[i].Startzeile; /// deleting Pointer, has to be done for every Dataset.
        delete Set[i].inputnames;
        delete Set[i].Fragen;
        delete GPSset[i].Startzeile; /// deleting Pointer, has to be done for every Dataset.
        delete GPSset[i].inputnames;
        delete GPSset[i].Fragen;
    }*/
    delete[] Set;
    delete[] GPSset;

    t2=clock();
    float diff ((float)t2-(float)t1);
    cout << endl << 2*target.Zeilenzahl << " files written -- time needed: " << diff/1000 << " s";
    pauseoutput();
    return 0;
}


///Write/Read functions

void DataSet::readfile(string filename, int columns)
{
    ///Open file
    ifstream fin;
    fin.open(filename);
    string line;
    string dump;
    this->Spaltenzahl = columns;

    /// get #line and reset the getline position
    int filelength;
    for (filelength = 0; std::getline(fin, line); filelength++)
        ;
    this->Zeilenzahl = filelength;
    fin.clear();
    fin.seekg(0, ios::beg);

    /// create Line array of size filelength

    Line* Zeile = new Line[filelength]; /// for data
    Line* qZeile = new Line[nquest]; /// for cellphone questions
    tLine* tZeile = new tLine[filelength]; /// for input files

    this->inputnames = tZeile;
    this->Startzeile = Zeile;
    this->Fragen = qZeile;

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
}

void DataSet::printData(void)
{
    system("color A");
    ostringstream strs;
    for(int i = 0; i < this->Zeilenzahl; ++i)
    {
        for(int j = 0; j<this->Spaltenzahl; ++j)
        {
            strs << setprecision(7) << this->Startzeile[i].data[j];
            if(j < this->Spaltenzahl-1) strs << ",";
        }
        strs << endl;
    }

    strs << endl << "===========Questions===========\n\n";

    for(int i = 0; i < nquest; ++i)
    {
        for(int j = 0; j<quelen; ++j)
        {
            strs << setprecision(7) << this->Fragen[i].data[j];
            if(j < quelen-1) strs << ",";
        }
        strs << endl;
    }

    string str = strs.str();
    cout << str;
}

void DataSet::writeDtoF(string filename)
{
    system("color A");
    ostringstream strs;
    for(int i = 0; i < this->Zeilenzahl; ++i)
    {
        for(int j = 0; j<this->Spaltenzahl; ++j)
        {
            strs << setprecision(11) << this->Startzeile[i].data[j];
            if(j < this->Spaltenzahl-1) strs << ",";
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

    cout << "File " << filename <<" successfully closed! -- " << this->Zeilenzahl << " Lines written!\n";
}

void DataSet::copDat(const DataSet Sample) /// copy datasets
{
    //DataSet result;
    this->Fragen = new Line[nquest]; /// for cellphone questions
    this->Startzeile = new Line[Sample.Zeilenzahl]; /// for data
    this->inputnames = new tLine[Sample.Zeilenzahl]; /// for input files
    this->Zeilenzahl = Sample.Zeilenzahl;
    this->Spaltenzahl = Sample.Spaltenzahl;

    for(int i = 0; i< Sample.Spaltenzahl; ++i)
    {
        this->mean[i] = Sample.mean[i];
        this->stdev[i] = Sample.stdev[i];
    }


    for(int i = 0; i< Sample.Zeilenzahl; ++i)
    {
        for(int j = 0; j< Sample.Spaltenzahl; ++j)
        {
            this->Startzeile[i].data[j] = Sample.Startzeile[i].data[j];
        }
        for(int j = 0; j< tmaxc; ++j)
        {
            this->inputnames[i].data[j] = Sample.inputnames[i].data[j];
        }
    }
    for(int i = 0; i< nquest; i++)
    {
        for(int j = 0; j< quelen; j++)
        {
            this->Fragen[i].data[j] = Sample.Fragen[i].data[j]; /// for cellphone questions
        }
    }
}

DataSet DataSet::operator+(const DataSet Sample2) const/// add up to Datasets stdev and mean are lost equal spalten and zeilen zahl requiered
{
    DataSet result;
    result.copDat(*this);
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

DataSet DataSet::operator*(const double num) const /// divide Dataset by double stdev and mean are lost
{
    DataSet result;
    result.copDat(*this);

    for(int i = 0; i< result.Spaltenzahl; ++i)
    {
        result.mean[i] = 0;
        result.stdev[i] = 0;
    }


    for(int i = 0; i< result.Zeilenzahl; ++i)
    {
        for(int j = 0; j< result.Spaltenzahl; ++j)
        {
            result.Startzeile[i].data[j] *= num;
        }
    }
    for(int i = 0; i< nquest; i++)
    {
        for(int j = 0; j< quelen; j++)
        {
            result.Fragen[i].data[j] *= num; /// for cellphone questions
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

void DataSet::moodcoefficient(void) /// calculate mood
{
    for(int i = 0; i<quelen; ++i) this->Fragen[i].data[quelen] = 0;

    for(int i = 0; i<nquest ; ++i) ///verrechnung der Fragen zu moodcoefficient
    {
        for(int j = 3; j<quelen; ++j)
        {
            this->Fragen[i].data[quelen] += this->Fragen[i].data[j];
        }
        this->Fragen[i].data[quelen] /= (quelen-2);
    }
    for(int i = 0; i < nquest; ++i) ///schreiben der modcoefficients, type Euler forward
    {
        for(int j = this->Fragen[i].data[0]; j < this->Zeilenzahl; ++j)
        {
            if(i > 0 && this->Fragen[i].data[0] == 0) break;
            this->Startzeile[j].data[stdarlen] = this->Fragen[i].data[quelen];
            //cout << this->Startzeile[j].data[stdarlen] <<endl;
        }

    }

    this->Spaltenzahl = stdarlen + 1; ///mark additional column
}

void DataSet::normalize(void) ///normalize DataSets
{
    for(int i = 1; i< this->Spaltenzahl; ++i)
    {
        this->mean[i] = this->getmean(i);
        this->stdev[i] = this->getstdev(i);
        for(int j = 0; j < this->Zeilenzahl; ++j)
        {
            if(i != gpspos && i != (gpspos+1)) this->Startzeile[j].data[i] /= this->mean[i];
        }
    }
}

DataSet DataSet::conGPSdep(const DataSet master) const /// convert Time ordered Data to location ordered Data
{
    DataSet gpsordered;// = master;
    gpsordered.copDat(master);
    //writeDtoF(gpsordered, "test.csv");
    //int maxdiff = 0;

    for(int i = 0; i<gpsordered.Zeilenzahl; ++i)
    {
        int mini = i+range;
        int maxi = 0;

        for(int j = i-range; j<this->Zeilenzahl; ++j)
        {
            if(j<0) j = 0; ///protection from array underflow
            if((j-i)> range) break;
            if(chkmap(gpsordered.Startzeile[i], this->Startzeile[j], precision))
            {
                if(mini > j) mini = j; ///first occurence of true
                maxi = j; ///last occurence of true
            }
        }
        //maxdiff = maxi-mini;
        //cout << mini << "    " << maxi << "     " << maxdiff +1 <<endl; ///clue on accuracy

        ///evaluaktion from mini to max
        for(int j = 0; j< gpsordered.Spaltenzahl; ++j)
        {
            long double meandump = 0;
            if(j != gpspos && j != (gpspos+1))///To keep GPS Mastercoordinates
            {
                for(int k = mini; k <= maxi; ++k)
                {
                    meandump += this->Startzeile[k].data[j];
                }
                meandump /= (maxi-mini+1);
                gpsordered.Startzeile[i].data[j] = meandump;
            }
        }
    }
    //writeDtoF(gpsordered, "test.csv");
    return gpsordered;
}



///Math functions

long double DataSet::getsum(const int column) const /// give column to sum up
{
    long double sum = 0;
    for(int i = 0; i < this->Zeilenzahl; ++i)
    {
        sum += this->Startzeile[i].data[column];
    }
    return sum;
}

double DataSet::getmean(const int column) const /// give linepointer, number of lines and column to derivate the mean
{
    long double sum = this->getsum(column);
    sum /= (double) this->Zeilenzahl;
    return sum;
}

double DataSet::getstdev(const int column) const /// give linepointer, number of lines and column to derivate the standard deviation
{
    double mean = this->getmean(column);
    long double sqsum = 0;
    for(int i = 0; i< this->Zeilenzahl; ++i) sqsum += pow((this->Startzeile[i].data[column]-mean),2);
    sqsum /= (double) this->Zeilenzahl;
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
