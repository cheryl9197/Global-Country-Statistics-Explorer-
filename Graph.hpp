#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <string>
#include <vector>
#include "CountryData.hpp"

// a relationship tuple stores the three components that define a shared condition between two countries
// operator== is defined here so duplicate checking in updateEdges can use a single comparison
struct Relationship {

    std::string seriesCode;
    double threshold;
    std::string relation;

    bool operator==(const Relationship& other) const {

        return seriesCode == other.seriesCode &&
               threshold == other.threshold &&
               relation == other.relation;
    }
};

// represents an undirected graph where nodes are countries and edges store relationship tuples
// edges[i][j] holds all relationships shared between country i and country j
// an empty vector at edges[i][j] means no edge exists between those two countries
class Graph {

protected:
    std::vector<Relationship> edges[512][512];
    CountryData* countries;
    int countryCount;

    // kept protected since nothing outside the class needs to call this directly
    int getIndex(const std::string& code) const;

public:
    Graph(CountryData* arr, int count);
    ~Graph();

    void initialize();
    bool updateEdges(const std::string& seriesCode, double threshold, const std::string& relation);
    void adjacent(const std::string& countryCode);
    bool path(const std::string& code1, const std::string& code2);
    void relationships(const std::string& code1, const std::string& code2);
};

#endif