/*
 * CAction.h
 *
 *  Created on: Oct 6, 2015
 *      Author: mszhang
 */

#ifndef _SRC_SEXAMPLE_H_
#define _SRC_SEXAMPLE_H_
#include <string>

using namespace std;

class SExample {
  public:
    vector<string> source;
    string target;

  public:
    SExample() {
		source.clear();
        target = "";
    }

};


#endif /* _SRC_SEXAMPLE_H_ */
