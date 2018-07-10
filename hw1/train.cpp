#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "hmm.h"
#define MAX_ROW 55
HMM hmm;
/*
typedef struct{
   char *model_name;
   int state_num;					//number of state
   int observ_num;					//number of observation
   double initial[MAX_STATE];			//initial prob. pi
   double transition[MAX_STATE][MAX_STATE];	//transition prob. A
   double observation[MAX_OBSERV][MAX_STATE];	//observation prob. B
} HMM;
*/
char* model;
char buf[MAX_ROW];
int seq[MAX_ROW];
int state_num;
int observ_num ;
double gama[MAX_ROW][MAX_STATE];
double observ_gama[MAX_OBSERV][MAX_STATE];
double eps[MAX_STATE][MAX_STATE];
double alpha[MAX_LINE][MAX_STATE];	
double beta[MAX_LINE][MAX_STATE];
int N = 0, T;

static void for_alpha(int T)
{
	//alpha initialization
	for(int i = 0; i < state_num; i++)
		alpha[0][i] = hmm.initial[i] * hmm.observation[seq[0]][i];
	
	//alpha induction
	for(int i = 1; i < T; i++)
		for(int j = 0; j < state_num; j++){
			alpha[i][j] = 0;
			for(int last = 0; last < state_num; last++)
				alpha[i][j] += alpha[i-1][last] * hmm.transition[last][j];
			alpha[i][j] *= hmm.observation[seq[i]][j];
		}
}

static void for_beta(int T)
{
	//beta initialization
	for(int i = 0; i < state_num; i++)
		beta[T-1][i] = 1;

	//beta induction
	for(int i = T-2; i >= 0; i--)
		for(int j = 0; j < state_num; j++){
			beta[i][j] = 0;
			for(int next = 0; next < state_num; next++)
				beta[i][j] += hmm.transition[j][next] * hmm.observation[seq[i+1]][next]*beta[i+1][next];
		}
}

static void for_gama(int T)
{
	for(int i = 0; i < T; i++){
		double arr[state_num];
		double sum = 0;
		for(int j = 0; j < state_num; j++){
			arr[j] = alpha[i][j] * beta[i][j];
			sum += arr[j];
		}
		for(int j = 0; j < state_num; j++){
			gama[i][j] += arr[j]/sum;
			observ_gama[seq[i]][j] += arr[j]/sum;
		}
	}
}

static void for_eps(int T)
{
	for(int i = 0; i < T-1; i++){
		double arr[state_num];
		double sum = 0;
		for(int j = 0; j < state_num; j++){
			arr[j] = alpha[i][j] * beta[i][j];
			sum += arr[j];
		}
		for(int j = 0; j < state_num; j++)
			for(int k = 0; k < state_num; k++)
				eps[j][k] += (alpha[i][j]*hmm.transition[j][k]*hmm.observation[seq[i+1]][k]*beta[i+1][k])/sum;
	}
			
}

static void train(char* model)
{
	FILE *fp = open_or_die(model, "r");
	memset(gama, 0, sizeof(gama));
    memset(observ_gama, 0, sizeof(observ_gama));
    memset(eps, 0, sizeof(eps));
    memset(alpha, 0, sizeof(alpha));
    memset(beta, 0, sizeof(beta));

	while( fgets(buf, MAX_ROW, fp) > 0 ){
		T = strlen(buf) - 1;
		N++;
		for (int i = 0; i < T; i++)
			seq[i] = buf[i] - 'A';
		for_alpha(T);
		for_beta(T);
		for_gama(T);
		for_eps(T);
	}
	fclose(fp);

	//update 
	for(int i = 0; i < state_num; i++){
		//update pi
		hmm.initial[i] = gama[0][i]/N;

		//update transition
		double gama_sum = 0;

		for(int j = 0; j < T-1; j++)
			gama_sum += gama[j][i];

		for(int next = 0; next < state_num; next++)
			hmm.transition[i][next] = eps[i][next]/gama_sum;
	}

	//update observation
	for(int i = 0; i < observ_num; i++)
		for(int j = 0; j < state_num; j++){
			double gama_sum = 0;
			for(int k = 0; k < T; k++)
				gama_sum += gama[k][j];
			hmm.observation[i][j] = observ_gama[i][j]/gama_sum;
		}

}

int main(int argc, char**argv)
{
	
	int iteration = atoi(argv[1]);
	char* init = argv[2];
	model = argv[3];
	char* out = argv[4];
	loadHMM(&hmm, init);
	state_num = hmm.state_num;
	observ_num = hmm.observ_num;
	for(int i = 0; i < iteration; i++)
		train(model);
	FILE *fout = open_or_die(out, "w");
	dumpHMM(fout, &hmm);
	fclose(fout);

	return 0;
}