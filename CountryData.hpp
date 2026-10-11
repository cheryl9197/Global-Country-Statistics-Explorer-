
#ifndef COUNTRYDATA_HPP
#define COUNTRYDATA_HPP
#include <iostream>
#include <string>
#include "TimeSeries.hpp"

class CountryData {

    private:
        std::string countryname;
        std::string countrycode;
        TimeSeries* seriesArray; 
        int size;
        int capacity;
        //helper function
//        int getSeriesIndex (std::string seriescode);

    public:
        CountryData (); //constructor
        ~CountryData (); //destructor
        //required functions
        bool load_p2 (std::string countryname);
        bool list_p2 ();
        bool print_p2 (std::string seriescode);
        bool add_p2 (std::string seriescode, int Y, double D);
        bool update_p2 (std::string seriescode, int Y, double D);
        bool delete_p2 (std::string seriescode);
        bool biggest_p2 (); 
        bool ts_p2 (std::string seriescode);

        std::string getCountryName() const;
        std::string getCountryCode() const;
        int getSeriesCount() const;
        TimeSeries getSeries(int i) const;
        double getSeriesMean(int index) const;
        int getSeriesIndex(std::string seriescode) const;
        bool addSeriesFromLine(std::string line);
};

#endif