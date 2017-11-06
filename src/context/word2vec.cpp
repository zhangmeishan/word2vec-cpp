/*
 * Classifier.cpp
 *
 *  Created on: Jan 25, 2016
 *      Author: mszhang
 */

#include "word2vec.h"
#include <chrono>
#include <omp.h>
#include "Argument_helper.h"

Classifier::Classifier() {
    // TODO Auto-generated constructor stub
    srand(0);
    //Node::id = 0;
}

Classifier::~Classifier() {
    // TODO Auto-generated destructor stub
}

void Classifier::init(const string& optionFile) {
	if (optionFile != "")
		m_options.load(optionFile);

	m_options.showOptions();
}

void Classifier::prepare() {
	//step 1
	Pipe pipe;
	pipe.initInputFile(m_options.inputFile.c_str());

	Instance *pInstance = pipe.nextInstance();
	int numInstance = 0;

	Alphabet unsorted_source_vob, unsorted_target_vob;
	int context_size = 2;

	while (pInstance) {
		int size = pInstance->size();

		for (int idx = 0; idx < size; idx++) {
			unsorted_source_vob.add_string(pInstance->m_words[idx]);
			for (int idy = idx - context_size; idy < idx + context_size; idy++) {
				if (idy == idx) continue;
				string featmark = "F@" + obj2string(idy - idx);
				string featvalue = "";
				if (idy < 0 || idy >= size) featvalue = "</s>";
				else featvalue = pInstance->m_words[idy];
				unsorted_target_vob.add_string(featmark + "=" + featvalue);
			}
		}

		numInstance++;
		if ((numInstance + 1) % m_options.verboseIter == 0) {
			cout << numInstance + 1 << " ";
			if ((numInstance + 1) % (40 * m_options.verboseIter) == 0)
				cout << std::endl;
			cout.flush();
		}
		if (m_options.maxInstance > 0 && numInstance == m_options.maxInstance)
			break;

		pInstance = pipe.nextInstance();
	}

	pipe.uninitInputFile();

	cout << numInstance << " " << endl;
	cout << "Total source  num: " << unsorted_source_vob.size() << endl;
	cout << "Total target num: " << unsorted_target_vob.size() << endl;

	//step2 driver initialization
	omp_set_num_threads(m_options.thread);
	w2v.m_w2v_model.init_emb(&unsorted_source_vob, &unsorted_target_vob, m_options.embSize, m_options.thread,  m_options.cutoff);
	w2v.init(m_options);

	Alphabet& source_vob = w2v.m_w2v_model.m_source_vob;
	Alphabet& target_vob = w2v.m_w2v_model.m_target_vob;
	cout << "After sorting and reducing: " << endl;
	cout << "Total source num: " << source_vob.size() << endl;
	cout << "Total target num: " << target_vob.size() << endl;
	
	//step 3  feat file generation (can be very huge large that impossible to put all examples into memory)
	std::ofstream os(m_options.featFile.c_str());
	pipe.initInputFile(m_options.inputFile.c_str());
	pInstance = pipe.nextInstance();
	numInstance = 0;
	w2v.m_example_num = 0;
	while (pInstance) {
		int size = pInstance->size();

		for (int idx = 0; idx < size; idx++) {
			int source_id = source_vob[pInstance->m_words[idx]];
			if (source_id == -1)continue;
			for (int idy = idx - context_size; idy < idx + context_size; idy++) {
				if (idy == idx) continue;
				string featmark = "F@" + obj2string(idy - idx);
				string featvalue = "";
				if (idy < 0 || idy >= size) featvalue = "</s>";
				else featvalue = pInstance->m_words[idy];
				int target_id = target_vob[featmark + "=" + featvalue];
				if (target_id == -1)continue;
				os << source_id << "\t" << target_id << std::endl;
				w2v.m_example_num++;
			}
		}
		numInstance++;
		if ((numInstance + 1) % m_options.verboseIter == 0) {
			cout << numInstance + 1 << " ";
			if ((numInstance + 1) % (40 * m_options.verboseIter) == 0)
				cout << std::endl;
			cout.flush();
		}
		if (m_options.maxInstance > 0 && numInstance == m_options.maxInstance)
			break;

		pInstance = pipe.nextInstance();
	}
	cout << numInstance << " " << endl;
	cout << "Total example num: " << w2v.m_example_num << endl;

	pipe.uninitInputFile();
	os.close();
}



void Classifier::train() {
	for (int idx = 0; idx < m_options.maxIter; idx++) {
		w2v.train_one_iteration(m_options.featFile, m_options.blockSize, m_options.maxIter);
	}

	w2v.save_model(m_options.outputSFile, m_options.outputTFile);
}

int main(int argc, char* argv[]) {
    std::string optionFile = "";
    dsr::Argument_helper ah;

    ah.new_named_string("option", "optionFile", "named_string", "option file to train a model, optional when training", optionFile);

    ah.process(argc, argv);

//    omp_set_num_threads(threads);
//  Eigen::setNbThreads(threads);
//  mkl_set_num_threads(4);
//  mkl_set_dynamic(false);
//  omp_set_nested(false);
//  omp_set_dynamic(false);


    Classifier classifier;
	classifier.init(optionFile);
	classifier.prepare();
	classifier.train();

}
