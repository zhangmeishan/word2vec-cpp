/*
 * Classifier.cpp
 *
 *  Created on: Jan 25, 2016
 *      Author: mszhang
 */

#include "chnword2vec.h"
#include <chrono>
#include <omp.h>
#include "Argument_helper.h"
#include "Utf.h"

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

	omp_set_num_threads(m_options.thread);
}


void Classifier::prepare() {
	//step 1
	std::cout << "Starting creating vocabularies...." << std::endl;
	auto start_time = std::chrono::high_resolution_clock::now();
	vector<Instance> insts(m_options.thread);
	FILE* is = fopen(m_options.inputFile.c_str(), "r");
	if (is == NULL) {
		std::cout << "File Read error: " << m_options.inputFile << std::endl;
		return;
	}
	int numInstance = 0;
	Alphabet unsorted_source_vob, unsorted_target_vob;
	while (true) {
		for (int idx = 0; idx < m_options.thread; idx++) {
			insts[idx].clear();
			insts[idx].read(is, m_options.conll);
		}

#pragma omp parallel for
		for (int idx = 0; idx < m_options.thread; idx++) {
			int sentSize = insts[idx].m_length;
			if (sentSize <= 0)continue;
			vector<SExample> sexams;			
			get_sexamples(insts[idx], sexams);

#pragma omp critical
			{
				for (int idz = 0; idz < sexams.size(); idz++) {
					unsorted_source_vob.add_string(sexams[idz].source);
					unsorted_target_vob.add_string(sexams[idz].target);
				}

				numInstance++;
				if ((numInstance + 1) % m_options.verboseIter == 0) {
					cout << numInstance + 1 << " ";
					if ((numInstance + 1) % (20 * m_options.verboseIter) == 0) {
						std::cout << std::endl;
						auto end_time = std::chrono::high_resolution_clock::now();
						std::cout << "Collapsed time: " << std::chrono::duration<dtype>(end_time - start_time).count() << std::endl;
					}
					cout.flush();
				}
			}


		}

		if (m_options.maxInstance > 0 && numInstance >= m_options.maxInstance)
			break;

		if (feof(is)) break;

	}

	fclose(is);

	cout << numInstance << " " << endl;
	cout << "Total source  num: " << unsorted_source_vob.size() << endl;
	cout << "Total target num: " << unsorted_target_vob.size() << endl;
	std::cout << "Collapsed time: " << std::chrono::duration<dtype>(std::chrono::high_resolution_clock::now() - start_time).count() << std::endl;

	//step2 driver initialization
	
	w2v.m_w2v_model.init_emb(&unsorted_source_vob, &unsorted_target_vob, m_options.embSize, m_options.thread,  m_options.cutoff);
	w2v.init(m_options);

	Alphabet& source_vob = w2v.m_w2v_model.m_source_vob;
	Alphabet& target_vob = w2v.m_w2v_model.m_target_vob;
	cout << "After sorting and reducing: " << endl;
	cout << "Total source num: " << source_vob.size() << endl;
	cout << "Total target num: " << target_vob.size() << endl;

	std::cout << "Collapsed time: " << std::chrono::duration<dtype>(std::chrono::high_resolution_clock::now() - start_time).count() << std::endl;

		
	//step 3  feat file generation (can be very huge large that impossible to put all examples into memory)	
	start_time = std::chrono::high_resolution_clock::now();
	std::cout << "Starting building examples...." << std::endl;
	vector<FILE*> os(m_options.thread);
	bool newfile_succcess = true;
//#pragma omp parallel for
	for (int idx = 0; idx < m_options.thread; idx++) {
		string cur_feat_file_name = m_options.featFile + to_string((long long)idx) + ".txt";
		os[idx] = fopen(cur_feat_file_name.c_str(), "w");
		if (os[idx] == NULL) {
			cout << "Writerr::startWriting() open file err: " << cur_feat_file_name << endl;
			newfile_succcess = false;
			break;
		}
	}

	if (!newfile_succcess) {
		return;
	}

	numInstance = 0;
	w2v.m_example_num = 0;
	is = fopen(m_options.inputFile.c_str(), "r");
	while (true) {
		for (int idx = 0; idx < m_options.thread; idx++) {
			insts[idx].clear();
			insts[idx].read(is, m_options.conll);
		}

#pragma omp parallel for
		for (int idx = 0; idx < m_options.thread; idx++) {
			int sentSize = insts[idx].m_length;
			if (sentSize <= 0)continue;
			vector<SExample> sexams;
			int example_num = 0;
			get_sexamples(insts[idx], sexams);
			for (int idk = 0; idk < sexams.size(); idk++) {
				int source_id = source_vob[sexams[idk].source];
				int target_id = target_vob[sexams[idk].target];
				if (target_id == -1)continue;
				//(*os[idx]) << source_output << "\t" << target_id << std::endl;
				fprintf(os[idx], "%d\t%d\n", source_id, target_id);
				example_num++;
			}


#pragma omp critical
			{
				w2v.m_example_num += example_num;
				numInstance++;
				if ((numInstance + 1) % m_options.verboseIter == 0) {
					cout << numInstance + 1 << " ";
					if ((numInstance + 1) % (20 * m_options.verboseIter) == 0) {
						std::cout << std::endl;
						auto end_time = std::chrono::high_resolution_clock::now();
						std::cout << "Collapsed time: " << std::chrono::duration<dtype>(end_time - start_time).count() << std::endl;
					}
					cout.flush();
				}
			}
		}


		if (m_options.maxInstance > 0 && numInstance >= m_options.maxInstance)
			break;

		if (feof(is)) break;

	}


	fclose(is);
	cout << numInstance << " " << endl;
	cout << "Total example num: " << w2v.m_example_num << endl;
	std::cout << "Collapsed time: " << std::chrono::duration<dtype>(std::chrono::high_resolution_clock::now() - start_time).count() << std::endl;

//#pragma omp parallel for
	for (int idx = 0; idx < m_options.thread; idx++) {
		fclose(os[idx]);
	}
	
}



void Classifier::train() {
	w2v.m_start_time = std::chrono::high_resolution_clock::now();
	for (int idx = 0; idx < m_options.maxIter; idx++) {
		w2v.train_one_iteration(m_options.featFile, m_options.thread, m_options.maxIter);
	}

	w2v.save_model(m_options.outputSFile, m_options.outputTFile);
}


void Classifier::finish() {
	for (int idx = 0; idx < m_options.thread; idx++) {
		string cur_feat_file_name = m_options.featFile + to_string((long long)idx) + ".txt";
		remove(cur_feat_file_name.c_str());
	}
}

void Classifier::get_sexamples(const Instance& inst, vector<SExample>& sexams) {
	sexams.clear();
	int sentSize = inst.size();
	vector<string> chnchars;
	SExample exam;
	for (int idx = 0; idx < sentSize; idx++) {
		exam.source = inst.m_words[idx];
		for (int idz = idx - m_options.context; idz <= idx + m_options.context; idz++) {
			if (idz == idx) continue;
			string targetword = "<S>";
			if (idz >= 0 && idz < sentSize) targetword = inst.m_words[idz];
			exam.target = "F@" + targetword;
			if (idz >= 0 && idz < sentSize && inst.m_col == 2) exam.target = exam.target + "#" + inst.m_labels[idz][0];
			sexams.push_back(exam);
		}
		
		if(inst.m_col ==2){
			string preppos = (idx > 0) ? inst.m_labels[idx - 1][0] : "<PREV>";
			string currpos = inst.m_labels[idx][0];
			string nextpos = (idx < sentSize - 1) ? inst.m_labels[idx + 1][0] : "<LAST>";
			exam.target = "F1P@" + currpos;
			sexams.push_back(exam);
			exam.target = "F3P@" + preppos + "#" + currpos + "#" + nextpos;
			sexams.push_back(exam);
		}
		string curword = inst.m_words[idx];
		int wordlen = getCharactersFromUTF8String(curword, chnchars);
		for (int idy = 0; idy < wordlen -1; idy++) {
			string prefixword = "";
			string suffixword = "";
			for (int idz = 0; idz < wordlen; idz++) {
				if (idz <= idy) prefixword = prefixword + chnchars[idz];
				else suffixword = suffixword + chnchars[idz];
			}

			exam.source = prefixword;
			for (int idz = idx - m_options.context; idz < idx + m_options.context; idz++) {
				if (idz == idx) continue;
				string targetword = "<S>";
				if (idz >= 0 && idz < sentSize) targetword = inst.m_words[idz];
				exam.target = "F@" + targetword;
				if (idz >= 0 && idz < sentSize && inst.m_col == 2) exam.target = exam.target + "#" + inst.m_labels[idz][0];
				sexams.push_back(exam);
			}

			{
				string targetword = "P@" + suffixword;
				exam.target = "F@" + targetword;
				if (inst.m_col == 2) exam.target = exam.target + "SUF@" + inst.m_labels[idx][0];
				sexams.push_back(exam);
			}


			if (inst.m_col == 2) {
				string preppos = (idx > 0) ? inst.m_labels[idx - 1][0] : "<PREV>";
				string currpos = "PRE@" + string(inst.m_labels[idx][0]);
				string nextpos = "SUF@" + string(inst.m_labels[idx][0]);
				exam.target = "F1P@" + currpos;
				sexams.push_back(exam);
				exam.target = "F3P@" + preppos + "#" + currpos + "#" + nextpos;
				sexams.push_back(exam);
			}

		}
	}
}


int main(int argc, char* argv[]) {
    std::string optionFile = "";
    dsr::Argument_helper ah;

    ah.new_named_string("option", "optionFile", "named_string", "option file to train a model, optional when training", optionFile);

    ah.process(argc, argv);


    Classifier classifier;
	classifier.init(optionFile);
	classifier.prepare();
	classifier.train();
	classifier.finish();

}
