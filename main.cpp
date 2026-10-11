#include <iostream>
#include <fstream>
#include <sstream>
#include "TimeSeries.hpp" 
#include "CountryData.hpp"
#include "Tree.hpp"
#include "HashTable.hpp"
#include "Graph.hpp"

// reads every line of the csv and groups rows by country name into the countries array
// instead of loading one country at a time, this builds all countries at once
// also populates the hash table after loading so LOOKUP_P4 works immediately after LOAD_P3
void load_p3 (CountryData countries[], int& countryCount, HashTable& hashTable) {

    std::ifstream inputFile("lab2_multidata.csv");   // open CSV containing all countries
    std::string line;

    while (std::getline(inputFile, line)) {

        std::stringstream ss(line);
        std::string countryName;

        if (std::getline(ss, countryName, ',')) {

            int countryIndex = -1;

            // search existing countries to see if this country has already been seen
            // this is necessary because multiple csv rows belong to the same country
            for (int i = 0; i < countryCount; ++i) {

                if (countries[i].getCountryName() == countryName) {

                    countryIndex = i;
                    break;
                }
            }

            // if country not found, assign it the next available slot
            // countryCount acts as both the size and the next insertion index
            if (countryIndex == -1) {

                countryIndex = countryCount;
                countryCount++;
            }
            
            countries[countryIndex].addSeriesFromLine(line);
        }
    }

    // once all countries are built, insert each one into the hash table
    // this is done after the loop so every country is fully populated before being hashed
    for (int i = 0; i < countryCount; i++) {

        hashTable.insertHelper(&countries[i]);
    }

    std::cout << "success" << std::endl;
    return;
}

// searches for a country by name and prints its name, code, and all series names
// using a reference to avoid copying the entire CountryData object during output
bool list_p3 (CountryData countries[], int countryCount, std::string countryname) {

    int index = -1;

    for (int i = 0; i < countryCount; ++i) {

        if (countries[i].getCountryName() == countryname) {

            index = i;
            break;
        }
    }

    if (index == -1) {
        
        return false;
    }
      
    // use a reference to avoid copying the entire CountryData object just for printing
    CountryData &c = countries[index]; 

    std::cout << c.getCountryName() << " " << c.getCountryCode() << " ";

    int seriesCount = c.getSeriesCount();
    
    for (int j = 0; j < seriesCount; ++j) {

        std::cout << c.getSeries(j).getSeriesName() << " ";
    }

    std::cout << std::endl;
    return true; 
}

// finds the series with the smallest mean for a given country code
// skips series with invalid means (-1) 
bool country_min_p3 (CountryData countries[], int countryCount, std::string countrycode) {

    int index = -1;

    for (int i = 0; i < countryCount; ++i) {

        if (countries[i].getCountryCode() == countrycode) {

            index = i;
            break;
        }
    }

    if (index == -1) {

        return false;
    }

    CountryData& c = countries[index];
    int seriesCount = c.getSeriesCount();

    int minIndex = -1;
    double minMean;

    // find the first valid series to use as the initial minimum
    // this avoids initializing minMean to an arbitrary value that could incorrectly win
    for (int j = 0; j < seriesCount; ++j) {

        double m = c.getSeriesMean(j);

        if (m != -1) {

            minIndex = j;
            minMean = m;
            break;
        }
    }

    if (minIndex == -1) {

        return false;
    }

    // start from minIndex + 1 since we already used that as our baseline
    for (int k = minIndex + 1; k < seriesCount; ++k) {

        double m = c.getSeriesMean(k);

        if (m != -1 && m < minMean) {

            minMean = m;
            minIndex = k;
        }
    }

    std::cout << c.getSeries(minIndex).getCode() << std::endl;
    return true;
}

// scans all countries for a given series code and prints the min and max mean across all of them
// uses an initialized flag instead of a sentinel value to safely handle the first valid entry
// without assuming any particular value is a safe starting minimum or maximum
void range_p3 (CountryData countries[], int countryCount, std::string seriescode) {

    bool initialized = false;
    double minMean;
    double maxMean;

    for (int i = 0; i < countryCount; ++i) {

        int seriesIndex = countries[i].getSeriesIndex(seriescode);

        if (seriesIndex != -1) {

            double m = countries[i].getSeriesMean(seriesIndex);

            if (m != -1) {

                // use the first valid mean to initialize both min and max
                // this avoids hardcoding a starting value that might not exist in the data
                if (!initialized) {
                
                    minMean = m;
                    maxMean = m;
                    initialized = true;
                } else {

                    if (m < minMean) {

                        minMean = m;
                    } 

                    if (m > maxMean) {

                        maxMean = m;
                    }
                }
            }
        }
    }

    // only print if at least one valid mean was found
    // if no country had this series code or all means were invalid, report failure
    if (!initialized) {

        std::cout << "failure" << std::endl;
        return;
    }

    std::cout << minMean << " " << maxMean << std::endl;
    return;
}

// helper function to find a country's index by name
// used in a few places where we need the array position but only have the name
int findCountryIndex(CountryData countries[], int countryCount, std::string countryname) {

    for (int i = 0; i < countryCount; ++i) {

        if (countries[i].getCountryName() == countryname) {

            return i;
        }
    }
    return -1;
}

// main loop reads commands and directs to the appropriate function
// the tree is heap allocated so it can be created on demand after a build command
// and cleaned up properly at exit to avoid memory leaks
int main () {

    CountryData countries[512];  // fixed size array large enough to hold all countries in the dataset
    int countryCount = 0;
    Tree* tree = nullptr;
    HashTable hashTable;  // no need to heap allocate, lives for the whole program
    Graph* graph = nullptr; // heap allocated on first INITIALIZE_P5 so it has access to the fully loaded countries array

    std::string line;

    while (std::getline(std::cin, line)) {

        std::stringstream ss(line);
        std::string command;
        ss >> command;

        if (command == "LOAD_P3") {

            // pass hashTable so load_p3 can populate it alongside the countries array
            load_p3(countries, countryCount, hashTable);

        } else if (command == "LIST_P3") {

            std::string countryname;
            std::getline(ss, countryname);
            countryname.erase(0, 1);
            list_p3(countries, countryCount, countryname);

        } else if (command == "COUNTRY_MIN_P3") {

            std::string countrycode;
            ss >> countrycode;
            country_min_p3(countries, countryCount, countrycode);

        } else if (command == "RANGE_P3") {

            std::string seriescode;
            ss >> seriescode;
            range_p3(countries, countryCount, seriescode);

        } else if (command == "BUILD_P3") {

            std::string seriescode;
            ss >> seriescode;

            // only allocate the tree once; subsequent build calls reuse the same object
            // the tree internally handles rebuilding when build is called again
            if (tree == nullptr) {

                tree = new Tree();
            }
            
            tree->build_p3(countries, countryCount, seriescode);

            std::cout << "success" << std::endl;

        } else if (command == "FIND_P3") {

            double mean;
            std::string operation;
            ss >> mean >> operation;

            // guard against calling find before a tree has been built
            if (tree != nullptr) {

                tree->find_p3(countries, mean, operation);
            } else {

                std::cout << "failure" << std::endl;
            }
            
        } else if (command == "DELETE_P3") {

            std::string countryname;
            std::getline(ss, countryname);
            countryname.erase(0, 1);

            // guard against calling delete before a tree has been built
            if (tree != nullptr) {

                tree->delete_p3(countries, countryname);
            } else {

                std::cout << "failure" << std::endl;
            }

        } else if (command == "LIMITS_P3") {

            std::string condition;
            ss >> condition;

            // guard against calling limits before a tree has been built
            if (tree != nullptr) {

                tree->limits_p3(countries, condition);
            } else {

                std::cout << "failure" << std::endl;
            }

        } else if (command == "TRACE_P3") {

            std::string countryname;
            std::getline(ss, countryname);
            countryname.erase(0, 1);

            // guard against calling trace before a tree has been built
            if (tree != nullptr) {

                tree->trace_p3(countries, countryname);
            } else {

                std::cout << "failure" << std::endl;
            }

        } else if (command == "LOOKUP_P4") {

            std::string country_code;
            ss >> country_code;
            int index, steps;

            // index and steps are set by lookup and printed only on success
            if (hashTable.lookup_p4(country_code, index, steps)) {

                std::cout << "index " << index << " searches " << steps << std::endl;
            } else {

                std::cout << "failure" << std::endl;
            }

        } else if (command == "REMOVE_P4") {

            std::string country_code;
            ss >> country_code;

            // get the name before removing since it's needed for tree deletion
            std::string country_name = "";

            for (int i = 0; i < countryCount; i++) {

                if (countries[i].getCountryCode() == country_code) {

                    country_name = countries[i].getCountryName();
                    break;
                }
            }

            if (hashTable.remove_p4(country_code)) {

                // use silent_delete so delete_p3 doesn't print its own success/failure
                // remove_p4 is the one responsible for output here
                if (tree != nullptr && country_name != "") {

                    tree->silent_delete(countries, country_name);
                }

                std::cout << "success" << std::endl;
            } else {

                std::cout << "failure" << std::endl;
            }
            
        } else if (command == "INSERT_P4") {

            std::string country_code;
            ss >> country_code;

            // country must already exist in countries[] from load_p3 to be insertable
            if (hashTable.insert_p4(country_code, countries, countryCount)) {

                std::cout << "success" << std::endl;
            } else {

                std::cout << "failure" << std::endl;
            }

        } else if (command == "CLEAN_P4") {

            // rehashes the table to remove tombstones and restore lookup efficiency
            hashTable.clean_p4();
            std::cout << "success" << std::endl;

        // new project 5 commands:
        } else if (command == "INITIALIZE_P5") {

            // only allocate the graph once, subsequent calls reuse the same object
            // initialize() handles wiping existing edges so no need to reallocate
            if (graph == nullptr) {

                graph = new Graph(countries, countryCount);
            }

            graph->initialize();
            std::cout << "success" << std::endl;

        } else if (command == "UPDATE_EDGES_P5") {

            std::string seriesCode;
            double threshold;
            std::string relation;
            ss >> seriesCode >> threshold >> relation;

            // updateEdges returns false if no new relationships were added
            // this covers both the case where no countries qualify and where all tuples already exist
            if (graph->updateEdges(seriesCode, threshold, relation)) {

                std::cout << "success" << std::endl;
            } else {

                std::cout << "failure" << std::endl;
            }

        } else if (command == "ADJACENT_P5") {

            std::string countryCode;
            ss >> countryCode;
            graph->adjacent(countryCode);

        } else if (command == "PATH_P5") {

            std::string code1, code2;
            ss >> code1 >> code2;

            if (graph->path(code1, code2)) {

                std::cout << "true" << std::endl;
            } else {

                std::cout << "false" << std::endl;
            }

        } else if (command == "RELATIONSHIPS_P5") {

            std::string code1, code2;
            ss >> code1 >> code2;
            graph->relationships(code1, code2);

        } else if (command == "EXIT") {

            break;
        }
    } 

    delete tree;
    delete graph;  // graph is heap allocated so it must be explicitly deleted to avoid memory leaks
    return 0;
}