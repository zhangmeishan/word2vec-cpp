#ifndef _ALPHABET_
#define _ALPHABET_

#include "MyLib.h"
#include "ParaSort.h"

/*
 please check to ensure that m_size not exceeds the upbound of int
 */

/*
  This class serializes feature from string to int.
  Index starts from 0.
*/

/**
 * The basic class of quark class.
 *  @param  std::string        String class name to be used.
 *  @param  int         ID class name to be used.
 *  @author Naoaki Okazaki
 */
class basic_quark {
  public:
    typedef unordered_map<std::string, int> StringToId;
    typedef std::vector<std::string> IdToString;
	typedef std::vector<int> IdToFreq;

    StringToId m_string_to_id;
    IdToString m_id_to_string;
	IdToFreq   m_id_to_freq;
    int m_size;
	int m_max_freq;
	long long m_allword_count;

  protected:
	int m_max_size;
	int m_reduce_threshold;

  public:
    /**
     * Construct.
     */
    basic_quark() {
        clear();
		m_reduce_threshold = 1;
		m_max_size = 2000000000;
		//m_max_size = 100000;
    }

    /**
     * Destruct.
     */
    virtual ~basic_quark() {
    }

    /**
     * Map a string to its associated ID.
     *  If string-to-integer association does not exist, allocate a new ID.
     *  @param  str         String value.
     *  @return           Associated ID for the string value.
     */
    int operator[](const std::string& str) {
        StringToId::const_iterator it = m_string_to_id.find(str);
        if (it != m_string_to_id.end()) {
            return it->second;
        } else {
            return -1;
        }
    }


    /**
     * Convert ID value into the associated string value.
     *  @param  qid         ID.
     *  @param  def         Default value if the ID was out of range.
     *  @return           String value associated with the ID.
     */
    const std::string& from_id(const int& qid, const std::string& def = "") const {
        if (qid < 0 || m_size <= qid) {
            return def;
        } else {
            return m_id_to_string[qid];
        }
    }



    /**
     * Convert string value into the associated ID value.
     *  @param  str         String value.
     *  @return           ID if any, otherwise -1.
     */
    int add_string(const std::string& str, int freq = 1) {
        StringToId::const_iterator it = m_string_to_id.find(str);
        if (it != m_string_to_id.end()) {
			int qid = it->second;
			m_id_to_freq[qid] = m_id_to_freq[qid] + freq;
			if(m_id_to_freq[qid] > m_max_freq) m_max_freq = m_id_to_freq[qid];
			m_allword_count += freq;
            return it->second;
        } else {
            int newid = m_size;
            m_id_to_string.push_back(str);
			m_id_to_freq.push_back(freq);
			if(m_size == 0) m_max_freq = freq;
			m_allword_count += freq;
            m_string_to_id.insert(std::pair<std::string, int>(str, newid));
            m_size++;
			if (m_size >= m_max_size) {
				reduce();
			}
            return newid;
        }
    }

    void clear() {
        m_string_to_id.clear();
        m_id_to_string.clear();
		m_id_to_freq.clear();
        m_size = 0;
		m_max_freq = 0;
		m_allword_count = 0;
    }

    /**
     * Get the number of string-to-id associations.
     *  @return           The number of association.
     */
    size_t size() const {
        return m_size;
    }


    void read(std::ifstream &inf) {
        clear();
        string featKey;
        int featId, featFreq;
        inf >> m_size;
        for (int i = 0; i < m_size; ++i) {
            inf >> featKey >> featFreq >> featId;
            m_string_to_id[featKey] = i;
            m_id_to_string.push_back(featKey);
            m_id_to_freq.push_back(featFreq);
            assert(featId == i);
        }
    }

    void write(std::ofstream &outf) const {
        outf << m_size << std::endl;
        for (int i = 0; i < m_size; i++) {
            outf << m_id_to_string[i] << " " << m_id_to_freq[i] << " " << i << std::endl;
        }
    }
	
	void sort(vector<int>& indexes, int thread, int freqcut) const {
		int *allIndexes = new int[m_size];
		int *allFreqs = new int[m_size];
		int count = 0;
		for (int idx = 0; idx < m_size; idx++) {
			if (m_id_to_freq[idx] <= freqcut) continue;
			allIndexes[count] = idx;
			allFreqs[count] = m_id_to_freq[idx];
			count++;
		}		
		parallel_radix_sort::SortPairs(allFreqs, allIndexes, count, thread);
		indexes.resize(count);
		for (int idx = 0; idx < count; idx++) {
			indexes[idx] = allIndexes[count - idx -1];
		}

		delete[] allIndexes;
		delete[] allFreqs;
	}

protected:
	void reduce() {
		//std::cout << "Reaching max size, reducing low-frequency items" << std::endl;
		//std::cout << "Current Size: " << m_size << std::endl;
		StringToId tmp_string_to_id;

#pragma omp parallel for
		for (int idx = 0; idx < m_size; idx++) {
			if (m_id_to_freq[idx] <= m_reduce_threshold) continue;
			tmp_string_to_id[m_id_to_string[idx]] = m_id_to_freq[idx];
		}

		clear();

		for (StringToId::const_iterator it = tmp_string_to_id.begin(); it != tmp_string_to_id.end(); it++) {
			add_string(it->first, it->second);
		}

		//std::cout << "Remain Size: " << m_size << std::endl;
		std::cout << "(" << m_size  << ")" << " ";
		std::cout.flush();
		m_reduce_threshold++;
	}
};

typedef basic_quark Alphabet;
typedef basic_quark*  PAlphabet;

#endif

