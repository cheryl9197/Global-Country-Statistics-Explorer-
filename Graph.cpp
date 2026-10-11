#include "Graph.hpp"
#include <vector>
#include <string>
#include <queue>
#include <iostream>
#include <cmath>

// store the pointer and count once so every function can access country data
// without needing it passed in every time
Graph::Graph(CountryData* arr, int count) {
    countries = arr;
    countryCount = count;
}

// vectors clean themselves up automatically
// nothing to delete since we don't own the countries array
Graph::~Graph() {}

// clearing every cell resets the graph to a fully disconnected state
// this handles both the first call and subsequent calls that need to wipe existing edges
void Graph::initialize() {

    for (int i = 0; i < countryCount; i++) {

        for (int j = 0; j < countryCount; j++) {

            edges[i][j].clear();
        }
    }
}

// linear search is necessary here since countries are not indexed by code in the graph
// returns -1 so callers can check whether the country was found before proceeding
int Graph::getIndex(const std::string& code) const {

    for (int i = 0; i < countryCount; i++) {

        if (countries[i].getCountryCode() == code) {
            return i;
        }
    }
    return -1;
}

// scans all country pairs and adds a relationship tuple to their shared edge if both satisfy the relation
// returns true if at least one new relationship was added, false if nothing changed
bool Graph::updateEdges(const std::string& seriesCode, double threshold, const std::string& relation) {
    
    bool added = false;

    for (int i = 0; i < countryCount; i++) {

        int seriesIdxI = countries[i].getSeriesIndex(seriesCode);

        if (seriesIdxI != -1) {

            double meanI = countries[i].getSeriesMean(seriesIdxI);

            if (meanI != -1) {

                // j starts at i + 1 to avoid processing the same pair twice
                // (i, j) and (j, i) represent the same edge so we only need one direction here
                for (int j = i + 1; j < countryCount; j++) {

                    int seriesIdxJ = countries[j].getSeriesIndex(seriesCode);

                    if (seriesIdxJ != -1) {

                        double meanJ = countries[j].getSeriesMean(seriesIdxJ);

                        if (meanJ != -1) {

                            bool iSatisfies = false;
                            bool jSatisfies = false;

                            if (relation == "greater") {

                                iSatisfies = meanI > threshold;
                                jSatisfies = meanJ > threshold;
                            } else if (relation == "less") {

                                iSatisfies = meanI < threshold;
                                jSatisfies = meanJ < threshold;
                            } else if (relation == "equal") {

                                // fabs handles the case where the difference is negative
                                // a plain subtraction check would incorrectly pass for values far below threshold
                                iSatisfies = std::fabs(meanI - threshold) <= 1e-3;
                                jSatisfies = std::fabs(meanJ - threshold) <= 1e-3;
                            }

                            if (iSatisfies && jSatisfies) {

                                Relationship newRel;
                                newRel.seriesCode = seriesCode;
                                newRel.threshold = threshold;
                                newRel.relation = relation;

                                // check for duplicates before inserting so the same tuple
                                // is never stored more than once on the same edge
                                bool exists = false;

                                for (int k = 0; k < edges[i][j].size(); k++) {

                                    if (edges[i][j][k] == newRel) {

                                        exists = true;
                                        break;
                                    }
                                }

                                // insert into both directions so adjacency lookups work
                                // regardless of which index is i and which is j
                                if (!exists) {

                                    edges[i][j].push_back(newRel);
                                    edges[j][i].push_back(newRel);
                                    added = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return added;
}

// prints the names of all countries that share an edge with the given country
// an empty vector at edges[i][j] means no edge exists between i and j
void Graph::adjacent(const std::string& countryCode) {

    int i = getIndex(countryCode);

    if (i == -1) {

        std::cout << "failure" << std::endl;
        return;
    }

    bool hasNeighbor = false;

    for (int j = 0; j < countryCount; j++) {

        if (edges[i][j].size() > 0) {

            std::cout << countries[j].getCountryName() << " ";
            hasNeighbor = true;
        }
    }

    if (!hasNeighbor) {

        std::cout << "none" << std::endl;
    } else {

        std::cout << std::endl;
    }
}

// determines if two countries are connected by any path in the graph using BFS
// visited array prevents revisiting countries and avoids infinite loops in cycles
bool Graph::path(const std::string& code1, const std::string& code2) {

    int start = getIndex(code1);
    int end = getIndex(code2);

    if (start == -1 || end == -1) {
        return false;
    }

    bool visited[512] = {false};
    std::queue<int> q;

    visited[start] = true;
    q.push(start);

    while (!q.empty()) {

        int current = q.front();
        q.pop();

        if (current == end) {
            return true;
        }

        // queue any unvisited neighbor so it gets explored in a future iteration
        for (int j = 0; j < countryCount; j++) {

            if (edges[current][j].size() > 0 && !visited[j]) {

                visited[j] = true;
                q.push(j);
            }
        }
    }

    return false;
}

// prints all relationship tuples stored on the edge between two countries
// an empty vector means no edge exists, so none is printed
void Graph::relationships(const std::string& code1, const std::string& code2) {

    int i = getIndex(code1);
    int j = getIndex(code2);

    // guard against invalid codes even though spec says they will exist
    if (i == -1 || j == -1) {

        std::cout << "none" << std::endl;
        return;
    }

    if (edges[i][j].size() == 0) {

        std::cout << "none" << std::endl;
        return;
    }

    // no trailing space after the last tuple to match the spec format exactly
    for (int k = 0; k < edges[i][j].size(); k++) {

        std::cout << "(" << edges[i][j][k].seriesCode << " " << edges[i][j][k].threshold << " " << edges[i][j][k].relation << ")";

        if (k < edges[i][j].size() - 1) {

            std::cout << " ";
        }
    }

    std::cout << std::endl;
}