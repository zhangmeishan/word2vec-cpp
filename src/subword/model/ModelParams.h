/*
 * ModelParams.h
 *
 *  Created on: Jan 25, 2016
 *      Author: mszhang
 */

#ifndef SRC_ModelParams_H_
#define SRC_ModelParams_H_


#include "Alphabet.h"
#include "NRMat.h"


using namespace nr;
using namespace std;


class ModelParams {
  public:
    ModelParams() {};
    virtual ~ModelParams() {};

  public:
    Alphabet m_source_vob;
    Alphabet m_target_vob;

    NRMat<dtype> m_source_embs;
    NRMat<dtype> m_target_embs;

    int m_dim;

  public:
    void init_emb(PAlphabet p_source_vob, PAlphabet p_target_vob, int dim, int thread, int cutOff) {
        vector<int> indexes;
        p_source_vob->sort(indexes, thread, cutOff);
        m_source_vob.clear();
        int vec_size = indexes.size();
        for (int idx = 0; idx < vec_size; idx++) {
            m_source_vob.add_string(p_source_vob->m_id_to_string[indexes[idx]], p_source_vob->m_id_to_freq[indexes[idx]]);
        }

        p_target_vob->sort(indexes, thread, cutOff);
        m_target_vob.clear();
        vec_size = indexes.size();
        for (int idx = 0; idx < vec_size; idx++) {
            m_target_vob.add_string(p_target_vob->m_id_to_string[indexes[idx]], p_target_vob->m_id_to_freq[indexes[idx]]);
        }

        m_dim = dim;
        m_source_embs.resize(m_source_vob.m_size, m_dim);
        for (int i = 0; i < m_source_vob.m_size; i++) {
            for (int j = 0; j < m_dim; j++) {
                m_source_embs[i][j] = (1.0 * rand() / RAND_MAX - 0.5) / m_dim;
            }
        }

        m_target_embs.resize(m_target_vob.m_size, m_dim);
        m_target_embs = 0;
        /*
        for (int i = 0; i < m_target_vob.m_size; i++) {
        	for (int j = 0; j < m_dim; j++) {
        		m_target_embs[i][j] = (1.0 * rand() / RAND_MAX - 0.5) / m_dim;
        	}
        }*/

    }



    void save_source_model(const string& outSFile) {
        std::ofstream os(outSFile.c_str());
        for (int idx = 0; idx < m_source_vob.m_size; idx++) {
            os << m_source_vob.m_id_to_string[idx];
            for (int idy = 0; idy < m_dim; idy++) {
                os << " " << m_source_embs[idx][idy];
            }
            os << std::endl;
        }
        os.close();
    }

    void save_target_model(const string& outTFile) {
        std::ofstream os(outTFile.c_str());
        for (int idx = 0; idx < m_target_vob.m_size; idx++) {
            os << m_target_vob.m_id_to_string[idx];
            for (int idy = 0; idy < m_dim; idy++) {
                os << " " << m_target_embs[idx][idy];
            }
            os << std::endl;
        }
        os.close();
    }

    void load_source_model(const string& inSFile) {

    }

    void load_target_model(const string& inTFile) {

    }

};

#endif /* SRC_ModelParams_H_ */
