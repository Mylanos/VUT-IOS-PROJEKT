#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <limits.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define ERROR 1
#define SUCCES 0
#define maxDuration 2000
#define minHSRDuration 0
#define minWDuration 20
#define minimalPierCapacity 5

#define myMMAP(pointer){(pointer) = mmap(NULL, sizeof(*(pointer)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);}
#define mySLEEP(max_time){if(max_time != 0) sleep(rand() % max_time);}
#define myUNMAP(pointer){munmap((pointer), sizeof((pointer)));}

	
FILE *outputFile;
int *actionNumber = NULL;
int *hackNumber = NULL;
int *serfNumber = NULL;

int assignDuration(char *argument, int upperLimit, int lowerLimit);
int assignPersonNumber(char *argument);
int loadArgs(int argc, char *argv[], int *firstArg, int *secondArg, int *thirdArg, int *fourthArg, int *fifthArg, int *sixthArg);
int assignCapacity(char *argument);

/*-----------------------------------FUNCTIONS---------------------------------------*/

/**
 * [prevadza string na cislo a kontroluje zadany rozsah]
 * @param  argument [string z argumenty kapacity]
 * @return          [vrati -1 pri chybe, inak vrati prevedene cislo]
 */
int assignCapacity(char *argument)
{
	char *testPTR;
	int number = strtol(argument, &testPTR, 10);
	if(*testPTR == 0)
	{
		if(number >= minimalPierCapacity)
			return number;
		else
		{
			fprintf(stderr, "Nastala chyba: Kapacita mola (posledny argument) bol mimo rozsah!\n");
			return -1;
		}
	}
	else
	{
		fprintf(stderr, "Nastala chyba: Zadany argument nebolo cele cislo!\n");
		return -1;
	}
}
/**
 * [prevadza string na cislo a kontroluje rozsah tvorenia noveho procesu]
 * @param  argument   [string z argumentov H/S/R]
 * @param  upperLimit [maximalna hranica doby]
 * @param  lowerLimit [minimalna hranica doby]
 * @return            [vrati -1 pri chybe, inak vrati prevedene cislo]
 */
int assignDuration(char *argument, int upperLimit, int lowerLimit)
{
	char *testPTR;
	int number = strtol(argument, &testPTR, 10);
	if(*testPTR == 0)
	{
		if((number >= lowerLimit)&&(number <= upperLimit))
			return number;
		else 
		{
			fprintf(stderr, "Nastala chyba: Zadany argument bol mimo rozsah!\n");
			return -1;
		}
	}
	else
	{
		fprintf(stderr, "Nastala chyba: Zadany argument nebolo cele cislo!\n");
		return -1;
	}
}

/**
 * [assignPersonNumber description]
 * @param  argument [description]
 * @return          [description]
 */
int assignPersonNumber(char *argument)
{
	char *testPTR;
	int number = strtol(argument, &testPTR, 10);
	if(*testPTR == 0)	/*ak bolo zadane cislo, teda ptr na znak ktory nemoze tvorit double je nulovy*/
	{
		if(number >= 2)
		{
			if((number % 2)== 0)
				return number;
			else
			{
				fprintf(stderr, "Nastala chyba: Zadany argument nebol delitelny dvoma!\n");
				return -1;
			}
		}
		else 
			return -1;
	}
	else
	{
		fprintf(stderr, "Nastala chyba: Zadany argument nebolo cele cislo!\n");
		return -1;
	}
}

/**
 * [nacitava argumenty a kontroluje spravny format]
 * @param  argc      [pocet argumentov]
 * @param  argv      [pole argumentov]
 * @param  firstArg  [ukazatel na hodnotu v prvom argumente]
 * @param  secondArg [ukazatel na hodnotu v prvom argumente]
 * @param  thirdArg  [ukazatel na hodnotu v prvom argumente]
 * @param  fourthArg [ukazatel na hodnotu v prvom argumente]
 * @param  fifthArg  [ukazatel na hodnotu v prvom argumente]
 * @param  sixthArg  [ukazatel na hodnotu v prvom argumente]
 * @return           [pri chybe vrati ERROR(1), inak SUCCES(0)]
 */
int loadArgs(int argc, char *argv[], int *firstArg, int *secondArg, int *thirdArg, int *fourthArg, int *fifthArg, int *sixthArg)
{	
	if(argc > 1)
	{	
		if(argc == 7)
		{	
			//prvy argument - pocet osob
			*firstArg = assignPersonNumber(argv[1]);
			if(*firstArg < 0)	return ERROR;
			//druhy argument - doba generovania procesu hackers
			*secondArg = assignDuration(argv[2], maxDuration, minHSRDuration);
			if(*secondArg < 0)	return ERROR;
			//treti argument - doba generovania procesu serfs
			*thirdArg = assignDuration(argv[3], maxDuration, minHSRDuration);
			if(*thirdArg < 0)	return ERROR;
			//stvry argument - doba plavby
			*fourthArg = assignDuration(argv[4], maxDuration, minHSRDuration);
			if(*thirdArg < 0)	return ERROR;
			//piaty argument - doba navratu na molo
			*fifthArg = assignDuration(argv[5], maxDuration, minWDuration);
			if(*thirdArg < 0)	return ERROR;
			//siesty argument - kapacita mola
			*sixthArg = assignCapacity(argv[6]);
			if(*sixthArg < 0)	return ERROR;
		
			return SUCCES;
		}
		else
		{
			fprintf(stderr, "Nastala chyba: Zadali ste malo argumentov!\n");
			return ERROR;
		}
	}
	else
	{
		fprintf(stderr, "Nastala chyba: Nezadali ste argumenty!\n");
		return ERROR;
	}
}
void initialize()
{
	//za zapisom do suboru davat fflush LADATIPS
	//alebo predtym na zaciatku mainu setbuf(outPoutfile, NULL);
	//outputfile = fopen("proj2.out", "w");
	myMMAP(actionNumber);
	myMMAP(hackNumber);
	myMMAP(serfNumber);
	*actionNumber = 0;
	*hackNumber = 0;
	*serfNumber = 0;
}
void cleanup()
{
	myUNMAP(actionNumber);
	myUNMAP(hackNumber);
	myUNMAP(serfNumber);
}

void processHacker(int hackerID)
{
	printf("%d\t: HACK %d\t : starts\n", *actionNumber, hackerID);

	*actionNumber = *actionNumber + 1;
	*hackNumber = *hackNumber + 1;
	printf("%d\t: SERF %d\t : waits\t: %d\t: %d\n", *actionNumber, hackerID, *hackNumber, *serfNumber);
}
void processSerf(int serfID)
{
	printf("%d\t: SERF %d\t : starts\n", *actionNumber, serfID);

	*actionNumber = *actionNumber + 1;
	*serfNumber = *serfNumber + 1;
	printf("%d\t: SERF %d\t : waits\t: %d\t: %d\n", *actionNumber, serfID, *hackNumber, *serfNumber);
}

void generateHackers(int numOfPeople, int delay)
{
	pid_t generator;
	int pid;
	for(int id = 1; id <= numOfPeople; id++)
	{
		if((pid = fork()) < 0)
		{	//error
			perror("fork: ");
			exit(0);
		}
		if(pid == 0)
		{
			*actionNumber = *actionNumber + 1;
			processHacker(id);
			exit(0);
		}
		generator = pid;
		mySLEEP(delay);
	}

	waitpid(generator, NULL, 0);
}
void generateSerfs(int numOfPeople, int delay)
{
	pid_t generator;
	int pid;
	for(int id = 1; id <= numOfPeople; id++)
	{
		if((pid = fork()) < 0)
		{	//error
			perror("fork: ");
			exit(0);
		}
		if(pid == 0)
		{
			*actionNumber = *actionNumber + 1;
			processSerf(id);
			exit(0);
		}
		generator = pid;
		mySLEEP(delay);
	}

	waitpid(generator, NULL, 0);
}
void processPier()
{
//	while()
 printf("SOM V PIER POGGERS\n");
	
}


/*-----------------------------------FUNCTIONS---------------------------------------*/

/*-------------------------------------MAIN------------------------------------------*/
/**
 * [main]
 * @param  argc [pocet argumentov]
 * @param  argv [pole argumentov]
 * @return      [vrati 1 ak nastala chyba, inak vrati 0]
 */
int main(int argc, char *argv[])
{
	// spracovanie argumentov
	int sizePersons, procTimeH, procTimeS, sailDurationR, pierReturnTime, pierCapacity;
	int error = loadArgs(argc, argv, &sizePersons, &procTimeH, &procTimeS, &sailDurationR, &pierReturnTime, &pierCapacity);
	if (error)	return ERROR;
	//spracovanie argumentov

	initialize();

	int hackers = 0;
	int serfs = 0;
	int pid;
	pid_t generator;
	pid_t personGenerator;

	if((pid = fork()) < 0)
	{	//error
		perror("fork: ");
		exit(0);
	}

	if(pid == 0)
	{	//child
		processPier(sailDurationR, minimalPierCapacity);
		exit(0);
	}
	else 
	{	//parent
		generator = pid;
		if( (pid = fork()) < 0)
		{
			perror("fork: ");
			exit(0);
		}
		if(pid == 0)
		{
			if((pid = fork()) < 0)
			{
				perror("fork: ");
				exit(0);
			}
			if(pid == 0)
			{
				generateHackers(sizePersons, procTimeH);
				exit(0);
			}
			else
				generateSerfs(sizePersons, procTimeS);
				exit(0);
		}
		else
			personGenerator = pid;
	
	}
	
	waitpid(generator, NULL, 0);
	waitpid(personGenerator, NULL, 0);

	cleanup();

	return SUCCES;
}

/*-------------------------------------MAIN------------------------------------------*/	
