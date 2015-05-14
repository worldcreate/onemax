#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <memory.h>

#define POPULATION 10
#define GENERATION 10
#define MUTATION 1
#define BIT_SIZE 50
#define TRIAL 1
#define CHILDNUM 2
#define SEED 0
#define CROSSTYPE 1	//0:ER,1:SGA

typedef struct{
	char bit[BIT_SIZE];
	int fitness;
}Individual;

typedef struct{
	double avg;
	double max;
	double min;
}Score;

FILE *fp;

int getRnd(int min, int max){
	// 一応min,maxの確認
	if(min>max){
		int t=min;
		min=max;
		max=t;
	}
	return min+(int)(rand()*(max-min+1.0)/(1.0+RAND_MAX));
}

void print(Individual *individual){
	for (int i = 0; i<BIT_SIZE; i++){
		printf("%d", individual->bit[i]);
	}
	printf(":%d\n", individual->fitness);
}

void listPrint(Individual **individuals){
	for (int i = 0; i<POPULATION; i++){
		print(individuals[i]);
	}
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

// Fisher-Yates shuffle
void shuffle(Individual **individuals){
	int max=POPULATION-1;
	for (int i = max; i>1; i--){
		int tar;
		tar = getRnd(0, i);
		swap(&individuals[tar], &individuals[i]);
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
	// 書き直す必要あり
	int r=getRnd(0,sum-1);
	int saving=0;
	// 乱数により、親二体を選択
	#ifdef DEBUG
		for(int i=0;i<POPULATION;i++){
			printf("%d,%d\n",i,individuals[i]->fitness);
		}
	#endif
	for(int i=0,k=0;i<POPULATION;){
		saving+=individuals[i]->fitness;
		if(saving >= r){
			c[k]=individuals[i];
			#ifdef DEBUG
				printf("%d\n",c[k]->fitness);
			#endif
			k++;
			if(k==2){
				break;
			}
			r=getRnd(0,sum-1);
			i=0;
			saving=0;
			continue;
		}
		i++;
	}
	#ifdef DEBUG
		printf("\n");
	#endif
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

	trial->max+=now->max;
	trial->min+=now->min;
	trial->avg+=now->avg;
}

void Execute(Individual **individuals,Score* trialScores){
	for (int i = 0; i<GENERATION; i++){
		Score genScore;
		Crossover(individuals);
		fprintf(fp, "gen=%d,", i);
		getStatus(individuals,&genScore,&trialScores[i]);
		fprintf(fp, "%lf,%lf,%lf\n", genScore.min, genScore.max, genScore.avg);
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
		scores[i].min=0;
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
		fprintf(fp,"gen=%d,%lf,%lf,%lf\n",i,trialScores[i].min/TRIAL,trialScores[i].max/TRIAL,trialScores[i].avg/TRIAL);
	}
	end = clock();
	fprintf(fp, "time:%d[ms]\n", end - start);
	myClose();
	
	return 0;
}
