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
    int parse_instance(const vector<vector<string> >& inputs, Instance& inst);
    void get_features(const Instance& inst, vector<string>& sources, vector<string>& targets, int pos, bool clear);

};

#endif /* SRC_WORD2VEC_H_ */
