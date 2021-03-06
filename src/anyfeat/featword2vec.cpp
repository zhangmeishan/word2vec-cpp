/*
 * Classifier.cpp
 *
 *  Created on: Jan 25, 2016
 *      Author: mszhang
 */

#include "featword2vec.h"
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
					for (int idk = 0; idk < sexams[idz].source.size(); idk++) {
						unsorted_source_vob.add_string(sexams[idz].source[idk]);
					}
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
			for (int idz = 0; idz < sexams.size(); idz++) {
				string outsource = "";
				int source_valid_count = 0;
				for (int idk = 0; idk < sexams[idz].source.size(); idk++) {
					int source_id = source_vob[sexams[idz].source[idk]];
					if (source_id >= 0) {
						source_valid_count++;
						outsource = outsource + to_string((long long)source_id) + "\t";
					}
				}
				int target_id = target_vob[sexams[idz].target];
				if (source_valid_count < 1 ||target_id == -1)continue;
				//(*os[idx]) << source_output << "\t" << target_id << std::endl;
				fprintf(os[idx], "%s%d\n", outsource.c_str(), target_id);
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
	for (int idy = 0; idy < sentSize; idy++) {		
		string curword = inst.m_words[idy];
		int wordlen = getCharactersFromUTF8String(curword, chnchars);
		exam.source.clear();

		int ngram = m_options.mingram;
		chnchars.insert(chnchars.begin(), "<");
		chnchars.push_back(">");
		int charsize = chnchars.size();
		while (ngram <= m_options.maxgram) {
			for (int idz = 0; idz <= charsize - ngram; idz++) {
				string featValue = "N@"+ to_string((long long)ngram) + "#";
				for (int c = idz; c < idz + ngram; c++) {
					featValue = featValue + chnchars[c];
				}
				exam.source.push_back(featValue);
			}
			ngram++;
		}

		for (int idz = idy - m_options.context; idz <= idy + m_options.context; idz++) {
			if (idz == idy) continue;
			string featmark = "C@" + to_string((long long)(idz - idy));
			string featvalue = "";
			if (idz < 0 || idz >= sentSize) featvalue = "</s>";
			else featvalue = inst.m_words[idz];
			featvalue = featmark + "=" + featvalue;
			exam.source.push_back(featvalue);			
		}

		{
			string prev_word = idy > 0 ? inst.m_words[idy-1] : "</s>";
			string next_word = (idy + 1 < sentSize) ? inst.m_words[idy + 1] : "</s>";
			exam.source.push_back("C2@" + prev_word + "#" + next_word);
		}

		exam.target = curword;	
		sexams.push_back(exam);
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
