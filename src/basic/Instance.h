#ifndef _INSTANCE_H_
#define _INSTANCE_H_

#include "MyLib.h"
using namespace std;

class Instance {
  public:
    void clear() {
        //memset(m_words, 0, 256 * 128 * sizeof(char));
        //memset(m_labels, 0, 256 * 256 * 128 * sizeof(char));
        m_col = -1;
        m_length = 0;
    }

    int size() const {
        return m_length;
    }

    int read(FILE *fin) {
        m_length = 0;
        while (readline(m_length, fin) > 0) {
            m_length++;
            if (feof(fin)) {
                break;
            }

            //discard the sentence
            if (m_length >= max_sent_length) {
                while (readline(0, fin) > 0) {
                }
                m_length = 0;
                return 0;
            }
        }

        return m_length;
    }

  private:
    int readline(int pos, FILE *fin) {
        int idx = 0, idy = 0, ch;
        while (!feof(fin)) {
            ch = fgetc(fin);
            if ((ch == ' ') || (ch == '\t') || (ch == '\n') || (ch == '\r')) {
                if (idy > 0) {
                    if (idx == 0)m_words[pos][idy] = 0;
                    else m_labels[pos][idx - 1][idy] = 0;
                    idx++;
                    if (idx == max_feat_length + 1)idx--;
                }
                idy = 0;
                if ((ch == '\n'))break;
                else continue;
            }
            if (idx == 0)m_words[pos][idy] = ch;
            else m_labels[pos][idx - 1][idy] = ch;
            idy++;
            if (idy >= max_word_length - 2) idy--;
        }

        if (idx > 0 && m_col < 0) m_col = idx;

        return idx;
    }

  public:
    char m_words[max_sent_length][max_word_length];
    char m_labels[max_sent_length][max_feat_length][max_word_length];
    int m_length;
    int m_col;
};

#endif /*_INSTANCE_H_*/
