#include <iostream>
#include <fstream>
#include <sstream>
#include "HashTable.hpp"

// constructor
HashTable::HashTable() {

    for (int i = 0; i < TABLE_SIZE; i++) {

        table[i] = nullptr;
        status[i] = 0;
    }
}

// destructor
HashTable::~HashTable() {

    for (int i = 0; i < TABLE_SIZE; i++) {

        table[i] = nullptr;
    }
}

// converts a three-letter country code into a base-10 number
// treating each letter as a base-26 digit starting at A = 0
int HashTable::computeW(const std::string& code) {

    int d0 = code[0] - 'A';
    int d1 = code[1] - 'A';
    int d2 = code[2] - 'A';
    return d0 * 676 + d1 * 26 + d2;  // 26^2 = 676
}

// primary hash function, maps w to an index in the table
int HashTable::h1(int w) {

    return w % TABLE_SIZE;
}

// secondary hash function, used to compute the step size when a collision occurs
// result is always forced to be odd so it stays coprime with TABLE_SIZE (512)
// this guarantees the probe sequence visits every slot before repeating
int HashTable::h2(int w) {

    int val = (w / TABLE_SIZE) % TABLE_SIZE;

    if (val % 2 == 0) {

        return val + 1;
    }
    return val;
}

// searches for a country code using double hashing
// stops early if an empty slot is found since the country could never have jumped past it
// tombstones are skipped since a deletion there doesn't mean the chain ended
bool HashTable::lookup_p4(const std::string& country_code, int& index, int& steps) {

    int w = computeW(country_code);
    int primary = h1(w);
    int step = h2(w);
    steps = 0;

    for (int i = 0; i < TABLE_SIZE; i++) {
        
        int idx = (primary + i * step) % TABLE_SIZE;
        steps++;

        // if slot is empty, country is not in the table
        if (status[idx] == 0) {

            return false;
        }

        // if slot is occupied and matches, we found it
        if (status[idx] == 1 && table[idx]->getCountryCode() == country_code) {

            index = idx;
            return true;
        }

        // if slot is tombstone or occupied by different country, keep probing
    }

    return false;
}

// marks the slot as a tombstone rather than empty so probing chains stay intact
// setting the pointer to nullptr avoids dangling references to the removed country
bool HashTable::remove_p4(const std::string& country_code) {

    int w = computeW(country_code);
    int primary = h1(w);
    int step = h2(w);

    for (int i = 0; i < TABLE_SIZE; i++) {

        int indx = (primary + i * step) % TABLE_SIZE;

        // empty slot means country was never here
        if (status[indx] == 0) {

            return false;
        }

        // found it, mark as tombstone
        if (status[indx] == 1 && table[indx]->getCountryCode() == country_code) {

            status[indx] = 2;
            table[indx] = nullptr;
            return true;
        }

        // tombstone or different country, keep probing
    }
    return false;
}

// inserts a country into the hash table if it isn't already there
// relies on the country already existing in countries[] from load_p3
// tombstone slots are reused since they count as available space
bool HashTable::insert_p4(const std::string& country_code, CountryData countries[], int& countryCount) {

    // bail early if the country is already in the table
    int dummyIndex, dummySteps;

    if (lookup_p4(country_code, dummyIndex, dummySteps)) {

        return false;
    }

    // find the country in the countries array loaded by load_p3
    // if it doesn't exist there at all, there's nothing to insert
    int countryIndex = -1;

    for (int i = 0; i < countryCount; i++) {

        if (countries[i].getCountryCode() == country_code) {

            countryIndex = i;
            break;
        }
    }

    if (countryIndex == -1) {

        return false;
    }

    // probe for the first empty or tombstone slot
    int w = computeW(country_code);
    int primary = h1(w);
    int step = h2(w);

    for (int i = 0; i < TABLE_SIZE; i++) {

        int indx = (primary + i * step) % TABLE_SIZE;

        if (status[indx] == 0 || status[indx] == 2) {

            table[indx] = &countries[countryIndex];
            status[indx] = 1;
            return true;
        }
    }

    return false;
}

// removes all tombstones by collecting active entries, sorting them, and reinserting
// sorting alphabetically before reinsertion ensures everyone gets the same indices
// which is necessary for the output to be deterministic and gradeable
void HashTable::clean_p4() {

    // step 1: collect all currently occupied countries
    CountryData* temp[TABLE_SIZE];
    int tempCount = 0;

    for (int i = 0; i < TABLE_SIZE; i++) {

        if (status[i] == 1) {

            temp[tempCount] = table[i];
            tempCount++;
        }
    }

    // step 2: sort alphabetically by country code (bubble sort)
    for (int i = 0; i < tempCount - 1; i++) {

        for (int j = 0; j < tempCount - i - 1; j++) {

            if (temp[j]->getCountryCode() > temp[j+1]->getCountryCode()) {

                CountryData* swap = temp[j];
                temp[j] = temp[j+1];
                temp[j+1] = swap;
            }
        }
    }

    // step 3: clear the entire hash table
    for (int i = 0; i < TABLE_SIZE; i++) {

        table[i] = nullptr;
        status[i] = 0;
    }

    // step 4: reinsert in alphabetical order so indices are consistent
    for (int i = 0; i < tempCount; i++) {

        int w = computeW(temp[i]->getCountryCode());
        int primary = h1(w);
        int step = h2(w);

        for (int j = 0; j < TABLE_SIZE; j++) {

            int indx = (primary + j * step) % TABLE_SIZE;

            if (status[indx] == 0) {
                
                table[indx] = temp[i];
                status[indx] = 1;
                break;
            }
        }
    }
}

// inserts a country pointer directly without checking for duplicates
// used by load_p3 to bulk-populate the table after all countries are built from the csv
// skipping the duplicate check is safe here since load_p3 only sees each country once
void HashTable::insertHelper(CountryData* country) {

    int w = computeW(country->getCountryCode());
    int primary = h1(w);
    int step = h2(w);

    for (int i = 0; i < TABLE_SIZE; i++) {

        int indx = (primary + i * step) % TABLE_SIZE;

        // find the first empty or tombstone slot and place the country there
        if (status[indx] == 0 || status[indx] == 2) {

            table[indx] = country;
            status[indx] = 1;
            return;
        }
    }
}