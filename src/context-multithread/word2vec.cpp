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

void Classifier::extractFeat() {
	//step 1
	Pipe pipe;
	pipe.initInputFile(m_options.inputFile.c_str());

	Instance *pInstance = pipe.nextInstance();
	int numInstance = 0;

	Alphabet org_word, org_context;
	int context_size = 2;

	while (pInstance) {
		int size = pInstance->size();

		for (int idx = 0; idx < size; idx++) {
			org_word.add_string(pInstance->m_words[idx]);
			for (int idy = idx - context_size; idy < idx + context_size; idy++) {
				if (idy == idx) continue;
				string featmark = "F@" + obj2string(idy - idx);
				string featvalue = "";
				if (idy < 0 || idy >= size) featvalue = "</s>";
				else featvalue = pInstance->m_words[idy];
				org_context.add_string(featmark + "=" + featvalue);
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
	cout << "Total word num: " << org_word.size() << endl;
	cout << "Total context num: " << org_context.size() << endl;

	//step 2
	vector<int> indexes;

	org_word.sort(indexes, m_options.cutoff);
	w2v.m_source_vob.clear();
	int vec_size = indexes.size();
	for (int idx = 0; idx < vec_size; idx++) {
		w2v.m_source_vob.add_string(org_word.m_id_to_string[indexes[idx]], org_word.m_id_to_freq[indexes[idx]]);
	}
	
	org_context.sort(indexes, m_options.cutoff);
	w2v.m_target_vob.clear();
	vec_size = indexes.size();
	for (int idx = 0; idx < vec_size; idx++) {
		w2v.m_target_vob.add_string(org_context.m_id_to_string[indexes[idx]], org_context.m_id_to_freq[indexes[idx]]);
	}


	//step 3
	std::ofstream os(m_options.featFile.c_str());
	//os << "m_source_vob" << std::endl;
	//w2v.m_source_vob.write(os);
	//os << "m_target_vob" << std::endl;
	//w2v.m_target_vob.write(os);
	pipe.initInputFile(m_options.inputFile.c_str());
	pInstance = pipe.nextInstance();
	numInstance = 0;
	w2v.m_example_num = 0;
	while (pInstance) {
		int size = pInstance->size();

		for (int idx = 0; idx < size; idx++) {
			//org_word.add_string(pInstance->m_words[idx]);
			int org_id = w2v.m_source_vob[pInstance->m_words[idx]];
			if (org_id == -1)continue;
			for (int idy = idx - context_size; idy < idx + context_size; idy++) {
				if (idy == idx) continue;
				string featmark = "F@" + obj2string(idy - idx);
				string featvalue = "";
				if (idy < 0 || idy >= size) featvalue = "</s>";
				else featvalue = pInstance->m_words[idy];
				//org_context.add_string(featmark + "=" + featvalue);
				int context_id = w2v.m_target_vob[featmark + "=" + featvalue];
				if (context_id == -1)continue;
				os << org_id << "\t" << context_id << std::endl;
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
	w2v.m_interval = w2v.m_example_num / 100;

	pipe.uninitInputFile();
	os.close();

}


void Classifier::prepare() {
	w2v.init_unitable();
	w2v.init_emb(m_options.embSize);
}


void Classifier::train() {
	for (int idx = 0; idx < m_options.maxIter; idx++) {
		//w2v.train_one_iteration(m_options.featFile, m_options.blockSize);
	}

	w2v.save_model(m_options.outputSFile, m_options.outputTFile);
}

void Classifier::train(int threads) {
	w2v.init_block(m_options.blockSize);
	for (int idx = 0; idx < m_options.maxIter; idx++) {
		w2v.train_one_iteration_thread(m_options.featFile, threads);
	}

	w2v.save_model(m_options.outputSFile, m_options.outputTFile);
}

int main(int argc, char* argv[]) {
    std::string optionFile = "";
    dsr::Argument_helper ah;
    int threads = 2;

    ah.new_named_string("option", "optionFile", "named_string", "option file to train a model, optional when training", optionFile);
    ah.new_named_int("th", "thread", "named_int", "number of threads for openmp", threads);

    ah.process(argc, argv);

    //omp_set_num_threads(threads);
//  Eigen::setNbThreads(threads);
//  mkl_set_num_threads(4);
//  mkl_set_dynamic(false);
//  omp_set_nested(false);
//  omp_set_dynamic(false);


    Classifier Classifier;
	Classifier.init(optionFile);
	Classifier.extractFeat();
	Classifier.prepare();
	Classifier.train(threads);

}
