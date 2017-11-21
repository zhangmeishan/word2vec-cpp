/*
 * Classifier.h
 *
 *  Created on: Jan 25, 2016
 *      Author: mszhang
 */

#ifndef SRC_WORD2VEC_H_
#define SRC_WORD2VEC_H_


#include "Options.h"
#include "Instance.h"
#include "Utf.h"
#include "NRMat.h"
#include "Alphabet.h"
#include "Driver.h"
#include "SExample.h"

using namespace nr;
using namespace std;


class Classifier {
  public:
    Classifier();
    virtual ~Classifier();

  public:
    Options m_options;

    Driver w2v;

  public:
    void init(const string& optionFile);

    void prepare();
    void train();
    void finish();

  private:
    void get_sexamples(const Instance& inst, vector<SExample>& sexams);

};

#endif /* SRC_WORD2VEC_H_ */
