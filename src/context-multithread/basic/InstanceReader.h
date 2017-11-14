#ifndef _CONLL_READER_
#define _CONLL_READER_

#include "Reader.h"
#include <sstream>

using namespace std;
/*
 this class reads conll-format data 
 */
class InstanceReader : public Reader {
  public:
    InstanceReader() {
    }
    ~InstanceReader() {
    }

    Instance *getNext() {
        m_instance.clear();

		string strLine;
		vector<string> vecLine;
		vecLine.clear();
		while (1) {
			if (!my_getline(m_inf, strLine)) {
				break;
			}
			if (strLine.empty())
				break;
			vecLine.push_back(strLine);
		}

		if (vecLine.size() == 0) return NULL;

		vector<string> units;
					
		split_bychar(vecLine[0], units, '\t');
		int sentSize = vecLine.size();
		int unitsize = units.size() - 1;
		m_instance.allocate(sentSize, unitsize);
		for (int i = 0; i < vecLine.size(); i++) {
			split_bychar(vecLine[i], units, '\t');
			m_instance.m_words[i] = units[0];
			for (int idx = 0; idx < unitsize; idx++) {
				m_instance.m_labels[i][idx] = units[idx + 1];
			}
		}

        return &m_instance;
    }
};

#endif

