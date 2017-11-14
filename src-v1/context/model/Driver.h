/*
 * Driver.h
 *
 *  Created on: Jan 25, 2016
 *      Author: mszhang
 */

#ifndef SRC_Driver_H_
#define SRC_Driver_H_


#include "Options.h"
#include "Instance.h"
#include "Utf.h"
#include "QuickSigmoid.h"
#include "ModelParams.h"
#include "Example.h"
#include <chrono>
#include <iomanip>

using namespace nr;
using namespace std;


class Driver {
public:
	Driver() {};
	virtual ~Driver() {};

public:
	ModelParams m_w2v_model;

	QuickSigmoid m_sigmoid;
	NRVec<int> m_unitable;
	long long m_example_num;
	dtype m_alpha, m_starting_alpha;
	int m_iter;
	int m_negative;
	dtype m_sample;
	int m_interval;
	std::chrono::high_resolution_clock::time_point m_start_time;

protected:
	int m_table_size;
	long long m_actual_word_count;
	

public:
	void init(const Options& opt) {
		//sample table
		m_table_size = 1e8;
		m_unitable.resize(m_table_size);
		const Alphabet& target_vob = m_w2v_model.m_target_vob;
		int a, i;
		long long normalizer = 0;
		dtype d1, power = 0.75;
		for (a = 0; a < target_vob.m_size; a++) normalizer += pow(target_vob.m_id_to_freq[a], power);
		i = 0;
		d1 = pow(target_vob.m_id_to_freq[i], power) / (dtype)normalizer;
		for (a = 0; a < m_table_size; a++) {
			m_unitable[a] = i;
			if (a / (dtype)m_table_size > d1) {
				i++;
				d1 += pow(target_vob.m_id_to_freq[i], power) / (dtype)normalizer;
			}
			if (i >= target_vob.m_size) i = target_vob.m_size - 1;
		}
		//
		m_sigmoid.init(100000);

		//
		m_alpha = opt.adaAlpha;
		m_sample = opt.sample;
		m_starting_alpha = m_alpha;
		m_iter = 0;
		m_negative = opt.negative;
		m_interval = 20 * opt.verboseIter;
		m_actual_word_count = 0;
		m_start_time = std::chrono::high_resolution_clock::now();
	}


	void train_one_iteration(const string& featFile, int block_size, int max_iter) {	
		unsigned long long next_random = (unsigned long long)rand();
#pragma omp parallel for
		for (int idx = 0; idx < block_size; idx++) {
			string cur_feat_file_name = featFile + obj2string(idx) + ".txt";
			FILE* is = fopen(cur_feat_file_name.c_str(), "r");
			if (is == NULL) {
				cout << "Reader::startReading() open file err: " << cur_feat_file_name << endl;
				continue;
			}
			long long run_example_num = 0;
			long long last_word_count = 0;
			Example exam;
			while (exam.read(is) != -1) {	
#pragma omp critical
					{
						if (run_example_num - last_word_count > m_interval) {
							m_actual_word_count += run_example_num - last_word_count;
							last_word_count = run_example_num;
							auto now_time = std::chrono::high_resolution_clock::now();
							std::cout << "iter: " << m_iter
								<< ",\tAlpha: " << std::setprecision(2) << m_alpha
								<< ",\tProgress: " << std::setprecision(2) << m_actual_word_count / (dtype)(max_iter*m_example_num + 1) * 100
								//<< ", example num:" << m_actual_word_count
								<< ",\ttime: " << std::chrono::duration<dtype>(now_time - m_start_time).count()
								<< std::endl;
							fflush(stdout);

							m_alpha = m_starting_alpha * (1 - m_actual_word_count / (dtype)(max_iter*m_example_num + 1));
							if (m_alpha < m_starting_alpha * 0.0001) m_alpha = m_starting_alpha * 0.0001;
						}
					}

				int wrdi = exam.sourceId;
				int ctxi = exam.targetId;
				run_example_num++;

				const Alphabet& source_vob = m_w2v_model.m_source_vob;
				const Alphabet& target_vob = m_w2v_model.m_target_vob;
				NRMat<dtype>& source_embs = m_w2v_model.m_source_embs;
				NRMat<dtype>& target_embs = m_w2v_model.m_target_embs;

			
				if (wrdi == -1 || ctxi == -1) continue;
				if (m_sample > 0) {
					dtype ran = (sqrt(source_vob.m_id_to_freq[wrdi] / (m_sample * source_vob.m_allword_count)) + 1) * (m_sample * source_vob.m_allword_count) / source_vob.m_id_to_freq[wrdi];
					next_random = next_random * (unsigned long long)25214903917 + 11;
					if (ran < (next_random & 0xFFFF) / (dtype)65536) continue;
					ran = (sqrt(target_vob.m_id_to_freq[ctxi] / (m_sample * target_vob.m_allword_count)) + 1) * (m_sample * target_vob.m_allword_count) / target_vob.m_id_to_freq[ctxi];
					next_random = next_random * (unsigned long long)25214903917 + 11;
					if (ran < (next_random & 0xFFFF) / (dtype)65536) continue;
				}

				int dim = m_w2v_model.m_dim;

				long long target, label;
				NRVec<dtype> neu1e(dim);
				neu1e = 0;
				dtype* cur_source_emb = source_embs[wrdi];
				for (int d = 0; d < m_negative + 1; d++) {
					if (d == 0) {
						target = ctxi;
						label = 1;
					}
					else {
						next_random = next_random * (unsigned long long)25214903917 + 11;
						target = m_unitable[(next_random >> 16) % m_table_size];
						if (target == 0) target = next_random % (target_vob.m_size - 1) + 1;
						if (target == ctxi) continue;
						label = 0;
					}
					dtype f = 0, g = 0;
					dtype* cur_target_emb = target_embs[target];
					for (int c = 0; c < dim; c++) f += cur_source_emb[c] * cur_target_emb[c];
					g = (label - m_sigmoid.get(f)) * m_alpha;
					for (int c = 0; c < dim; c++) neu1e[c] += g * cur_target_emb[c];
					for (int c = 0; c < dim; c++) cur_target_emb[c] += g * cur_source_emb[c];
				}
				// Learn weights input -> hidden
				for (int c = 0; c < dim; c++) cur_source_emb[c] += neu1e[c];
			}

#pragma omp critical
			if (run_example_num - last_word_count > 0) {
				m_actual_word_count += run_example_num - last_word_count;
				last_word_count = run_example_num;
				auto now_time = std::chrono::high_resolution_clock::now();
				std::cout << "iter: " << m_iter
					<< ",\tAlpha: " << std::setprecision(2) << m_alpha
					<< ",\tProgress: " << std::setprecision(2) << m_actual_word_count / (dtype)(max_iter*m_example_num + 1) * 100
					//<< ", example num:" << m_actual_word_count
					<< ",\ttime: " << std::chrono::duration<dtype>(now_time - m_start_time).count()
					<< std::endl;
				fflush(stdout);

				m_alpha = m_starting_alpha * (1 - m_actual_word_count / (dtype)(max_iter*m_example_num + 1));
				if (m_alpha < m_starting_alpha * 0.0001) m_alpha = m_starting_alpha * 0.0001;
			}

			fclose(is);
		}


		
		m_iter++;
	}

	void save_model(const string& outSFile, const string& outTFile) {
		m_w2v_model.save_source_model(outSFile);
		m_w2v_model.save_target_model(outTFile);
	}

};

#endif /* SRC_Driver_H_ */
