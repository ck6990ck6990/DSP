#include <stdio.h>
#include <string>
#include <string.h>
#include <vector>
#include <iterator>
#include <map>
#include <Ngram.h>
using namespace std;

typedef map<string, vector<string> > MAP;
typedef struct{
	int idx;
	string str;
	double prob;
}node;

vector<string> ans;
inline void buildmap(MAP& Map){
	FILE *fp = fopen("ZhuYin-Big5.map", "r");
	char buf[65536];
	while(fgets(buf, sizeof(buf), fp) != NULL){
		char *token = strtok(buf, " ");
		string key(token);
		token = strtok(NULL, " \n");
		while(token != NULL){
			Map[key].push_back(token);
			token = strtok(NULL, " \n");
		}
	}
	Map["\n"].push_back("</s>");
}

inline double ngramProb(Vocab& voc, Ngram& lm, const char *a, const char *b){
	VocabIndex wid1 = voc.getIndex(a);
	VocabIndex wid2 = voc.getIndex(b);
	if(wid1 == Vocab_None)
		wid1 = voc.getIndex(Vocab_Unknown);
	if(wid2 == Vocab_None)
		wid2 = voc.getIndex(Vocab_Unknown);
	VocabIndex context[] = {wid1, Vocab_None};
	return lm.wordProb(wid2, context);
}

int main(int argc, char* argv[]){
	if(argc != 4){
		printf("Usage : ./mydisambig [input file] [lm] [order]");
		exit(0);
	}
	
	int order = atoi(argv[3]);
	Vocab voc;
	Ngram lm(voc, order);
	File lmfile(argv[2], "r");
	lm.read(lmfile);
	lmfile.close();
	MAP Map;
	buildmap(Map);
	
	char buf[512];
	FILE *input = fopen(argv[1], "r");
	while(fgets(buf, sizeof(buf), input) != NULL){
		vector<string> chars;
		char *token = strtok(buf, " ");
		while(token != NULL){
			chars.push_back(token);
			token = strtok(NULL, " ");
		}
		vector<vector<node> > table;
		vector<node> last;

		last.push_back((node){0, "<s>", 1.0});
		table.push_back(last);
		
		for(int i = 0; i < chars.size(); i++){
			// int ii = distance(chars.begin(), i);
			last.clear();
			for(auto j = Map[chars[i]].begin(); j != Map[chars[i]].end(); j++){
				double maxprob = -999999.0;
				int maxidx = 0;
				for(int k = 0; k < table[i].size(); k++){
					double prob = table[i][k].prob + ngramProb(voc, lm, table[i][k].str.data(), j->data());
					if(prob > maxprob){
						maxprob = prob;
						maxidx = k;
					}
				}
				last.push_back((node){maxidx, *j, maxprob});
				ans.push_back(*j);
			}
			table.push_back(last);
		}

		vector<string> output;
		int idx = 0;
		for(int i = table.size()-1; i >= 0; i--){
			output.push_back(table[i][idx].str);
			idx = table[i][idx].idx;
		}
		for(int len = output.size()-1; len >= 0; len--)
			printf("%s%c", output[len].data(), " \n"[len==0]);
			
	}
}