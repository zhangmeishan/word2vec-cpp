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
#include "FastIO.h"

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
	vector<string> strLine;
	vector<vector<vector<string> > >  instances(m_options.thread);
	FILE* is = fopen(m_options.inputFile.c_str(), "r");
	if (is == NULL) {
		std::cout << "File Read error: " << m_options.inputFile << std::endl;
		return;
	}
	int numInstance = 0;
	Alphabet unsorted_source_vob, unsorted_target_vob;
	while (true) {
		int count = 0;
		for (int idx = 0; idx < m_options.thread; idx++) {
			instances[idx].clear();
			while (!feof(is)) {
				readline(strLine, is);
				if (strLine.empty())
					break;
				instances[idx].push_back(strLine);
			}
			count++;
			if (feof(is)) break;
		}

#pragma omp parallel for
		for (int idx = 0; idx < count; idx++) {
			Instance inst;
			int sentSize = parse_instance(instances[idx], inst);
			if (sentSize <= 0)continue;
			vector<string> sources, targets;			
			for (int idz = 0; idz < sentSize; idz++) {
				get_features(inst, sources, targets, idz, false);
			}

#pragma omp critical
			{
				for (int idz = 0; idz < sources.size(); idz++) {
					unsorted_source_vob.add_string(sources[idz]);
				}
				for (int idz = 0; idz < targets.size(); idz++) {
					unsorted_target_vob.add_string(targets[idz]);
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
		string cur_feat_file_name = m_options.featFile + obj2string(idx) + ".txt";
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
		int count = 0;
		for (int idx = 0; idx < m_options.thread; idx++) {
			instances[idx].clear();
			while (!feof(is)) {
				readline(strLine, is);
				if (strLine.empty())
					break;
				instances[idx].push_back(strLine);
			}
			count++;
			if (feof(is)) break;
		}


#pragma omp parallel for
		for (int idx = 0; idx < count; idx++) {
			Instance inst;
			int sentSize = parse_instance(instances[idx], inst);
			if (sentSize <= 0)continue;
			vector<string> sources, targets;
			int example_num = 0;
			for (int idz = 0; idz < sentSize; idz++) {
				get_features(inst, sources, targets, idz, true);

				vector<int> sourceIds;
				bool source_valid = false;			
				for (int idk = 0; idk < sources.size(); idk++) {
					int source_id = source_vob[sources[idk]];
					sourceIds.push_back(source_id);
					if (source_id >= 0)source_valid = true;
					
				}
				if (!source_valid)continue;

				string source_output = obj2string(sourceIds[0]);
				for (int idk = 1; idk < sources.size(); idk++) {
					source_output = source_output + "\t" + obj2string(sourceIds[idk]);
				}

				for (int idk = 0; idk < targets.size(); idk++) {
					int target_id = target_vob[targets[idk]];
					if (target_id == -1)continue;
					//(*os[idx]) << source_output << "\t" << target_id << std::endl;
					fprintf(os[idx], "%s\t%d\n", source_output.c_str(), target_id);
					example_num++;
				}
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
		string cur_feat_file_name = m_options.featFile + obj2string(idx) + ".txt";
		remove(cur_feat_file_name.c_str());
	}
}

void Classifier::get_features(const Instance& inst, vector<string>& sources, vector<string>& targets, int pos, bool clear) {
	if (clear) {
		sources.clear();
		targets.clear();
	}
	sources.push_back(inst.m_words[pos]);
	int sentSize = inst.size();
	for (int idy = pos - m_options.context; idy < pos + m_options.context; idy++) {
		if (idy == pos) continue;
		string featmark = "F@" + obj2string(idy - pos);
		string featvalue = "";
		if (idy < 0 || idy >= sentSize) featvalue = "</s>";
		else featvalue = inst.m_words[idy];
		targets.push_back(featmark + "=" + featvalue);
	}
}

int Classifier::parse_instance(const vector<vector<string> >& inputs, Instance& inst) {
	int sentSize = inputs.size();
	if (sentSize == 0) return 0;
	int unitsize = inputs[0].size() - 1;
	inst.allocate(sentSize, unitsize);
	for (int i = 0; i < sentSize; i++) {
		inst.m_words[i] = inputs[i][0];
		for (int idz = 0; idz < unitsize; idz++) {
			inst.m_labels[i][idz] = inputs[i][idz + 1];
		}
	}

	return sentSize;
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

	//getchar();

}
