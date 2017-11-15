#ifndef _OPTIONS_
#define _OPTIONS_

#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include "MyLib.h"

using namespace std;

class Options {
  public:
    int maxIter;
    int verboseIter;
    int maxInstance;
    int cutoff;
    int embSize;
    string inputFile;
    string outputSFile;
    string outputTFile;
    string featFile;
    int negative;
    dtype adaAlpha;
    int thread;
    dtype sample;
    int context;
    int conll;

    dtype initRange;
    dtype regParameter;
    dtype dropProb;
    bool wordEmbFineTune;
    int no_use_blockSize;

    //embedding files
    string wordFile;

    Options() {
        maxIter = 5;
        cutoff = 5;
        maxInstance = -1;
        verboseIter = 100;
        embSize = 50;
        inputFile = "";
        outputSFile = "";
        outputTFile = "";
        featFile = "tmp";
        negative = 5;
        adaAlpha = 0.025;
        thread = 2;
        sample = 0.001;
        context = 2;
        conll = 0; //0, denotes normal; 1 denotes conll; 2 denotes feature pair already (one line one pair)

        initRange = 0.01;
        regParameter = 1e-8;
        dropProb = 0.0;
        no_use_blockSize = 100;
        wordEmbFineTune = true;
        wordFile = "";
    }

    virtual ~Options() {

    }

    void setOptions(const vector<string> &vecOption) {
        int i = 0;
        for (; i < vecOption.size(); ++i) {
            pair<string, string> pr;
            string2pair(vecOption[i], pr, '=');
            if (pr.first == "cutoff")
                cutoff = atoi(pr.second.c_str());
            if (pr.first == "maxIter")
                maxIter = atoi(pr.second.c_str());
            if (pr.first == "verboseIter")
                verboseIter = atoi(pr.second.c_str());
            if (pr.first == "inputFile")
                inputFile = pr.second;
            if (pr.first == "outputTFile")
                outputTFile = pr.second;
            if (pr.first == "outputSFile")
                outputSFile = pr.second;
            if (pr.first == "embSize")
                embSize = atoi(pr.second.c_str());
            if (pr.first == "maxInstance")
                maxInstance = atoi(pr.second.c_str());
            if (pr.first == "featFile")
                featFile = pr.second;
            if (pr.first == "negative")
                negative = atoi(pr.second.c_str());
            if (pr.first == "adaAlpha")
                adaAlpha = atof(pr.second.c_str());
            if (pr.first == "thread")
                thread = atoi(pr.second.c_str());
            if (pr.first == "sample")
                sample = atof(pr.second.c_str());
            if (pr.first == "context")
                context = atoi(pr.second.c_str());
            if (pr.first == "conll")
                conll = atoi(pr.second.c_str());
            if (conll > 2 || conll < 0) conll = 0;


            if (pr.first == "regParameter")
                regParameter = atof(pr.second.c_str());
            if (pr.first == "dropProb")
                dropProb = atof(pr.second.c_str());
            if (pr.first == "wordEmbFineTune")
                wordEmbFineTune = (pr.second == "true") ? true : false;
            if (pr.first == "initRange")
                initRange = atof(pr.second.c_str());
            if (pr.first == "wordFile")
                wordFile = pr.second;
            if (pr.first == "no_use_blockSize")
                no_use_blockSize = atoi(pr.second.c_str());
        }
    }

    void showOptions() {
        std::cout << "cutoff = " << cutoff << std::endl;
        std::cout << "verboseIter = " << verboseIter << std::endl;
        std::cout << "maxIter = " << maxIter << std::endl;
        std::cout << "maxInstance = " << maxInstance << std::endl;
        std::cout << "embSize = " << embSize << std::endl;
        std::cout << "inputFile = " << inputFile << std::endl;
        std::cout << "outputSFile = " << outputSFile << std::endl;
        std::cout << "outputTFile = " << outputTFile << std::endl;
        std::cout << "featFile = " << featFile << std::endl;
        std::cout << "negative = " << negative << std::endl;
        std::cout << "adaAlpha = " << adaAlpha << std::endl;
        std::cout << "thread = " << thread << std::endl;
        std::cout << "sample = " << sample << std::endl;
        std::cout << "context = " << context << std::endl;
        std::cout << "conll = " << conll << std::endl;

        std::cout << "wordEmbFineTune = " << wordEmbFineTune << std::endl;
        std::cout << "regParameter = " << regParameter << std::endl;
        std::cout << "dropProb = " << dropProb << std::endl;
        std::cout << "initRange = " << initRange << std::endl;
        std::cout << "saveItermediate = " << outputSFile << std::endl;
        std::cout << "wordFile = " << wordFile << std::endl;
        std::cout << "no_use_blockSize = " << no_use_blockSize << std::endl;
    }

    void load(const std::string& infile) {
        ifstream inf;
        inf.open(infile.c_str());
        vector<string> vecLine;
        while (1) {
            string strLine;
            if (!my_getline(inf, strLine)) {
                break;
            }
            if (strLine.empty())
                continue;
            vecLine.push_back(strLine);
        }
        inf.close();
        setOptions(vecLine);
    }
};

#endif

