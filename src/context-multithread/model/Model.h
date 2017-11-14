/*
 * Model.h
 *
 *  Created on: Jan 25, 2016
 *      Author: mszhang
 */

#ifndef SRC_MODEL_H_
#define SRC_MODEL_H_


#include "Options.h"
#include "Pipe.h"
#include "Utf.h"
#include "NRMat.h"
#include "Alphabet.h"
#include "QuickSigmoid.h"
#include "Example.h"
#include <chrono>
#include <iomanip>
#include "ThreadPool.h"

using namespace nr;
using namespace std;


class Model {
  public:
    Model() {};
    virtual ~Model() {};

  public:
    Alphabet m_source_vob;
    Alphabet m_target_vob;
    QuickSigmoid m_sigmoid;
    NRVec<int> m_unitable;

    NRMat<dtype> m_source_embs;
    NRMat<dtype> m_target_embs;

    int m_dim;
    long long m_example_num;
    dtype m_alpha;
    int m_iter;


    std::ifstream m_file;
    int m_block_size;
    dtype m_distance;


    vector<Example> m_block_examples;
    long long m_run_example_num;
    long long m_interval;
    std::chrono::high_resolution_clock::time_point m_start_time;

  protected:
    int m_table_size;

  public:
    void init_unitable() {
        m_table_size = 1e8;
        m_unitable.resize(m_table_size);
        int a, i;
        long long normalizer = 0;
        dtype d1, power = 0.75;
        for (a = 0; a < m_target_vob.m_size; a++) normalizer += pow(m_target_vob.m_id_to_freq[a], power);
        i = 0;
        d1 = pow(m_target_vob.m_id_to_freq[i], power) / (dtype)normalizer;
        for (a = 0; a < m_table_size; a++) {
            m_unitable[a] = i;
            if (a / (dtype)m_table_size > d1) {
                i++;
                d1 += pow(m_target_vob.m_id_to_freq[i], power) / (dtype)normalizer;
            }
            if (i >= m_target_vob.m_size) i = m_target_vob.m_size - 1;
        }
    }

    void init_emb(int dim) {
        m_dim = dim;
        m_source_embs.resize(m_source_vob.m_size, m_dim);
        for (int i = 0; i < m_source_vob.m_size; i++) {
            for (int j = 0; j < m_dim; j++) {
                m_source_embs[i][j] = (1.0 * rand() / RAND_MAX - 0.5) / m_dim;
            }
        }

        m_target_embs.resize(m_target_vob.m_size, m_dim);
        //m_target_embs = 0;
        for (int i = 0; i < m_target_vob.m_size; i++) {
            for (int j = 0; j < m_dim; j++) {
                m_target_embs[i][j] = (1.0 * rand() / RAND_MAX - 0.5) / m_dim;
            }
        }

        //
        m_iter = 0;
        m_alpha = 0;
        m_sigmoid.init(1000000);
    }


    void train_one_iteration(const string& featFile, int block_size) {
        std::ifstream is(featFile.c_str());
        vector<Example> batch_examples(block_size);
        int read_sgn = 0;
        int read_code = 0;
        auto start_time = std::chrono::high_resolution_clock::now();
        long long run_example_num = 0;
        long long interval = m_example_num / 100;
        dtype distance = 0.0;
        while (read_sgn != -1) {
            for (int idx = 0; idx < block_size; idx++) {
                read_code = batch_examples[idx].read(is);
                if (read_code == -1) {
                    read_sgn = -1;
                    break;
                }
            }

            //train word2vec started
//#pragma omp parallel for
            for (int idx = 0; idx < block_size; idx++) {
                if (batch_examples[idx].sourceId == -1 || batch_examples[idx].targetId == -1) continue;
                run_example_num++;
                if ((run_example_num + 1) %  interval == 0) {
                    auto now_time = std::chrono::high_resolution_clock::now();
                    std::cout << "iter: " << m_iter
                              << ", Alpha: " << std::setprecision(2) << m_alpha
                              << ", Progress: " << run_example_num / interval
                              << ", example num:" << run_example_num
                              << ", time: " << std::chrono::duration<dtype>(now_time - start_time).count()
                              << std::endl;
                    fflush(stdout);
                }

                int source = batch_examples[idx].sourceId;
                int target = batch_examples[idx].targetId;

                for (int dup = 0; dup < 100; dup++) {
                    dtype sum = 0.0;
                    for (int idz = 0; idz < m_dim; idz++) {
                        sum += m_source_embs[source][idz] * m_target_embs[target][idz];
                        dtype a = m_sigmoid.get(sum);
                        dtype b = 1.0 / (1 + exp(-sum));
                        distance += (a - b) * (a - b);

                        if (abs(a - b) > 0.0001) {
                            std::cout << "Single distance bigger than 0.01" << std::endl;
                        }
                    }
                }

                if (distance > 1000) {
                    std::cout << "Sum distance bigger than 1000" << std::endl;
                    distance = 0;
                }
            }

        }

        is.close();
        m_iter++;
    }

    void save_model(const string& outSFile, const string& outTFile) {

    }

    //
    void init_block(int block_size) {
        //m_file.open(featFile.c_str());
        m_block_size = block_size;
        m_block_examples.resize(m_block_size);
    }


    int process_one_example(int i) {
        m_run_example_num++;
        if ((m_run_example_num + 1) % m_interval == 0) {
            auto now_time = std::chrono::high_resolution_clock::now();
            std::cout << "iter: " << m_iter
                      << ", Alpha: " << std::setprecision(2) << m_alpha
                      << ", Progress: " << m_run_example_num / m_interval
                      << ", example num:" << m_run_example_num
                      << ", time: " << std::chrono::duration<dtype>(now_time - m_start_time).count()
                      << std::endl;
            fflush(stdout);
        }

        int source = m_block_examples[i].sourceId;
        int target = m_block_examples[i].targetId;

        for (int dup = 0; dup < 100; dup++) {
            dtype sum = 0.0;
            for (int idz = 0; idz < m_dim; idz++) {
                sum += m_source_embs[source][idz] * m_target_embs[target][idz];
                dtype a = m_sigmoid.get(sum);
                dtype b = 1.0 / (1 + exp(-sum));
                m_distance += (a - b) * (a - b);

                if (abs(a - b) > 0.0001) {
                    std::cout << "Single distance bigger than 0.01" << std::endl;
                }
            }
        }

        return 1;
    }

    void train_one_iteration_thread(const string& featFile, int thread_num) {
        m_start_time = std::chrono::high_resolution_clock::now();
        std::ifstream is(featFile.c_str());
        m_run_example_num = 0;
        int read_code;
        while (true) {
            int count = 0;
            for (int idx = 0; idx < m_block_size; idx++) {
                int read_code = m_block_examples[idx].read(is);
                if (read_code == -1) {
                    break;
                }
                count++;
            }

            ThreadPool threads(thread_num);

            for (int idx = 0; idx < count; idx++) {
                if (m_block_examples[idx].sourceId == -1 || m_block_examples[idx].targetId == -1) continue;
                threads.enqueue(&Model::process_one_example, this, idx);
            }


            if (read_code == -1)break;
        }
    }

};

#endif /* SRC_MODEL_H_ */
