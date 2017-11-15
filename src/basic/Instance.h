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

    int read(FILE *fin, bool bConll) {
        if (bConll) {
            return readConll(fin);
        } else {
            return readNormal(fin);
        }
    }

  private:
    //input file formart: instance is separated by one empty line
    //word1 feat11 feat12
    //word2 feat21 feat22
    //......
    int readConll(FILE *fin) {
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

    //input file formart: each instance is expressed by one line
    //word1_feat11_feat12 word2_feat12_feat12  ...
    int readNormal(FILE *fin) {
        m_length = 0;
        int idx = 0, idy = 0, ch;
        while (!feof(fin)) {
            ch = fgetc(fin);
            if ((ch == '_') || (ch == ' ') || (ch == '\t') || (ch == '\r')) {
                if (idy > 0) {
                    if (idx == 0)m_words[m_length][idy] = 0;
                    else m_labels[m_length][idx - 1][idy] = 0;
                    idx++;
                    if (idx == max_feat_length + 1)idx--;
                }
                idy = 0;
                if ((ch == ' ') || (ch == '\t') || (ch == '\r')) {
                    m_length++;
                    if (idx > 0 && m_col < 0) m_col = idx;
                    idx = 0;
                    if (m_length >= max_sent_length) {
                        while (!feof(fin)) {
                            ch = fgetc(fin);
                            if (ch == '\n') {
                                break;
                            }
                        }
                        m_length = 0;
                        break;
                    }
                }
            } else if (ch == '\n') {
                break;
            } else {
                if (idx == 0)m_words[m_length][idy] = ch;
                else m_labels[m_length][idx - 1][idy] = ch;
                idy++;
                if (idy >= max_word_length - 2) idy--;
            }
        }


        return m_length;
    }

  public:
    char m_words[max_sent_length][max_word_length];
    char m_labels[max_sent_length][max_feat_length][max_word_length];
    int m_length;
    int m_col;
};

#endif /*_INSTANCE_H_*/
