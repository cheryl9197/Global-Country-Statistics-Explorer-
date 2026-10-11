#ifndef HASHTABLE_HPP
#define HASHTABLE_HPP
#include "CountryData.hpp"
#include <string>

class HashTable {

    private:

        static const int TABLE_SIZE = 512;
        CountryData* table[TABLE_SIZE];
        int status[TABLE_SIZE];  // 0 = empty, 1 = occupied, 2 = tombstone (previously occupied)

        // hashing helpers, kept private since nothing outside needs to call them directly
        int computeW(const std::string& code);
        int h1(int w);
        int h2(int w);

    public:

        HashTable();
        ~HashTable();

        bool lookup_p4(const std::string& country_code, int& index, int& steps);
        bool remove_p4(const std::string& country_code);
        bool insert_p4(const std::string& country_code, CountryData countries[], int& countryCount);
        void clean_p4();

        // used by load_p3 to bulk insert without checking for duplicates
        void insertHelper(CountryData* country);
};

#endif