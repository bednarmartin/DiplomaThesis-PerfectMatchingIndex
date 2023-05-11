#include <iostream>
#include <ctime>
#include "tests.hpp"
#include "graph.cpp"
#include "unordered_set"
#include "perfectmatching.h"
#include "binaryfunctions.hpp"
#include "kempeswitches.h"
#include "SATperfectmatching.h"
#include "disjuction.h"
#include <climits>

void doKempe();

void doRepresentantTest(char *const *argv);

void doDisjunction(char *const *argv);

void doTest(int argc, char *const *argv);

void doConvert(char *const *argv);

void poleTestWithoutSAT(Edge &edge1, Edge &edge2, Graph &graph, std::ofstream &outdata);

void poleTestWithSAT(int numOfThreads, Edge &edge1, Edge &edge2, Graph &graph, std::ofstream &outdata);

void processGraph(Graph &graph, std::ofstream &outdata, bool SAT, bool noDangling, int numOfThreads) {
    if (noDangling) {
        findPerfectMatchingSAT(graph);
        return;
    }
    for (int i = 0; i < (3 * graph.numberOfVertices) / 2; i++) {
        Edge edge1 = graph.edgesList[i];
        for (int j = i + 1; j < (3 * graph.numberOfVertices) / 2; j++) {
            Edge edge2 = graph.edgesList[j];
            if (std::unordered_set{edge1.vertex1, edge1.vertex2, edge2.vertex1, edge2.vertex2}.size() == 4) {
                if (SAT) {
                    poleTestWithSAT(numOfThreads, edge1, edge2, graph, outdata);
                } else {
                    poleTestWithoutSAT(edge1, edge2, graph, outdata);

                }
            }
        }
    }
}

void poleTestWithSAT(int numOfThreads, Edge &edge1, Edge &edge2, Graph &graph, std::ofstream &outdata) {
    long long int representation = findPerfectMatchingSATWithDanglingEdges(graph, edge1, edge2,
                                                                           numOfThreads);
    outdata << getRepresentant(representation);
    outdata << " - [" << edge1.vertex1 << " " << edge1.vertex2 << "],[" << edge2.vertex1 << " "
            << edge2.vertex2 << "]" << std::endl;
}

void poleTestWithoutSAT(Edge &edge1, Edge &edge2, Graph &graph, std::ofstream &outdata) {
    perfectMatchings.clear();
    PoleMaker::make4Pole(graph, edge1, edge2);
    std::unordered_set<long long int> usedEdges;
    findPerfectMatching(graph, 0ll, usedEdges, getOnes(graph.numberOfVertices));
    std::vector<std::unordered_set<long long int>> usedPerfectMatches;
    determineCombinationsOfPerfectMatchings(graph);
    outdata << getRepresentant(graph.combinations);
    PoleMaker::reverseMake4Pole(graph, edge1, edge2);
    outdata << " - [" << edge1.vertex1 << " " << edge1.vertex2 << "],[" << edge2.vertex1 << " "
            << edge2.vertex2 << "]" << std::endl;
}

bool isGood(long long int representation) {
    initializePermutations();
    initializeMasks();
    return isRepresentant(representation) && isKempeClosed(representation);
}

void doKempe() {
    std::ofstream kempeOutdata("KempeOutput.txt");
    determineAllPossibleRepresentants(kempeOutdata);
    kempeOutdata.close();
}

void doRepresentantTest(char *const *argv) {
    if (isGood(std::stoll(argv[2]))) {
        std::cout << "IS GOOD" << std::endl;
    } else {
        std::cout << "IS BAD" << std::endl;
        if (isRepresentant(std::stoll(argv[2]))) {
            std::cout << "NUMBER IS REPRESENTANT" << std::endl;
        } else {
            std::cout << "NUMBER IS NOT REPRESENTANT" << std::endl;
            std::cout << "REPRESENTANT IS " << getRepresentant((std::stoll(argv[2]))) << std::endl;
        }
    }
    std::cout << convertRepresentation(std::stoll(argv[2])) << std::endl;
}

void doConvert(char *const *argv) {
    long long int representation = std::stoll(argv[2]);
    std::cout << convertRepresentation(representation) << std::endl;
}

void doTest(int argc, char *const *argv) {
    int numOfThreads = 4;
    bool SAT = true;
    bool noDangling = false;

    GraphReader graphReader = GraphReader(argv[2]);
    long long int counter = 1;
    std::ofstream outdata(argv[3]);
    if (argc > 4) {
        if (argv[4] == std::string("SAT")) {
            noDangling = true;
        }
    }
    int threshold = 0;
    int end = INT_MAX;

    /*if (argc >= 6) {
        threshold = std::stoi(argv[4]);
        end = std::stoi(argv[5]);
    }*/

    while (true) {
        Graph graph = graphReader.next();
        if (!graphReader.hasNext() || counter >= end) {
            break;
        }
        std::cout << "GRAPH " << counter << std::endl;
        graph.determineBridge();
        if (graph.hasBridge || counter < threshold) {
            std::cout << "HAS BRIDGE" << std::endl;
        } else {
            outdata << "GRAPH " << counter << " - " << argv[1] << std::endl;
            processGraph(graph, outdata, SAT, noDangling, numOfThreads);
            outdata << std::endl;
        }

        delete[] graph.adjacencyList;
        delete[] graph.edgesList;
        counter++;
    }
    outdata.close();
}

void doDisjunction(char *const *argv) {
    std::vector<long long int> representants = getRepresentants(argv[2]);
    for (int i = 0; i < representants.size(); i++) {
        long long int representant1 = representants[i];
        for (int j = i + 1; j < representants.size(); j++) {
            long long int representant2 = representants[j];
            bool was = false;
            for (int p1 = 0; p1 < 24; p1++) {
                if (was) {
                    break;
                }
                long long int newRepresentant1 = getPermutation(representant1, p1);
                for (int p2 = 0; p2 < 24; p2++) {
                    if (was) {
                        break;
                    }
                    long long int newRepresentant2 = getPermutation(representant2, p2);
                    if ((newRepresentant1 & newRepresentant2) == 0) {
                        std::cout << newRepresentant1 << " - " << newRepresentant2 << " FROM REPRESENTANTS: "
                                  << representant1 << " AND " << representant2 << std::endl;
                        was = true;
                    }
                }
            }
        }
    }
}


int main([[maybe_unused]] int argc, char **argv) {
    //runAllTests();
    std::clock_t start;
    double duration;
    start = std::clock();

    initializePermutations();

    if (argv[1] == std::string("kempe")) {
        doKempe();
    } else if (argv[1] == std::string("representant_test")) {
        doRepresentantTest(argv);
    } else if (argv[1] == std::string("disjuction")) {
        doDisjunction(argv);
    } else if (argv[1] == std::string("convert")) {
        doConvert(argv);
    } else if (argv[1] == std::string("test")) {
        doTest(argc, argv);
    }

    deletePermutation();

    duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
    std::cout << "Operation took " << duration << " seconds" <<
              std::endl;
    return 0;
}




