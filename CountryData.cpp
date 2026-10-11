#include <iostream>
#include <fstream>
#include <sstream>
#include "TimeSeries.hpp"
#include "CountryData.hpp"
#include "Tree.hpp"

// constructor: initialize empty array with initial capacity
// allows dynamic resizing as more time series are added
// sets initial size to 0
CountryData::CountryData() {
    size = 0;
    capacity = 2;
    seriesArray = new TimeSeries[capacity];
}

// destructor: free dynamically allocated memory to prevent leaks
// ensures array is properly deleted
CountryData::~CountryData() {

    delete[] seriesArray;

}

// load all time series for a given country from CSV file
// ignores rows that do not match the requested country
// skips invalid/missing data (negative values) and dynamically expands array if needed
bool CountryData::load_p2 (std::string countryname) {

    std::ifstream inputFile("lab2_multidata.csv");   // open CSV containing all countries
    std::string line;

    //tracks each line
    while (std::getline(inputFile, line)) {

        TimeSeries ts;
        std::stringstream ss(line);
        std::string token;
        int index = 0;
        int year = 1960;
        bool match = true;  // track if this row belongs to requested country

        //tracks each token
        while (std::getline(ss, token, ',')) {

            if (index == 0) {

                if (token != countryname) {

                    match = false;  // skip this row if country does not match
                    break;
                } else {

                    this->countryname = token; // store country name for reference
                }

            } else if (index == 1) {

                this->countrycode = token; // store country code

            } else if (index == 2) {

                ts.setSeriesName(token); // store series name in the ts

            } else if (index == 3) {

                ts.setSeriesCode(token); // store series code in the ts

            } else if (index > 3) {

                double value = std::stod(token);

                if (value >=0) { // skip missing/invalid data

                    ts.add(year, value); // add valid yearly data
                }
                year++;
            }
            index++;
        }

        if (match == true && index >=4) {
            if (size == capacity) {

                TimeSeries* newSeriesArray = new TimeSeries [capacity * 2]; // expand array if full
                
                for (int i = 0; i < size; ++i) {

                    newSeriesArray[i] = seriesArray[i]; // copy old series
                }

                delete[] seriesArray; // free old array
                seriesArray = newSeriesArray;
                capacity *= 2; // update capacity
            }

            seriesArray[size] = ts; // store new series
            size++;
        }
        
    }
    return true; // always returns true; unmatched rows are ignored
}


// list all series names for the current country
// prints country name, code, and all series names
// useful for quickly checking which data is loaded
bool CountryData::list_p2 () {

    std::cout << countryname << " " << countrycode << " ";
    
        for (int k = 0; k < size; ++k) { 

            std::cout << seriesArray[k].getSeriesName() << " ";
        }

    std::cout << std::endl;
    return true;
}

// add a data point to a series
// fails if series code does not exist
// ensures data integrity by checking series exists before adding
bool CountryData::add_p2 (std::string seriescode, int Y, double D) {

    int seriesIndex = getSeriesIndex(seriescode);

    if (seriesIndex == -1) { //check for O(1) case

        return false;
    } 

    return seriesArray[seriesIndex].add(Y, D); 
}

// update a data point in a series
// fails if series or year does not exist
// ensures only existing data points are modified
bool CountryData::update_p2 (std::string seriescode, int Y, double D) {

    int seriesIndex = getSeriesIndex(seriescode);

    if (seriesIndex == -1) { //check for O(1) case

        return false;
    }

    return seriesArray[seriesIndex].update(Y, D);
}

// print a series to console
// fails if series code does not exist
// outputs all years and values for that series
bool CountryData::print_p2 (std::string seriescode) {

    int seriesIndex = getSeriesIndex(seriescode);

    if (seriesIndex == -1) { //check for O(1) case

        return false;
    }

    return seriesArray[seriesIndex].print();
}

// delete a series from the country data
// fails if series code does not exist
// shifts remaining series down to avoid gaps
bool CountryData::delete_p2 (std::string seriescode) {

    int seriesIndex = getSeriesIndex(seriescode);

    if (seriesIndex == -1) { //check for O(1) case
        
        return false;
    }

    for (int i = seriesIndex; i < size - 1; ++i) { 

        seriesArray[i] = seriesArray[i+1]; //shift by 1 from series index to be deleted
    }

    size--;
    return true;
}

// find and print the series with the largest mean
// ignores series with no valid data
// fails if no series has valid data
bool CountryData::biggest_p2 () {  

    double biggestMean = -1;
    int biggestIndex = -1;

    for (int i = 0; i < size - 1; ++i) {

        double m = seriesArray[i].mean();

        if (m >= 0) { // ignore series with invalid data

            if (biggestIndex == -1 || m > biggestMean) { //tracks biggest mean and index

                biggestMean = m;
                biggestIndex = i;
            }
        }
    }

    if (biggestIndex == -1) {

        return false;
    }

    std::cout << seriesArray[biggestIndex].getCode() << std::endl;
    return true;
}

// display size and capacity of a series
// fails if series code does not exist
// useful for debugging internal array state
bool CountryData::ts_p2 (std::string seriescode) {

    int seriesIndex = getSeriesIndex(seriescode);

    if (seriesIndex == -1) {
        
        return false;
    } 
    
    int S = seriesArray[seriesIndex].getsize();
    int C = seriesArray[seriesIndex].getcapacity();
    
    std::cout << "size is " << S << " capacity is " << C << std::endl;
    return true;
}

// get index of series by series code
// returns -1 if series does not exist
// used internally by other functions to locate series
int CountryData::getSeriesIndex (std::string seriescode) const {

    int index = 0;

    for (index; index < size; ++index) {

        if (seriesArray[index].getCode() == seriescode) { 

            return index;
        }
    }

    return -1;
} 

std::string CountryData::getCountryName() const {
    return countryname;
}

std::string CountryData::getCountryCode() const {
    return countrycode;
}

int CountryData::getSeriesCount() const {
    return size;
}

TimeSeries CountryData::getSeries(int i) const {
    return seriesArray[i];
}

double CountryData::getSeriesMean(int index) const {
    return seriesArray[index].mean();
}

bool CountryData::addSeriesFromLine (std::string line) {

    TimeSeries ts;
    std::stringstream ss(line);
    std::string token;
    int index = 0;
    int year = 1960;

    //tracks each token
    while (std::getline(ss, token, ',')) {

        if (index == 0) {

                this->countryname = token; // store country name for reference

        } else if (index == 1) {

            this->countrycode = token; // store country code

        } else if (index == 2) {

            ts.setSeriesName(token); // store series name in the ts

        } else if (index == 3) {

            ts.setSeriesCode(token); // store series code in the ts

        } else if (index > 3) {

            double value = std::stod(token);

            if (value >=0) { // skip missing/invalid data

                ts.add(year, value); // add valid yearly data
            }
            year++;
        }
        index++;
    }

    if (index >=4) {
        if (size == capacity) {

            TimeSeries* newSeriesArray = new TimeSeries [capacity * 2]; // expand array if full
            
            for (int i = 0; i < size; ++i) {

                newSeriesArray[i] = seriesArray[i]; // copy old series
            }

            delete[] seriesArray; // free old array
            seriesArray = newSeriesArray;
            capacity *= 2; // update capacity
        }

        seriesArray[size] = ts; // store new series
        size++;
    }

    return true; // always returns true; unmatched rows are ignored
}