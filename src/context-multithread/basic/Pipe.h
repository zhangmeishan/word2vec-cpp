#ifndef _JST_PIPE_
#define _JST_PIPE_

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include "Instance.h"
#include "InstanceReader.h"
#include <iterator>

using namespace std;

//#define MAX_BUFFER_SIZE 256

class Pipe {
  public:
    Pipe() {
        m_jstReader = new InstanceReader();
        max_sentense_size = 512;
    }

    ~Pipe(void) {
        if (m_jstReader)
            delete m_jstReader;
    }

    int initInputFile(const char *filename) {
        if (0 != m_jstReader->startReading(filename))
            return -1;
        return 0;
    }

    void uninitInputFile() {
        if (m_jstReader)
            m_jstReader->finishReading();
    }

    Instance* nextInstance() {
        Instance *pInstance = m_jstReader->getNext();
        if (!pInstance)
            return 0;

        return pInstance;
    }

	/* one example for reading
    void readInstances(const string& m_strInFile, vector<Instance>& vecInstances, int maxInstance = -1) {
        vecInstances.clear();
        initInputFile(m_strInFile.c_str());

        Instance *pInstance = nextInstance();
        int numInstance = 0;

        while (pInstance) {

            Instance trainInstance;
            trainInstance.copyValuesFrom(*pInstance);
            vecInstances.push_back(trainInstance);
            numInstance++;

            if (numInstance == maxInstance) {
                break;
            }

            pInstance = nextInstance();

        }

        uninitInputFile();

        cout << endl;
        cout << "instance num: " << numInstance << endl;
    }
	*/
	
  public:
    int max_sentense_size;

  protected:
    Reader *m_jstReader;

};

#endif
