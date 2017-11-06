/*
 * Driver.h
 *
 *  Created on: Jan 25, 2016
 *      Author: mszhang
 */

#ifndef SRC_Driver_H_
#define SRC_Driver_H_


#include "Options.h"
#include "Pipe.h"
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
	NRVec<long long> m_unitable;
	long long m_example_num;
	dtype m_alpha, m_starting_alpha;
	int m_iter;
	int m_negative;

protected:
	int m_table_size;

public:
	void init(const Options& opt) {
		//sample table
		m_table_size = 1e8;
		m_unitable.resize(m_table_size);
		const Alphabet& target_vob = m_w2v_model.m_target_vob;
		long long a, i;
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
		m_sigmoid.init(1000000);

		//
		m_alpha = opt.adaAlpha;
		m_starting_alpha = m_alpha;
		m_iter = opt.maxIter;
		m_negative = opt.negative;
	}


	void train_one_iteration(const string& featFile, int block_size, int max_iter) {
		std::ifstream is(featFile.c_str());
		vector<Example> batch_examples(block_size);
		int read_code;
		auto start_time = std::chrono::high_resolution_clock::now();
		long long run_example_num = 0;
		long long interval = m_example_num / 100;

		unsigned long long next_random = (unsigned long long)rand();
		while (true) {
			int count = 0;
			for (int idx = 0; idx < block_size; idx++) {
				read_code = batch_examples[idx].read(is);
				if (read_code == -1) break;
				count++;
			}

			//train word2vec started
#pragma omp parallel for
			for (int idx = 0; idx < count; idx++) {
				if (batch_examples[idx].sourceId == -1 || batch_examples[idx].targetId == -1) continue;
				run_example_num++;
				if ((run_example_num + 1) % interval == 0) {
					auto now_time = std::chrono::high_resolution_clock::now();
					std::cout << "iter: " << m_iter
						<< ", Alpha: " << std::setprecision(2) << m_alpha
						<< ", Progress: " << run_example_num / interval
						<< ", example num:" << run_example_num
						<< ", time: " << std::chrono::duration<dtype>(now_time - start_time).count()
						<< std::endl;
					fflush(stdout);
				}

				m_alpha = m_starting_alpha * (1 - run_example_num / (dtype)(max_iter*m_example_num + 1));
				if (m_alpha < m_starting_alpha * 0.0001) m_alpha = m_starting_alpha * 0.0001;

				long long wrdi = batch_examples[idx].sourceId;
				long long ctxi = batch_examples[idx].targetId;

				/* not so useful
				if (m_sample > 0) {
					dtype ran = (sqrt(m_source_vob.m_id_to_freq[wrdi] / (m_sample * m_source_vob.m_allword_count)) + 1) * (m_sample * m_source_vob.m_allword_count) / m_source_vob.m_id_to_freq[wrdi];
					next_random = next_random * (unsigned long long)25214903917 + 11;
					if (ran < (next_random & 0xFFFF) / (dtype)65536) continue;
					ran = (sqrt(m_target_vob.m_id_to_freq[ctxi] / (m_sample * m_target_vob.m_allword_count)) + 1) * (m_sample * m_target_vob.m_allword_count) / m_target_vob.m_id_to_freq[ctxi];
					next_random = next_random * (unsigned long long)25214903917 + 11;
					if (ran < (next_random & 0xFFFF) / (dtype)65536) continue;
				}*/
				const Alphabet& source_vob = m_w2v_model.m_source_vob;
				const Alphabet& target_vob = m_w2v_model.m_target_vob;
				NRMat<dtype>& source_embs = m_w2v_model.m_source_embs;
				NRMat<dtype>& target_embs = m_w2v_model.m_target_embs;
				int dim = m_w2v_model.m_dim;

				long long target, label;
				NRVec<dtype> neu1e(dim);
				neu1e = 0;
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
					for (int c = 0; c < dim; c++) f += source_embs[wrdi][c] * target_embs[target][c];
					g = (label - m_sigmoid.get(f)) * m_alpha;
					for (int c = 0; c < dim; c++) neu1e[c] += g * target_embs[target][c];
					for (int c = 0; c < dim; c++) target_embs[target][c] += g * source_embs[wrdi][c];
				}
				// Learn weights input -> hidden
				for (int c = 0; c < dim; c++) source_embs[wrdi][c] += neu1e[c];

			}

			if (read_code == -1) break;

		}

		is.close();
		m_iter++;
	}

	void save_model(const string& outSFile, const string& outTFile) {
		m_w2v_model.save_source_model(outSFile);
		m_w2v_model.save_target_model(outTFile);
	}

};

#endif /* SRC_Driver_H_ */
