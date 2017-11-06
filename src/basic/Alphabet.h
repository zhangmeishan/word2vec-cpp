#ifndef _ALPHABET_
#define _ALPHABET_

#include "MyLib.h"
#include "ParaSort.h"

/*
 please check to ensure that m_size not exceeds the upbound of long long
 */

/*
  This class serializes feature from string to long long.
  Index starts from 0.
*/

/**
 * The basic class of quark class.
 *  @param  std::string        String class name to be used.
 *  @param  long long         ID class name to be used.
 *  @author Naoaki Okazaki
 */
class basic_quark {
  public:
    typedef unordered_map<std::string, long long> StringToId;
    typedef std::vector<std::string> IdToString;
	typedef std::vector<long long> IdToFreq;

    StringToId m_string_to_id;
    IdToString m_id_to_string;
	IdToFreq   m_id_to_freq;
    long long m_size;
	long long m_max_freq;
	long long m_allword_count;

  public:
    /**
     * Construct.
     */
    basic_quark() {
        clear();
    }

    /**
     * Destruct.
     */
    virtual ~basic_quark() {
    }

    /**
     * Map a string to its associated ID.
     *  If string-to-long longeger association does not exist, allocate a new ID.
     *  @param  str         String value.
     *  @return           Associated ID for the string value.
     */
    long long operator[](const std::string& str) {
        StringToId::const_iterator it = m_string_to_id.find(str);
        if (it != m_string_to_id.end()) {
            return it->second;
        } else {
            return -1;
        }
    }


    /**
     * Convert ID value long longo the associated string value.
     *  @param  qid         ID.
     *  @param  def         Default value if the ID was out of range.
     *  @return           String value associated with the ID.
     */
    const std::string& from_id(const long long& qid, const std::string& def = "") const {
        if (qid < 0 || m_size <= qid) {
            return def;
        } else {
            return m_id_to_string[qid];
        }
    }



    /**
     * Convert string value long longo the associated ID value.
     *  @param  str         String value.
     *  @return           ID if any, otherwise -1.
     */
    long long add_string(const std::string& str, long long freq = 1) {
        StringToId::const_iterator it = m_string_to_id.find(str);
        if (it != m_string_to_id.end()) {
			long long qid = it->second;
			m_id_to_freq[qid] = m_id_to_freq[qid] + freq;
			if(m_id_to_freq[qid] > m_max_freq) m_max_freq = m_id_to_freq[qid];
			m_allword_count += freq;
            return it->second;
        } else {
            long long newid = m_size;
            m_id_to_string.push_back(str);
			m_id_to_freq.push_back(freq);
			if(m_size == 0) m_max_freq = freq;
			m_allword_count += freq;
            m_string_to_id.insert(std::pair<std::string, long long>(str, newid));
            m_size++;
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
        long long featId, featFreq;
        inf >> m_size;
        for (long long i = 0; i < m_size; ++i) {
            inf >> featKey >> featFreq >> featId;
            m_string_to_id[featKey] = i;
            m_id_to_string.push_back(featKey);
            m_id_to_freq.push_back(featFreq);
            assert(featId == i);
        }
    }

    void write(std::ofstream &outf) const {
        outf << m_size << std::endl;
        for (long long i = 0; i < m_size; i++) {
            outf << m_id_to_string[i] << " " << m_id_to_freq[i] << " " << i << std::endl;
        }
    }
	
	void sort(vector<long long>& indexes, int thread, long long freqcut) const {
		long long *allIndexes = new long long[m_size];
		long long *allFreqs = new long long[m_size];
		long long count = 0;
		for (long long idx = 0; idx < m_size; idx++) {
			if (m_id_to_freq[idx] <= freqcut) continue;
			allIndexes[count] = idx;
			allFreqs[count] = m_id_to_freq[idx];
			count++;
		}		
		parallel_radix_sort::SortPairs(allFreqs, allIndexes, count, thread);
		indexes.resize(count);
		for (long long idx = 0; idx < count; idx++) {
			indexes[idx] = allIndexes[count - idx -1];
		}

		delete[] allIndexes;
		delete[] allFreqs;
	}
};

typedef basic_quark Alphabet;
typedef basic_quark*  PAlphabet;

#endif

