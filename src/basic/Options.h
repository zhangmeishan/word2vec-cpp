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
	int blockSize;
	int negative;
	dtype adaAlpha;
	int thread;

	dtype initRange;	
	dtype adaEps;
	dtype regParameter;
	dtype dropProb;
	int wordcontext;
	bool wordEmbFineTune;
    bool seg;

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
        blockSize = 1000;
		negative = 5;
		adaAlpha = 0.025;
		thread = 2;

        initRange = 0.01;              
        adaEps = 1e-6;      
        regParameter = 1e-8;
        dropProb = 0.0;       
        wordcontext = 2;
        wordEmbFineTune = true;        
        seg = false;
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
			if (pr.first == "blockSize")
				blockSize = atoi(pr.second.c_str());
			if (pr.first == "negative")
				negative = atoi(pr.second.c_str());
			if (pr.first == "adaAlpha")
				adaAlpha = atof(pr.second.c_str());
			if (pr.first == "thread")
				thread = atoi(pr.second.c_str());


            if (pr.first == "adaEps")
                adaEps = atof(pr.second.c_str());
            if (pr.first == "regParameter")
                regParameter = atof(pr.second.c_str());
            if (pr.first == "dropProb")
                dropProb = atof(pr.second.c_str());
            if (pr.first == "wordcontext")
                wordcontext = atoi(pr.second.c_str());
			if (pr.first == "wordEmbFineTune")
				wordEmbFineTune = (pr.second == "true") ? true : false;
			if (pr.first == "initRange")
				initRange = atof(pr.second.c_str());
            if (pr.first == "seg")
                seg = (pr.second == "true") ? true : false;
            if (pr.first == "wordFile")
                wordFile = pr.second;
        }
    }

    void showOptions() {
        std::cout << "cutoff = " << cutoff << std::endl;	
		std::cout << "verboseIter = " << verboseIter << std::endl;
        std::cout << "maxIter = " << maxIter << std::endl;
		std::cout << "maxInstance = " << maxInstance << std::endl;
		std::cout << "embSize = " << embSize << std::endl;
		std::cout << "inputFile = " << inputFile << std::endl;
		std::cout << "outputSFile = " << outputTFile << std::endl;
		std::cout << "outputTFile = " << outputTFile << std::endl;
		std::cout << "featFile = " << featFile << std::endl;
		std::cout << "blockSize = " << blockSize << std::endl;
		std::cout << "negative = " << negative << std::endl;
		std::cout << "adaAlpha = " << adaAlpha << std::endl;
		std::cout << "thread = " << thread << std::endl;
            
        std::cout << "wordcontext = " << wordcontext << std::endl;
        std::cout << "wordEmbFineTune = " << wordEmbFineTune << std::endl;		
		std::cout << "adaEps = " << adaEps << std::endl;
		std::cout << "regParameter = " << regParameter << std::endl;
		std::cout << "dropProb = " << dropProb << std::endl;	
		std::cout << "initRange = " << initRange << std::endl;
        std::cout << "saveItermediate = " << outputSFile << std::endl;
        std::cout << "seg = " << seg << std::endl;
        std::cout << "wordFile = " << wordFile << std::endl;
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

