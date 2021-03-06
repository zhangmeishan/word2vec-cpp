/*
 * CAction.h
 *
 *  Created on: Oct 6, 2015
 *      Author: mszhang
 */

#ifndef _SRC_EXAMPLE_H_
#define _SRC_EXAMPLE_H_

class Example {
  public:
    int sourceId;
    int targetId;

  public:
    Example() {
        sourceId = targetId = -1;
    }

  public:
    int read(FILE *fin) {
        sourceId = targetId = -1;
        int  ch = 0;
        while (!feof(fin)) {
            ch = fgetc(fin);
            if (ch >= '0' && ch <= '9') {
                break;
            }
        }

        if (ch >= '0' && ch <= '9') {
            sourceId = ch - '0';
            while (!feof(fin)) {
                ch = fgetc(fin);
                if (ch >= '0' && ch <= '9') {
                    sourceId = sourceId * 10 + (ch - '0');
                } else {
                    break;
                }
            }
        }

        while (!feof(fin)) {
            ch = fgetc(fin);
            if (ch >= '0' && ch <= '9') {
                break;
            }
        }
        if (ch >= '0' && ch <= '9') {
            targetId = ch - '0';
            while (!feof(fin)) {
                ch = fgetc(fin);
                if (ch >= '0' && ch <= '9') {
                    targetId = targetId * 10 + (ch - '0');
                } else {
                    break;
                }
            }
        }

        if (feof(fin))return -1;

        return 0;
    }

	void write(FILE *fout) {
		fprintf(fout, "%d\t%d\n", sourceId, targetId);
	}

};


#endif /* _SRC_EXAMPLE_H_ */
