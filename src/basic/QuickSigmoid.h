#ifndef _SIGMOID_H_
#define _SIGMOID_H_

using namespace std;
using namespace nr;

class QuickSigmoid {
  public:
    QuickSigmoid() {
        max_exp = 6;
    }
  public:
    void init(int table_size) {
        partition_num = table_size;
        values.resize(partition_num);

        for (int i = 0; i < partition_num; i++) {
            dtype f = (2.0 * i / partition_num - 1) * max_exp;
            values[i] = exp(f);
            values[i] = values[i] / (values[i] + 1);
        }
    }

    dtype  get(dtype f) const {
        if (f >= max_exp) return 1.0;
        if (f <= -max_exp) return 0.0;
        int index = (f + max_exp) * partition_num / (2 * max_exp);
        if (index < 0) return 0.0;
        if (index >= partition_num) return 1.0;
        return values[index];
    }


  public:
    NRVec<dtype> values;
    int max_exp;
    int partition_num;
};

#endif /*_SIGMOID_H_*/
