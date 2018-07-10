#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include "hmm.h"
using namespace std;
#define MAX_NUM 10
int n;
HMM models[MAX_NUM];
char *list, *test, *result;
double delta[MAX_SEQ][MAX_STATE];
char buf[MAX_LINE];
FILE* fout;

static double calculate(int index)
{
	memset(delta, 0, sizeof(delta));
	double maximum = 0;
	int buf_len = strlen(buf);
	HMM hmm = models[index];
	int state_num = hmm.state_num;
	for(int i = 0; i < state_num; i++){
		delta[0][i] = hmm.initial[i] * hmm.observation[buf[0]-'A'][i];
	}

	for(int i = 1; i < buf_len; i++)
		for(int j = 0; j < state_num; j++){
			// delta[i][j] = 0;
			for(int k = 0; k < state_num; k++)
				delta[i][j] = max(delta[i][j], delta[i-1][k]*hmm.transition[k][j]);
			delta[i][j] *= hmm.observation[buf[i]-'A'][j];
		}

	for(int i = 0; i < state_num; i++)
		maximum = max(maximum, delta[buf_len-1][i]);
	return maximum;
}

static void testing()
{
	int maxid = 0;
	double maxp = 0;
	for(int i = 0; i < n; i++){
		double prob = calculate(i);
		if(prob > maxp){
			maxp = prob;
			maxid = i;
		}
	}
	// printf("!!!!!\n");
	fprintf(fout, "%s %g\n", models[maxid].model_name, maxp);
}

int main(int argc, char** argv)
{
	list = argv[1];
	test = argv[2];
	result = argv[3];
	n = load_models(list, models, MAX_NUM);
	// printf("n = %d\n", n);
	FILE* ftest = open_or_die(test, "r");
	fout = open_or_die(result, "w");
	memset(buf, 0, sizeof(buf));
	while( fscanf(ftest, "%s", buf) > 0 ){
		// printf("!!!!!!\n");
		testing();
	}
	fclose(ftest);
	fclose(fout);
	return 0;
}