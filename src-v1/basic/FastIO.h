#ifndef _FASTIO_H_
#define _FASTIO_H_

#include <stdio.h>

void readline(vector<string>& outputs, FILE *fin) {
    outputs.clear();
    char ch;
    string cur_item = "";
    int count = 0;
    while (!feof(fin)) {
        ch = fgetc(fin);
        if ((ch == ' ') || (ch == '\t') || (ch == '\n') || (ch == '\r')) {
            if(count > 0) outputs.push_back(cur_item);
            cur_item = "";
            count = 0;
            if ((ch == '\n'))break;
            else continue;
        }
        cur_item = cur_item + ch;
        count++;
    }

}

#endif

