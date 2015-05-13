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

#ifndef DEBUG
void Mutation(Individual *individual){
	#ifdef DEBUG
		printf("mutation\n");
	#endif
	for (int i = 0; i<BIT_SIZE; i++){
		int r = getRnd(1, 100);
		if (r>MUTATION)
			continue;
		#ifdef DEBUG
			printf("ok\n");
		#endif
		individual->fitness -= individual->bit[i];
		individual->bit[i] = !(individual->bit[i]);
		individual->fitness += individual->bit[i];
	}
}
#endif

#ifdef DEBUG
void Mutation(Individual *individual,int color){
	#ifdef DEBUG
		printf("mutation\n");
	#endif
	for (int i = 0; i<BIT_SIZE; i++){
		int r = getRnd(1, 100);
		
		if (r>MUTATION){
			printf("\033[%dm",color);
			printf("%d",individual->bit[i]);
			continue;
		}
		individual->fitness -= individual->bit[i];
		individual->bit[i] = !(individual->bit[i]);
		individual->fitness += individual->bit[i];
		printf("\033[37m");
		printf("%d",individual->bit[i]);
	}
	printf(":%d\033[39m\n",individual->fitness);
}
#endif

void createChild(Individual **childs){
	#ifdef DEBUG
		for(int i=0;i<BIT_SIZE;i++){
			printf("\033[32m");
			printf("%d",childs[0]->bit[i]);
		}
		printf("\033[39m");
		printf("\n");
		for(int i=0;i<BIT_SIZE;i++){
			printf("\033[35m");
			printf("%d",childs[1]->bit[i]);
		}
		printf("\033[39m");
		printf("\n");
	#endif

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
		#ifndef DEBUG
		Mutation(childs[i]);
		Mutation(childs[i + 1]);
		#endif
		#ifdef DEBUG
			for(int j=0;j<BIT_SIZE;j++){
				printf("\033[32m");
				if(j>=point)
					printf("\033[35m");
				printf("%d",childs[i]->bit[j]);
			}
			printf(":%d",childs[i]->fitness);
			printf("\033[39m\n");
			Mutation(childs[i],32);
			for(int j=0;j<BIT_SIZE;j++){
				printf("\033[35m");
				if(j>=point)
					printf("\033[32m");
				printf("%d",childs[i+1]->bit[j]);
			}
			printf(":%d",childs[i+1]->fitness);
			printf("\033[39m\n");
			Mutation(childs[i+1],35);
		#endif
		
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
		
		#ifdef DEBUG
			printf("before\n");
			for(int j=0;j<POPULATION;j++){
				if(j==i){
					printf("\033[32m");
				}
				if(j==i+1){
					printf("\033[35m");
				}
				print(individuals[j]);
				printf("\033[39m");
			}
		#endif

		createChild(c);
		sort(c, 0, CHILDNUM + 2 - 1);
		individuals[i] = c[0];
		individuals[i + 1] = c[1];

		#ifdef DEBUG
			printf("after\n");
			for(int j=0;j<POPULATION;j++){
				if(j==i || j==i+1){
					printf("\033[32m");
				}
				print(individuals[j]);
				printf("\033[39m");
			}
		#endif

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
		#ifdef DEBUG
			printf("gen=%d\n",i);
		#endif
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
	#ifdef DEBUG
	Individual* individuals[POPULATION];
	Initialize(individuals,0);
	listPrint(individuals);
	printf("###########################\n");
	sort(individuals,0,POPULATION-1);
	listPrint(individuals);
	finallize(individuals);
	#else
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
	#endif
	return 0;
}
