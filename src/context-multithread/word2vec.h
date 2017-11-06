/*
 * Classifier.h
 *
 *  Created on: Jan 25, 2016
 *      Author: mszhang
 */

#ifndef SRC_WORD2VEC_H_
#define SRC_WORD2VEC_H_


#include "Options.h"
#include "Pipe.h"
#include "Utf.h"
#include "NRMat.h"
#include "Alphabet.h"
#include "Model.h"

using namespace nr;
using namespace std;


class Classifier {
  public:
    Classifier();
    virtual ~Classifier();

  public:
    Options m_options;
    Pipe m_pipe;

	Model w2v;

  public:
    void extractFeat();
	void init(const string& optionFile);

	void prepare();
	void train();
	void train(int threads);

};

#endif /* SRC_WORD2VEC_H_ */
