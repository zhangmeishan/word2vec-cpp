#ifndef _INSTANCE_H_
#define _INSTANCE_H_

#include "MyLib.h"
#include <iostream>
using namespace std;

class Instance {
  public:
    void clear() {
        m_words.clear();
        clearVec(m_labels);
    }

    int size() const {
        return m_words.size();
    }

    void allocate(int length, int item) {
        clear();
        m_words.resize(length);
		resizeVec(m_labels, length, item);
    }
  public:
    vector<string> m_words;
    vector<vector<string> > m_labels;
};

#endif /*_INSTANCE_H_*/
