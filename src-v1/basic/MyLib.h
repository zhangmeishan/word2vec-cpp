#ifndef _MYLIB_H_
#define _MYLIB_H_

#include <string>
#include <vector>
#include <fstream>
#include <cassert>
#include <deque>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <cmath>
#include <ctime>
#include <cfloat>
#include <cstring>
#include <sstream>
#include <unordered_map>
#include <unordered_set>


#if USE_FLOAT
typedef float dtype;
#else
typedef double dtype;
#endif

typedef long long blong;
using namespace std;

const static dtype minlogvalue = -1000;
const static dtype d_zero = 0.0;
const static dtype d_one = 1.0;
const static string nullkey = "-NULL-";
const static string unknownkey = "-UNKNOWN-";
const static string seperateKey = "#";
const static int max_length = 512;
const static int max_word_length = 16;
const static int max_stroke_length = 64;
const static int max_spell_length = 16;
const static string path_separator =
#ifdef _WIN32
    "\\";
#else
    "/";
#endif


inline int cmpIntIntPairByValue(const pair<int, int> &x, const pair<int, int> &y) {
    return x.second > y.second;
}

inline void sortMapbyValue(const unordered_map<int, int> &t_map, vector<pair<int, int> > &t_vec) {
    t_vec.clear();

    for (unordered_map<int, int>::const_iterator iter = t_map.begin(); iter != t_map.end(); iter++) {
        t_vec.push_back(make_pair(iter->first, iter->second));
    }
    std::sort(t_vec.begin(), t_vec.end(), cmpIntIntPairByValue);
}

inline void replace_char_by_char(string &str, char c1, char c2) {
    string::size_type pos = 0;
    for (; pos < str.size(); ++pos) {
        if (str[pos] == c1) {
            str[pos] = c2;
        }
    }
}

inline void split_bychars(const string& str, vector<string> & vec, const char *sep = " ") { //assert(vec.empty());
    vec.clear();
    string::size_type pos1 = 0, pos2 = 0;
    string word;
    while ((pos2 = str.find_first_of(sep, pos1)) != string::npos) {
        word = str.substr(pos1, pos2 - pos1);
        pos1 = pos2 + 1;
        if (!word.empty())
            vec.push_back(word);
    }
    word = str.substr(pos1);
    if (!word.empty())
        vec.push_back(word);
}

// remove the blanks at the begin and end of string
inline void clean_str(string &str) {
    string blank = " \t\r\n";
    string::size_type pos1 = str.find_first_not_of(blank);
    string::size_type pos2 = str.find_last_not_of(blank);
    if (pos1 == string::npos) {
        str = "";
    } else {
        str = str.substr(pos1, pos2 - pos1 + 1);
    }
}

inline bool my_getline(ifstream &inf, string &line) {
    if (!getline(inf, line))
        return false;
    int end = line.size() - 1;
    while (end >= 0 && (line[end] == '\r' || line[end] == '\n')) {
        line.erase(end--);
    }

    return true;
}

inline void str2uint_vec(const vector<string> &vecStr, vector<unsigned int> &vecInt) {
    vecInt.resize(vecStr.size());
    int i = 0;
    for (; i < vecStr.size(); ++i) {
        vecInt[i] = atoi(vecStr[i].c_str());
    }
}

inline void str2int_vec(const vector<string> &vecStr, vector<int> &vecInt) {
    vecInt.resize(vecStr.size());
    int i = 0;
    for (; i < vecStr.size(); ++i) {
        vecInt[i] = atoi(vecStr[i].c_str());
    }
}

template<typename A>
inline string obj2string(const A& a) {
    ostringstream out;
    out << a;
    return out.str();
}

template<typename A>
inline void obj2str_vec(const vector<A> &vecObjs, vector<string> &vecStr) {
    vecStr.resize(vecObjs.size());
    int i = 0;
    for (; i < vecObjs.size(); ++i) {
        ostringstream out;
        out << vecObjs[i];
        vecStr[i] = out.str();
    }
}

inline void join_bystr(const vector<string> &vec, string &str, const string &sep) {
    str = "";
    if (vec.empty())
        return;
    str = vec[0];
    int i = 1;
    for (; i < vec.size(); ++i) {
        str += sep + vec[i];
    }
}

inline void split_bystr(const string &str, vector<string> &vec, const string &sep) {
    vec.clear();
    string::size_type pos1 = 0, pos2 = 0;
    string word;
    while ((pos2 = str.find(sep, pos1)) != string::npos) {
        word = str.substr(pos1, pos2 - pos1);
        pos1 = pos2 + sep.size();
        if (!word.empty())
            vec.push_back(word);
    }
    word = str.substr(pos1);
    if (!word.empty())
        vec.push_back(word);
}

inline void split_pair_vector(const vector<pair<int, string> > &vecPair, vector<int> &vecInt, vector<string> &vecStr) {
	int i = 0;
	vecInt.resize(vecPair.size());
	vecStr.resize(vecPair.size());
	for (; i < vecPair.size(); ++i) {
		vecInt[i] = vecPair[i].first;
		vecStr[i] = vecPair[i].second;
	}
}

inline void split_bychar(const string& str, vector<string>& vec, const char separator = ' ') {
	//assert(vec.empty());
	vec.clear();
	string::size_type pos1 = 0, pos2 = 0;
	string word;
	while ((pos2 = str.find_first_of(separator, pos1)) != string::npos) {
		word = str.substr(pos1, pos2 - pos1);
		pos1 = pos2 + 1;
		if (!word.empty())
			vec.push_back(word);
	}
	word = str.substr(pos1);
	if (!word.empty())
		vec.push_back(word);
}

inline void string2pair(const string& str, pair<string, string>& pairStr, const char separator = '/') {
	string::size_type pos = str.find_last_of(separator);
	if (pos == string::npos) {
		string tmp = str + "";
		clean_str(tmp);
		pairStr.first = tmp;
		pairStr.second = "";
	}
	else {
		string tmp = str.substr(0, pos);
		clean_str(tmp);
		pairStr.first = tmp;
		tmp = str.substr(pos + 1);
		clean_str(tmp);
		pairStr.second = tmp;
	}
}

inline void convert_to_pair(vector<string>& vecString, vector<pair<string, string> >& vecPair) {
	assert(vecPair.empty());
	int size = vecString.size();
	string::size_type cur;
	string strWord, strPos;
	for (int i = 0; i < size; ++i) {
		cur = vecString[i].find('/');

		if (cur == string::npos) {
			strWord = vecString[i].substr(0);
			strPos = "";
		}
		else if (cur == vecString[i].size() - 1) {
			strWord = vecString[i].substr(0, cur);
			strPos = "";
		}
		else {
			strWord = vecString[i].substr(0, cur);
			strPos = vecString[i].substr(cur + 1);
		}

		vecPair.push_back(pair<string, string>(strWord, strPos));
	}
}

inline void split_to_pair(const string& str, vector<pair<string, string> >& vecPair) {
	assert(vecPair.empty());
	vector<string> vec;
	split_bychar(str, vec);
	convert_to_pair(vec, vecPair);
}

inline void chomp(string& str) {
	string white = " \t\n";
	string::size_type pos1 = str.find_first_not_of(white);
	string::size_type pos2 = str.find_last_not_of(white);
	if (pos1 == string::npos || pos2 == string::npos) {
		str = "";
	}
	else {
		str = str.substr(pos1, pos2 - pos1 + 1);
	}
}


inline void split_bystr(const string &str, vector<string> &vec, const char *sep) {
    split_bystr(str, vec, string(sep));
}

inline string tolowcase(const string& word) {
    string newword;
    for (unsigned int i = 0; i < word.size(); i++) {
        if (word[i] > 'A' && word[i] < 'Z') {
            char c = word[i] - 'A' + 'a';
            newword = newword + 'a' + c;
        } else {
            newword = newword + word[i];
        }
    }
    return newword;
}


// vector operations
template<typename A>
inline void clearVec(vector<vector<A> >& bivec) {
    int count = bivec.size();
    for (int idx = 0; idx < count; idx++) {
        bivec[idx].clear();
    }
    bivec.clear();
}

template<typename A>
inline void clearVec(vector<vector<vector<A> > >& trivec) {
    int count1, count2;
    count1 = trivec.size();
    for (int idx = 0; idx < count1; idx++) {
        count2 = trivec[idx].size();
        for (int idy = 0; idy < count2; idy++) {
            trivec[idx][idy].clear();
        }
        trivec[idx].clear();
    }
    trivec.clear();
}

template<typename A>
inline void resizeVec(vector<vector<A> >& bivec, const int& size1, const int& size2) {
    bivec.resize(size1);
    for (int idx = 0; idx < size1; idx++) {
        bivec[idx].resize(size2);
    }
}

template<typename A>
inline void resizeVec(vector<vector<vector<A> > >& trivec, const int& size1, const int& size2, const int& size3) {
    trivec.resize(size1);
    for (int idx = 0; idx < size1; idx++) {
        trivec[idx].resize(size2);
        for (int idy = 0; idy < size2; idy++) {
            trivec[idx][idy].resize(size3);
        }
    }
}

template<typename A>
inline void assignVec(vector<A>& univec, const A& a) {
    int count = univec.size();
    for (int idx = 0; idx < count; idx++) {
        univec[idx] = a;
    }
}

template<typename A>
inline void assignVec(vector<vector<A> >& bivec, const A& a) {
    int count1, count2;
    count1 = bivec.size();
    for (int idx = 0; idx < bivec.size(); idx++) {
        count2 = bivec[idx].size();
        for (int idy = 0; idy < count2; idy++) {
            bivec[idx][idy] = a;
        }
    }
}

template<typename A>
inline void assignVec(vector<vector<vector<A> > >& trivec, const A& a) {
    int count1, count2, count3;
    count1 = trivec.size();
    for (int idx = 0; idx < count1; idx++) {
        count2 = trivec[idx].size();
        for (int idy = 0; idy < count2; idy++) {
            count3 = trivec[idx][idy].size();
            for (int idz = 0; idz < count3; idz++) {
                trivec[idx][idy][idz] = a;
            }
        }
    }
}


template<typename A>
inline void addAllItems(vector<A>& target, const vector<A>& sources) {
    int count = sources.size();
    for (int idx = 0; idx < count; idx++) {
        target.push_back(sources[idx]);
    }
}


inline int cmpStringIntPairByValue(const pair<string, int> &x, const pair<string, int> &y) {
    return x.second > y.second;
}

#endif

