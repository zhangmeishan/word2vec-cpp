/*
 * CAction.h
 *
 *  Created on: Oct 6, 2015
 *      Author: mszhang
 */

#ifndef _SRC_EXAMPLE_H_
#define _SRC_EXAMPLE_H_

class Example{
	public:
		long long sourceId;
		long long targetId;
		
	public:
		Example(){
			sourceId = targetId = -1;
		}
		
	public:
		int read(std::ifstream &inf){
			string strLine;
			if (!my_getline(inf, strLine)) {
				return -1;
			}
			vector<string> units;
			split_bychar(strLine, units, '\t');
			if(units.size() == 2){
				sourceId = atoll(units[0].c_str());
				targetId = atoll(units[1].c_str());
			}
			else{
				sourceId = targetId = -1;
			}

			return 0;
			
		}
	
	
};


#endif /* _SRC_EXAMPLE_H_ */
