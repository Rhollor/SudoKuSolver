/*This work is licensed under a Creative Commons Attribution 4.0 International License
view  http://creativecommons.org/licenses/by/4.0/  for further details*/

/*SudoKu solver*/


#include<iostream> 
#include<fstream> 
#include <stdlib.h>
#include<stdio.h>
#include <time.h>

using namespace std;

#define SUDOKU_SIZE 9
#define END 13 /*Bit position that is used to indicate the cell to be filled*/
#define INPUTFILE "sudoku.in"
#define OUTPUTFILE "Solver.out"
#define CLEARBIT(x,v) (x&(~(1<<(v)))) 
#define COPY_STATE(dest,src) \
{\
for(int _i=0;_i<SUDOKU_SIZE;_i++)\
	for(int _j=0;_j<SUDOKU_SIZE;_j++)\
		dest[_i][_j] = src[_i][_j];\
}

#define VERBOSE(flg ,format, ...)\
if(flg)\
	printf("Verbose mode ON ...:"format"\n");\
	





/*Function prototypes*/
long long solvesudoku(unsigned int a[][SUDOKU_SIZE], unsigned int output[][SUDOKU_SIZE]);
unsigned int iscomplete(unsigned int output[][SUDOKU_SIZE]);
void fill(unsigned int value, unsigned int r, unsigned int c,unsigned int output[][SUDOKU_SIZE]);
unsigned int fillup(unsigned int output[][SUDOKU_SIZE]);
void print_output_array(unsigned int output[SUDOKU_SIZE][SUDOKU_SIZE]);
int backtrack(unsigned int output[][SUDOKU_SIZE]);

/*Unit Test*/
void unitTest();
bool generatePuzzles(unsigned int a[][SUDOKU_SIZE],int seed,int blocks);
/*Helper*/
void interpretoutput(unsigned int src[SUDOKU_SIZE][SUDOKU_SIZE] , unsigned int dest[SUDOKU_SIZE][SUDOKU_SIZE]);
int verifypuzzle(unsigned int a[SUDOKU_SIZE][SUDOKU_SIZE],int r,int c);

bool verbose =true;/*In case user needs more details*/
bool debug = false;/*In case of problems programmer*/
bool profiler=true;


void unitTest(){

for(int i=0;i<1000;i++){
	unsigned int a[SUDOKU_SIZE][SUDOKU_SIZE];
	unsigned int output[SUDOKU_SIZE][SUDOKU_SIZE];
	int blocksfilled = ((rand()%(SUDOKU_SIZE)+ 17));
	
	if(!generatePuzzles(a,(i*i<<2+i^65362 + i-842),blocksfilled))
		continue;
	if(verbose){
		cout<<"Printing after generation"<<endl;
		print_output_array(a);
	}
		
	long long timetaken =time(NULL);
	timetaken-= solvesudoku(a,output);
	//PROFILE();
		if(profiler){
			FILE* fp = fopen("prof.txt","ab");
			fprintf(fp,"%d,%d,%lld\n",i,blocksfilled,timetaken);
			fclose(fp);
		}
	}

}
bool generatePuzzles(unsigned int a[][SUDOKU_SIZE],int seed,int blocks) {
	srand ( seed );
	memset(a,0,sizeof(a));
	int incorrect;
	int times = 0;
	
		
		
	for(int i=0;i<blocks;i++){
		
		int r = rand()%SUDOKU_SIZE;
		int c = rand()%SUDOKU_SIZE;
		a[r][c] = rand()%SUDOKU_SIZE;
		cout<<"vallll "<<a[r][c]<<endl;
		incorrect = verifypuzzle(a,r,c);
		if(incorrect!=0){
			a[r][c] = 0;
			i--;
			times++;
		}
		cout<<"vallll out "<<a[r][c]<<endl;
		if(times >= 100){
			cout<<"unable to generate random puzzle"<<blocks<<endl;
			return false;
		}
	}
	
	
	
	return true;

}



int verifypuzzle(unsigned int a[SUDOKU_SIZE][SUDOKU_SIZE],int r,int c){
	int incorrect = 0;

	//for(int i=0;i<SUDOKU_SIZE;i++)
	//	for(int j=0;j<SUDOKU_SIZE;j++)
	
	{
		int i=r,j=c;
		if(a[i][j]!=0) {
			for(int k=0;k<SUDOKU_SIZE;k++)
				if((a[k][j] == a[i][j] && k!=j)|| (a[i][k]==a[i][j] && k!=j)){
					a[i][j]=0;
					
					return 1;
				}
				
			for(int p=0;p<9;p++){
				int r1 = 3*(i/3) +(p/3);
				int c1 = 3*(j/3) + (p%3);
				if((a[r1][c1] == a[i][j])&& r1!=i && j!=c1){
						a[i][j] = 0;
						//incorrect++;
						return 1;
				}
			}
				
		}
	
	}
				
	
	return incorrect;		
}




/*Helper*/
/*
Usage:
fills a value in the grid , propagates constraints
*/

void fill(unsigned int value, unsigned int r, unsigned int c,unsigned int output[][SUDOKU_SIZE]){

for(unsigned int i=0;i<SUDOKU_SIZE;i++)
	output[r][i] = CLEARBIT(output[r][i],value); /*clearing value th bit*/
for(unsigned int i=0;i<SUDOKU_SIZE;i++)
	output[i][c] = CLEARBIT(output[i][c],value); /*clearing value th bit*/
	
unsigned int p=(r/3)*3 , q=(c/3)*3;


output[p][q] = CLEARBIT(output[p][q],value);
output[p][q+1] = CLEARBIT(output[p][q+1],value);
output[p][q+2] = CLEARBIT(output[p][q+2],value);


output[p+1][q] = CLEARBIT(output[p+1][q],value);
output[p+1][q+1] = CLEARBIT(output[p+1][q+1],value);
output[p+1][q+2] = CLEARBIT(output[p+1][q+2],value);


output[p+2][q] = CLEARBIT(output[p+2][q],value);
output[p+2][q+1] = CLEARBIT(output[p+2][q+1],value);
output[p+2][q+2] = CLEARBIT(output[p+2][q+2],value);


output[r][c] = (1<<value) | (1<<END); 

}



/*Helper*/
/* Usage:
Traverses entire grid ,
Finds the first cell which can be ascertained with a certain value*/
unsigned int fillup(unsigned int output[][SUDOKU_SIZE]){
/*look for potential solutions*/
for(int i=0;i<SUDOKU_SIZE;i++){
	for(int j=0;j<SUDOKU_SIZE;j++){
	/*if a unique solution exists at 
		a location then its a output[i][j]
		is a power of 2 !!*/
		
		if(!(output[i][j]&(output[i][j]-1))){
			/*mark a bit so that we
			dont see it again*/
			/*extract bit pos*/
			int v = output[i][j];
			int cnt=0;
			/*counts the position of bits set*/
			while(!(v&1)){
				cnt++;
				v/=2;
			}
			if(cnt == 13){
				cout<<"Shouldnt Come here :( "<<endl;
				return 0;
			}else
				fill(cnt,i,j,output); 
			//cout<<cnt<<endl;
			return 1;
		}
	}
}
return 0;
}

/*The solver routine*/
long long solvesudoku(unsigned int a[][SUDOKU_SIZE], unsigned int output[][SUDOKU_SIZE]){

	for(unsigned int i=0;i<SUDOKU_SIZE;i++){
		for(unsigned int j=0;j<SUDOKU_SIZE;j++){
			output[i][j] = (0x3FE & ~(1<<END));/*possible values bit-manip*/
		}
	}

	for(unsigned int i=0;i<SUDOKU_SIZE;i++){
		for(unsigned int j=0;j<SUDOKU_SIZE;j++){
			if(a[i][j]!=0){
				fill(a[i][j],i,j,output);
				VERBOSE(verbose,"FILLING %u %u \n ",i,j);
				if(debug)
					print_output_array(output);
				
			}
		}
	}


	while(fillup(output)){
		VERBOSE(verbose,"completing puzzle ...\r");
	}

	if(iscomplete(output)){
		VERBOSE(verbose,"EASY PUZZLE :)\n");
	}
	else {
		VERBOSE(verbose,"HARD PUZZLE .. proceeding with Backtracking search\n");
		/*Backtracking + based on higher chances of success first*/
		if(backtrack(output)){
			VERBOSE(verbose,"SOLVEDPrinting RESULT\n");
		}
		else{
			VERBOSE(verbose,"SOLVED Printing RESULT\n");
			return -1;
		}
			
		
	}
	
	
	if(verbose)
		print_output_array(output);
	return 0;
}


int backtrack(unsigned int output[][SUDOKU_SIZE]){
	 VERBOSE(verbose,"BACKTACKING ...\r");
	 
	/*Finding the best next guy*/
	int lr=SUDOKU_SIZE,lc=SUDOKU_SIZE,t=0,pre_cnt =100;
	for(unsigned int i=0;i<SUDOKU_SIZE;i++){
		for(unsigned int j=0;j<SUDOKU_SIZE;j++){
			if(!((output[i][j]>>END) &1)){
				int v = output[i][j];
				int cnt = 1;
				while((v&(v-1))){
					v-=(v&(v-1));
					cnt++;		
				}
			
			lr = cnt<pre_cnt ? i:lr;
			lc = cnt<pre_cnt ? j:lc;
			pre_cnt = cnt<pre_cnt ? cnt:pre_cnt;
			t++;
			
			}
			
	}
	

}

	if(lr ==SUDOKU_SIZE || lc==SUDOKU_SIZE)
		return 1;

	/*extract lowest isolated bit*/
	int v =output[lr][lc];
	/*Checking for possible paths*/
	for(int i=1;i<32;i++){
		if((v>>i)&1){
			unsigned int temp[SUDOKU_SIZE][SUDOKU_SIZE];
			COPY_STATE(temp,output);
			fill(i, lr, lc,temp);
			if(!backtrack(temp))
				continue;
			else{
				COPY_STATE(output,temp);
				return 1;
			}
		}

	}
	return 0;



}

/*Helper*/
unsigned int iscomplete(unsigned int output[][SUDOKU_SIZE]){
	for(unsigned int i=0;i<SUDOKU_SIZE;i++){
		for(unsigned int j=0;j<SUDOKU_SIZE;j++){
			unsigned int val = output[i][j];
			val = val&~(1<<END);
			if((val&(val-1)))
				return 0;
		}
	}
	return 1;
}


/*Helper*/
void print_output_array(unsigned int output[SUDOKU_SIZE][SUDOKU_SIZE]){
cout<<endl<<endl<<endl;
	for(unsigned int i=0;i<SUDOKU_SIZE;i++){
		for(unsigned int j=0;j<SUDOKU_SIZE;j++){
			cout<<output[i][j]<<" ";				
			
			
		}
		cout<<endl;
	
	}
cout<<endl<<endl<<endl;
}


void interpretoutput(unsigned int src[SUDOKU_SIZE][SUDOKU_SIZE] , unsigned int dest[SUDOKU_SIZE][SUDOKU_SIZE]){
	for(unsigned int i=0;i<SUDOKU_SIZE;i++){
		for(unsigned int j=0;j<SUDOKU_SIZE;j++){
			
			int flg =0;
			for(int _p=1;_p<=SUDOKU_SIZE;_p++){
				if(!(src[i][j]& ~(1<<END | 1<<_p))){
					dest[i][j] = _p;
					//fout<<_p;
					flg=1;
					break;
				}
			}
			if(!flg)
				dest[i][j] = 0;
		}
	}

}

void usage(){
cout<<endl<<endl<<"USAGE :: executible <options,one or many>"<<endl;
cout<<"Options (all are case sensitive)"<<endl;
cout<<"		-h : Help , this menu"<<endl;
cout<<"		-UnitTest :Run unit test on sudoku solver "<<endl;
cout<<"		-Profile : Run Profiler calculate the time taken write into prof.txt in the same location as the executible [Switched on by default]"<<endl;
cout<<endl<<endl;

}

int main(int argc , char* argv[]){

	profiler = true;
	if(argc>1){
		if(memcmp(argv[1],"-ProfilerOFF",sizeof(char)*12) == 0)
			profiler = false;		
		
		if(memcmp(argv[1],"-h",sizeof(char)*2) == 0){
			usage();
			return 0;
		}else{
			if(memcmp(argv[1],"-UnitTest",sizeof(char)*3) == 0){
				unitTest();
				return 0;
			}
			
		}
		

	}

	ifstream fin;
	ofstream fout;
	fin.open(INPUTFILE);
	fout.open(OUTPUTFILE);
	unsigned int  n;
	fin>>n;
	srand ( time(NULL) );
	int random_number = rand();


	while(n--) {
		fout<<endl<<endl;
		unsigned int a[SUDOKU_SIZE][SUDOKU_SIZE];
		memset(a,0,sizeof(a));
		for(unsigned int i=0;i<SUDOKU_SIZE;i++)
		for(unsigned int j=0;j<SUDOKU_SIZE;j++)
			fin>>a[i][j];
		
		unsigned int intermediateOutput[SUDOKU_SIZE][SUDOKU_SIZE];
		unsigned int output[SUDOKU_SIZE][SUDOKU_SIZE];
		memset(intermediateOutput,0,sizeof(intermediateOutput));
		memset(output,0,sizeof(output));
		
		long long timetaken =time(NULL);
		timetaken -=solvesudoku(a,intermediateOutput);
		
		interpretoutput(intermediateOutput,output);
		for(int i=0;i<SUDOKU_SIZE;i++){
			for(int j=0;j<SUDOKU_SIZE;j++)
				fout<<output[i][j]<<" ";
			fout<<endl;
		}
		
		/*for(unsigned int i=0;i<SUDOKU_SIZE;i++){
			for(unsigned int j=0;j<SUDOKU_SIZE;j++){
				fout<<" ";
				int flg =0;
				for(int _p=1;_p<=SUDOKU_SIZE;_p++){
					if(!(output[i][j]& ~(1<<END | 1<<_p))){
						fout<<_p;
						flg=1;
						break;
					}
				}
				if(!flg)
					fout<<0;
			}
		fout<<endl;	
		}*/

	}


	fin.close();
	fout.close();
	return 0;
}
