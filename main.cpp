#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <memory.h>

#define POPULATION 50
#define GENERATION 100
#define MUTATION 1
#define BIT_SIZE 100
#define TRIAL 50
#define CHILDNUM 2
#define SEED 100
#define CROSSTYPE 0	//0:ER,1:SGA

typedef struct{
	char bit[BIT_SIZE];
	int fitness;
}Individual;

FILE *fp;

typedef struct{
	double avg;
	int max;
	int min;
}Score;

int getRnd(int min, int max){
	return (int)(((double)rand() + 1.0) / ((double)RAND_MAX + 2.0)*(max - min + 1)) + min;
}

void setIndividual(Individual **individual){
	*individual = (Individual*)malloc(sizeof(Individual));
	int fit = 0;
	for (int i = 0; i<BIT_SIZE; i++){
		fit += (*individual)->bit[i] = getRnd(0, 1);

	}
	(*individual)->fitness = fit;
}

void deleteIndividual(Individual **individual){
	free(*individual);
}

void Initialize(Individual** individuals,int t){
	srand(SEED+t);

	for (int i = 0; i<POPULATION; i++){
		setIndividual(&individuals[i]);
	}
}

void finallize(Individual **individuals){
	for (int i = 0; i<POPULATION; i++){
		deleteIndividual(&individuals[i]);
	}
}

void Evaluate(Individual *individual){
	int fit = 0;
	for (int i = 0; i<BIT_SIZE; i++){
		fit += individual->bit[i];
	}
	individual->fitness = fit;
}

void swap(Individual** left, Individual** right){
	Individual *temp;
	temp = *left;
	*left = *right;
	*right = temp;
}

void shuffle(Individual **individuals){
	int trialnum = getRnd(0, POPULATION);
	for (int i = 0; i<trialnum; i++){
		int tar1, tar2;
		tar1 = getRnd(0, POPULATION / 2 - 1);
		tar2 = getRnd(POPULATION / 2, POPULATION - 1);
		swap(&individuals[tar1], &individuals[tar2]);
	}
}

void Mutation(Individual *individual){
	for (int i = 0; i<BIT_SIZE; i++){
		int r = getRnd(1, 100);
		if (r>MUTATION)
			continue;
		individual->fitness -= individual->bit[i];
		individual->bit[i] = !(individual->bit[i]);
		individual->fitness += individual->bit[i];
	}
}

void createChild(Individual **childs){
	for (int i = 2; i<CHILDNUM + 2; i += 2){
		int point = getRnd(1, BIT_SIZE - 2);
		int fit1 = 0;
		int fit2 = 0;
		childs[i] = (Individual*)malloc(sizeof(Individual));
		childs[i + 1] = (Individual*)malloc(sizeof(Individual));
		for (int j = 0; j<point; j++){
			fit1 += childs[i]->bit[j] = childs[0]->bit[j];
			fit2 += childs[i + 1]->bit[j] = childs[1]->bit[j];
		}
		for (int j = point; j<BIT_SIZE; j++){
			fit1 += childs[i]->bit[j] = childs[1]->bit[j];
			fit2 += childs[i + 1]->bit[j] = childs[0]->bit[j];
		}
		childs[i]->fitness = fit1;
		childs[i + 1]->fitness = fit2;
		Mutation(childs[i]);
		Mutation(childs[i + 1]);
	}
}

void print(Individual *individual){
	for (int i = 0; i<BIT_SIZE; i++){
		printf("%d", individual->bit[i]);
	}
	printf(":%d", individual->fitness);
	printf("\n");
}

void listPrint(Individual **individuals){
	for (int i = 0; i<POPULATION; i++){
		print(individuals[i]);
	}
}

void sort(Individual** individuals, int left, int right){
	if (left >= right)
		return;

	Individual **temp = (Individual**)malloc(sizeof(Individual*)*(left + right + 1));
	

	int mid = (left + right) / 2;
	int i, j, k;
	sort(individuals, left, mid);
	sort(individuals, mid + 1, right);

	for (i = left; i <= mid; i++)
		temp[i] = individuals[i];
	for (i = mid + 1, j = right; i <= right; i++, j--)
		temp[i] = individuals[j];

	i = left;
	j = right;

	
	for (k = left; k <= right; k++){
		if (temp[i]->fitness >= temp[j]->fitness)
			individuals[k] = temp[i++];
		else
			individuals[k] = temp[j--];
	}
	
	free(temp);
}

void roulette(Individual **individuals,Individual **c,int sum){
	int r=getRnd(0,sum-1);
	int saving=0;
	// 乱数により、親二体を選択
	for(int i=0,k=0;i<POPULATION;){
		saving+=individuals[i]->fitness;
		if(saving >= r){
			c[k]=individuals[i];
			k++;
			if(k==CHILDNUM){
				break;
			}
			r=getRnd(0,sum-1);
			i=0;
			saving=0;
			continue;
		}
		i++;
	}
}

void Crossover(Individual **individuals){

#if CROSSTYPE==0	// ER
	shuffle(individuals);

	for (int i = 0; i<POPULATION; i += 2){
		Individual *c[CHILDNUM + 2];
		c[0] = individuals[i];
		c[1] = individuals[i + 1];
		createChild(c);
		sort(c, 0, CHILDNUM + 2 - 1);
		individuals[i] = c[0];
		individuals[i + 1] = c[1];
		for (int j = 2; j<CHILDNUM + 2; j++){
			deleteIndividual(&c[j]);
		}
	}
#elif CROSSTYPE==1	// SGA
	Individual *children[POPULATION];
	int sumFit=0;	//fitの合計値
	for(int i=0;i<POPULATION;i++){
		sumFit+=individuals[i]->fitness;
	}
	for(int i=0;i<POPULATION;i+=2){
		Individual *c[CHILDNUM+2];
		roulette(individuals,c,sumFit);	// 親二人の選択(ルーレット)
		createChild(c);
		
		for(int j=0;j<CHILDNUM;j++){
			children[i+j]=c[j+2];
		}
	}
	deleteIndividual(individuals);
	for(int i=0;i<POPULATION;i++){
		individuals[i]=children[i];
	}
#endif
}

void getStatus(Individual** individuals,Score *now,Score *trial){
	now->max=0;
	now->avg=0;
	now->min=INT_MAX;
	for (int i = 0; i<POPULATION; i++){
		int fit = individuals[i]->fitness;
		if (now->max<fit){
			now->max = fit;
		}
		if (now->min>fit){
			now->min = fit;
		}
		now->avg += fit;
	}
	now->avg /= POPULATION;
	if(now->max>trial->max){
		trial->max=now->max;
	}
	if(now->min<trial->min){
		trial->min=now->min;
	}
	trial->avg+=now->avg;
}

void Execute(Individual **individuals,Score* trialScores){

	for (int i = 0; i<GENERATION; i++){
		Score genScore;
		Crossover(individuals);
		fprintf(fp, "gen=%d,", i);
		getStatus(individuals,&genScore,&trialScores[i]);
		fprintf(fp, "%d,%d,%lf\n", genScore.min, genScore.max, genScore.avg);
	}
}

void myOpen(){
	time_t timer;
	struct tm *t_st;
	time(&timer);
	t_st = localtime(&timer);
	char name[256];
	sprintf(name, "%d%d%d.csv", t_st->tm_hour, t_st->tm_min, t_st->tm_sec);
	fp = fopen(name, "w");
}

void myClose(){
	fclose(fp);
}

void setScore(Score *scores){
	for(int i=0;i<GENERATION;i++){
		scores[i].max=0;
		scores[i].min=INT_MAX;
		scores[i].avg=0;
	}
}

int main(void){
	Individual* individuals[POPULATION];
	Score trialScores[GENERATION];
	clock_t start, end;
	
	myOpen();
	setScore(trialScores);
	start = clock();

	for (int t = 0; t<TRIAL; t++){
		fprintf(fp, "trial=%d\n", t);
		fprintf(fp, ",min,max,avg\n");
		Initialize(individuals,t);
		Execute(individuals,trialScores);
		finallize(individuals);
	}
	fprintf(fp,",trialmin,trialmax,trialavg\n");
	for(int i=0;i<GENERATION;i++){
		fprintf(fp,"gen=%d,%d,%d,%lf\n",i,trialScores[i].min,trialScores[i].max,trialScores[i].avg/TRIAL);
	}
	end = clock();
	fprintf(fp, "time:%d[ms]\n", end - start);
	myClose();
	return 0;
}
