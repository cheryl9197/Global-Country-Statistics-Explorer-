// stores (year, value) pairs using dynamically resized arrays
// provides functions to load, modify, analyze, and display time-series data

#include <iostream>
#include <sstream>
#include <fstream>
#include "TimeSeries.hpp"
#include "CountryData.hpp"
#include "Tree.hpp"

// constructor: initialize empty arrays with initial capacity
TimeSeries::TimeSeries () {

    size = 0;
    capacity = 2;
    yearArray = new int [capacity];
    dataArray = new double [capacity];
}

// destructor: free dynamically allocated memory
TimeSeries::~TimeSeries () {

    delete [] yearArray;
    delete [] dataArray;
}

// copy constructor
TimeSeries::TimeSeries(const TimeSeries& other) {

    size = other.size;
    capacity = other.capacity;
    seriesname = other.seriesname;
    seriescode = other.seriescode;

    yearArray = new int[capacity];
    dataArray = new double[capacity];

    for (int i = 0; i < size; ++i) {
        yearArray[i] = other.yearArray[i];
        dataArray[i] = other.dataArray[i];
    }
}

//copy assignment operator
TimeSeries& TimeSeries::operator=(const TimeSeries& other) {

    if (this == &other) {
            return *this;
        }

    delete[] yearArray;
    delete[] dataArray;

    size = other.size;
    capacity = other.capacity;
    seriesname = other.seriesname;
    seriescode = other.seriescode;

    yearArray = new int[capacity];
    dataArray = new double[capacity];

    for (int i = 0; i < size; ++i) {
        yearArray[i] = other.yearArray[i];
        dataArray[i] = other.dataArray[i];
    }

    return *this;
}

// loads time-series data from a csv file into the arrays
// skips invalid data (-1) and resizes arrays as needed
bool TimeSeries::load (std::string filename) {

    std::ifstream inputFile(filename);   // open input csv file
    std::string token; 
    int index = 0;                        // tracks column position in csv
    int year = 1960;                     // starting year for data values
 
    // parse each comma-separated field from the csv
    while (std::getline(inputFile, token, ',')) {

        // data values begin after the first four metadata fields
        if (index > 3) {

            double tokenValue = std::stod(token);

            // ignore missing or invalid data entries
            if (tokenValue != -1) {

                // resize arrays if capacity has been reached
                if (size == capacity) {

                    int* newYearArray = new int [capacity * 2];
                    double* newDataArray = new double [capacity * 2];
                    
                    // copy existing data into larger arrays
                    for (int i = 0; i < size; ++i) {
                        newYearArray[i] = yearArray[i];
                        newDataArray[i] = dataArray[i];
                    }

                    delete[] yearArray;
                    delete[] dataArray;
                    yearArray = newYearArray;
                    dataArray = newDataArray;
                    capacity *= 2;
                }

                // store valid (year, data) pair
                yearArray[size] = year;
                dataArray[size] = tokenValue;
                size++;
            }

            year++;   // increment year for next data column
        }
        else if (index == 2) { 
            // extract series name from csv metadata
            seriesname = token;
        }
        else if (index == 3) { 
            // extract series code from csv metadata
            seriescode = token;
        }

        index++;
    }   

    return true;
}

// prints all stored (year, value) pairs in order
bool TimeSeries::print () {

    // return false if there is no data to print
    if (size == 0) {
        return false;
    }

    // output each data point in (year,value) format
    for (int i = 0; i < size; ++i) {

        std::cout << "(" << yearArray[i] << "," << dataArray[i] << ")"; 
        
        // prevent trailing space after last entry
        if (i != size - 1) {
            std::cout << " ";
        }
    }

    std::cout << std::endl;
    return true;
} 

// adds a new (year, value) pair while maintaining sorted order by year
// returns false if the year already exists
bool TimeSeries::add (int Y, double D) {

    int token = 0;

    // find insertion position and check for duplicate year
    while (token < size && yearArray[token] <= Y) {

        if (yearArray[token] == Y) {
            return false;
        }

        ++token;
    }

    // resize arrays if additional space is required
    if (size + 1 == capacity) {

        int* newYearArray = new int [capacity * 2];
        double* newDataArray = new double [capacity * 2];
                   
        for (int i = 0; i < size; ++i) {
            newYearArray[i] = yearArray[i];
            newDataArray[i] = dataArray[i];
        }

        delete[] yearArray;
        delete[] dataArray;
        yearArray = newYearArray;
        dataArray = newDataArray;
        capacity *= 2;
    }

    // shift elements right to create space for the new entry
    for (int i = size - 1; i >= token; --i) {
        yearArray[i + 1] = yearArray[i];
        dataArray[i + 1] = dataArray[i];
    }

    // insert new (year, data) pair
    yearArray[token] = Y;
    dataArray[token] = D;
    size++;

    return true;
}

// updates the data value for a given year
// if d < 0, the entry is removed from the series
bool TimeSeries::update (int Y, double D) {

    int check = -1;

    // search for the index of the specified year
    for (int i = 0; i < size; ++i) {
        if (yearArray[i] == Y) {
            check = i;
            break;
        }
    }

    // year not found
    if (check == -1) {
        return false;
    }

    // remove entry if data value is invalid
    if (D < 0) {

        // shift elements left to overwrite removed entry
        for (int k = check; k < size - 1; ++k) {
            dataArray[k] = dataArray[k + 1];
            yearArray[k] = yearArray[k + 1];
        }

        size--;

        // shrink arrays if usage is significantly below capacity
        if ((size <= capacity / 4) && capacity >= 4) {

            int* newYearArray = new int [capacity / 2];
            double* newDataArray = new double [capacity / 2];

            for (int i = 0; i < size; ++i) {
                newYearArray[i] = yearArray[i];
                newDataArray[i] = dataArray[i];
            }

            delete[] yearArray;
            delete[] dataArray;
            yearArray = newYearArray;
            dataArray = newDataArray;
            capacity /= 2;
        }
    }
    else {
        // update existing data value
        dataArray[check] = D;
    }

    return true;
}

// computes and returns the mean of all stored data values
// returns -1 if no data exists
double TimeSeries::mean () {

    if (size == 0) {
        return -1;
    }

    double dataSum = 0;

    // accumulate sum of data values
    for (int i = 0; i < size; ++i) {
        dataSum += dataArray[i];
    }
   
    return (dataSum / size);
}

// determines whether the data values are monotonic
// (non-increasing or non-decreasing)
bool TimeSeries::is_monotonic () {

    int index = 0;

    // skip over initial equal values
    while ((index < size - 1) && (dataArray[index] == dataArray[index + 1])) {
        ++index;
    }

    // all values are equal
    if (index == size - 1) {
        return true;
    }

    // check for monotonic decreasing
    if (dataArray[index] > dataArray[index + 1]) {

        for (int i = index; i < size - 1; ++i) {
            if (dataArray[i] < dataArray[i + 1]) {
                return false;
            }
        }

        return true;
    }

    // check for monotonic increasing
    if (dataArray[index] < dataArray[index + 1]) {

        for (int i = index; i < size - 1; ++i) {
            if (dataArray[i] > dataArray[i + 1]) {
                return false;
            }
        }

        return true;
    }

    return true;
}

// computes the line of best fit (least squares regression)
// outputs slope (m) and intercept (b)
bool TimeSeries::best_fit (double &m, double &b) {

    if (size < 2) {
        m = 0;
        b = 0;
        return false;
    }

    double xySum = 0.0;
    double xSum  = 0.0;
    double ySum  = 0.0;
    double x2Sum = 0.0;

    // accumulate summation terms for regression formula
    for (int i = 0; i < size; ++i) {
        xySum += yearArray[i] * dataArray[i];
        xSum  += yearArray[i];
        ySum  += dataArray[i];
        x2Sum += yearArray[i] * yearArray[i];
    }

    m = (size * xySum - xSum * ySum) / (size * x2Sum - xSum * xSum);
    b = (ySum - m * xSum) / size;
    
    return true;
}

// returns the number of stored data points
int TimeSeries::getsize () {

    return size;
}

//return series code for use in country data
std::string TimeSeries::getCode() const {

    return seriescode;
}

//return capacity of arrays
int TimeSeries::getcapacity () {

    return capacity;
}

//return series name
std::string TimeSeries::getSeriesName () {

    return seriesname;
}

//set series name
void TimeSeries::setSeriesName (std::string name) {

    seriesname = name;

}

//set series code
void TimeSeries::setSeriesCode (std::string code) {

    seriescode = code;
}