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
#include <string.h>

#define ERROR 1
#define SUCCES 0
#define maxDuration 2000
#define minHSRDuration 0
#define minWDuration 20
#define minimalPierCapacity 5
#define boatCapacity 4
#define semBOARDING "proj2-xziska03-boarding"
#define semUNBOARDING "proj2-xziska03-UNBOARDING"
#define semPIER "proj2-xziska03-pier"
#define semMUTEX "proj2-xziska03-mutex"
#define semCAPWAITSCREW "proj2-xziska03-waitCrew"
#define semLEFTPIER "proj2-xziska03-leftPier"

#define myMMAP(pointer){(pointer) = mmap(NULL, sizeof(*(pointer)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);}
#define mySLEEP(max_time){if(max_time != 0) usleep(rand() % max_time);}
#define myUNMAP(pointer){munmap((pointer), sizeof((pointer)));}
	
int *actionNumber = NULL;
int *hackNumber = NULL;
int *serfNumber = NULL;
int *unboardedPerson = NULL;
FILE *outputFile;

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
 * [prevadza string na cislo a kontroluje zadane podmienky]
 * @param  argument [prevadzany argument]
 * @return          [vrati prevedene cislo alebo -1 pri chybe]
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
			fprintf(stderr, "Nastala chyba: Počet argumentov bol mimo rozsah!\n");
			return ERROR;
		}
	}
	else
	{
		fprintf(stderr, "Nastala chyba: Nezadali ste argumenty!\n");
		return ERROR;
	}
}

/**
 * [na standardny chybovy vystup vytlaci popis chyby a ukonci program]
 */
void printErrno(){
	fprintf(stderr, "Nastala chyba v sem_open() - errno: %d\n", errno);
	exit(1);
}
void openOutputFile() {
	if((outputFile = fopen("proj2.out", "w")) == NULL){
			fprintf(stderr, "Subor sa nepodarilo otvorit\n");
			exit(1);
	}
	setbuf(outputFile, NULL);
	return;
}

void closeOutputFile() {
    if (fclose(outputFile) == EOF) {
        fprintf(stderr, "Subor s prikazmy sa nepodarilo zatvorit\n");
        exit(1);
    }
    
	return;
    
}
/**
 * [Inicializuje potrebne zdroje(semafory a zdielane premenne)]
 * @param pierCapacity [kapacita mola na ktoru inicializujeme semafor]
 */
void initialize(int pierCapacity)
{
	errno=0;
	//za zapisom do suboru davat fflush LADATIPS
	//alebo predtym na zaciatku mainu setbuf(outPoutfile, NULL);
	//outp utfile = fopen("proj2.out", "w");
	openOutputFile();
	sem_t *sem; 
	if((sem = sem_open(semBOARDING, O_CREAT, 0666, 0)) == SEM_FAILED) 			//INIT NA KAPACITU LODE
		printErrno();
	for(int i = 0; i < (boatCapacity-1); i++){sem_post(sem);}
    sem_close(sem);

    if(((sem = sem_open(semUNBOARDING, O_CREAT, 0666, 0)) == SEM_FAILED)) 
    	printErrno();
    sem_close(sem);

    if((sem = sem_open(semPIER, O_CREAT, 0666, 0)) == SEM_FAILED)				//INIT NA KAPACITU MOLA
    	printErrno();
    for(int i = 0; i < pierCapacity; i++){sem_post(sem);}
   	sem_close(sem);
    if((sem = sem_open(semMUTEX, O_CREAT, 0666, 0)) == SEM_FAILED)				//INIT na jednu polozku
    	printErrno();
    sem_post(sem);
   	sem_close(sem);

   	if((sem = sem_open(semCAPWAITSCREW, O_CREAT, 0666, 0)) == SEM_FAILED) 		//INIT odomkne sa pre kapitana
   		printErrno();
    sem_close(sem);

    if((sem = sem_open(semLEFTPIER, O_CREAT, 0666, 0)) == SEM_FAILED) 		//INIT odomkne sa miesto na mole
   		printErrno();
    sem_close(sem);
	myMMAP(actionNumber);
	myMMAP(hackNumber);
	myMMAP(serfNumber);
	myMMAP(unboardedPerson);
	*actionNumber = 0;
	*hackNumber = 0;
	*serfNumber = 0;
}
/**
 * [odalokuje vsetky zdroje]
 */
void cleanup()
{
	myUNMAP(actionNumber);
	myUNMAP(hackNumber);
	myUNMAP(serfNumber);
	myUNMAP(unboardedPerson);
	sem_unlink(semBOARDING);
	sem_unlink(semUNBOARDING);
	sem_unlink(semPIER);
	sem_unlink(semMUTEX);
	sem_unlink(semCAPWAITSCREW);
	sem_unlink(semLEFTPIER);

	closeOutputFile();
}	

void captainBoards(int serfID, char *processName, int sailDuration){
	sem_t *mutex = sem_open(semMUTEX, O_RDWR);
	sem_t *unboarding = sem_open(semUNBOARDING, O_RDWR);
	sem_t *waitForCrew = sem_open(semCAPWAITSCREW, O_RDWR);
	sem_t *boarding = sem_open(semBOARDING, O_RDWR);

	sem_wait(mutex);
	fprintf(outputFile,"%d\t:%s %d\t\t:boards\t\t:%d\t:%d\n", ++(*actionNumber), processName, serfID, *hackNumber, *serfNumber);
	sem_post(mutex); 

	mySLEEP(sailDuration);	//plavba
	for(int i = 0; i < (boatCapacity - 1); i++){sem_post(unboarding);}	//uvolnenie posadky - 3 clenovia 
	sem_wait(waitForCrew);		//pockat na posadku(kapitan vystupuje posledny)
	*unboardedPerson = 0;		//reinicializovanie na nulu pre dalsie plavby
	sem_wait(mutex);
	fprintf(outputFile,"%d\t:%s %d\t\t:captain exits\t:%d\t:%d\n", ++(*actionNumber), processName, serfID, *hackNumber, *serfNumber);
	sem_post(mutex);

	sem_close(boarding);
	sem_close(waitForCrew);
	sem_close(unboarding);
	sem_close(mutex);
	cleanup();
	exit(0);
}

/**
 * [funkcia na prevedenie procesu Serf]
 * @param serfID [Poradove cislo Serfa]
 */
void processPier(int ID, char *processName, int sailDuration)
{
	sem_t *unboarding = sem_open(semUNBOARDING, O_RDWR);
	sem_t *mutex = sem_open(semMUTEX, O_RDWR);
	sem_t *waitForCrew = sem_open(semCAPWAITSCREW, O_RDWR);
	sem_t *boarding = sem_open(semBOARDING, O_RDWR);
	sem_t *pierCap = sem_open(semPIER, O_RDWR);		//kapacita mola

	sem_wait(mutex);
	if((strcmp(processName, "HACK")) == 0)
		(*hackNumber)++;
	else if((strcmp(processName, "SERF")) == 0)
		(*serfNumber)++;
	fprintf(outputFile,"%d\t:%s %d\t\t:waits\t\t:%d\t:%d\n", ++(*actionNumber), processName, ID, *hackNumber, *serfNumber);
	sem_post(mutex);


	if(*hackNumber == 4)
	{
		for(int i = 0; i < boatCapacity; i++){sem_post(pierCap);}
		*hackNumber = *hackNumber - 4;
		captainBoards(ID, processName, sailDuration);
	}
	else if(*serfNumber == 4)
	{
		for(int i = 0; i < boatCapacity; i++){sem_post(pierCap);}
		*serfNumber = *serfNumber - 4;
		captainBoards(ID, processName, sailDuration);
	}
	else if((*serfNumber >= 2) && (*hackNumber >= 2))
	{
		for(int i = 0; i < boatCapacity; i++){sem_post(pierCap);}
		*hackNumber = *hackNumber - 2;
		*serfNumber = *serfNumber - 2;
		captainBoards(ID, processName, sailDuration);
	}
	sem_wait(boarding);

	sem_wait(unboarding);

	sem_wait(mutex);
	fprintf(outputFile,"%d\t:%s %d\t\t:member exits\t:%d\t:%d\n", ++(*actionNumber), processName, ID, *hackNumber, *serfNumber);
	(*unboardedPerson)++;
	sem_post(mutex);

	if((*unboardedPerson) == 3)
	{
		sem_post(waitForCrew);
	}
	sem_post(boarding);

	sem_close(boarding);
	sem_close(waitForCrew);
	sem_close(mutex);
	sem_close(unboarding);
	sem_close(pierCap);
}

/**
 * [Generuje procesy a posiela procesy na molo]
 * @param numOfPeople  [pocet osob vstupujucich na molo]
 * @param procDelay    [doba po ktorej sa tvori novy proces]
 * @param processName  [nazov procesu, bud  "HACK" alebo "SERF"]
 * @param sailDuration [dlzka doby plavby]
 * @param returnTime   [dlzka doby navratu ak bolo molo plne]
 */
void generateProcess(int numOfPeople, int procDelay, char *processName, int sailDuration, int returnTime)
{
	sem_t *pierCap = sem_open(semPIER, O_RDWR);		//kapacita mola

	sem_t *mutex = sem_open(semMUTEX, O_RDWR);		//zabezpecuje aby do subora printoval iba jeden subor

	sem_t *leftPier = sem_open(semLEFTPIER, O_RDWR);

	pid_t generator[numOfPeople];
	int pid;


	printf("proces - %d | sail - %d | return - %d\n", procDelay, sailDuration, returnTime);
	for(int id = 1; id <= numOfPeople; id++)
	{
		sem_wait(mutex);
		fprintf(outputFile,"%d\t:%s %d\t\t:starts\n", ++(*actionNumber), processName, id);
		sem_post(mutex);

		if((pid = fork()) < 0)
		{	//error
			perror("fork: ");
			exit(0);
		}
		if(pid == 0)
		{
			while(1)
			{
				if(sem_trywait(pierCap) < 0)		//molo je plne
				{
					printf("aaaaa\n");
					sem_wait(mutex);
					fprintf(outputFile,"%d\t:%s %d\t\t:leaves queue\t:%d\t:%d\n", ++(*actionNumber), processName, id, *hackNumber, *serfNumber);
					sem_post(mutex);
					mySLEEP(returnTime);		//proces odchadza na 'returnTime' 

					sem_wait(mutex);
					fprintf(outputFile,"%d\t:%s %d\t\t:is back\n", ++(*actionNumber), processName, id);
					sem_post(mutex);
				}
				else
				{
					processPier(id, processName, sailDuration);
					sem_close(pierCap);
					sem_close(mutex);
					sem_close(leftPier);
					exit(1);
				}
				printf("v cyklee\n");
			}
			printf("test\n");
		}
		else
		{	
			generator[id-1] = pid;
			mySLEEP(procDelay);
		}
	}

	for (int i = 0; i < numOfPeople; ++i)
	{
		waitpid(generator[i], NULL, 0);
	}
	sem_close(pierCap);
	sem_close(mutex);
	sem_close(leftPier);
}

/*-----------------------------------FUNCTIONS---------------------------------------*/

/*-------------------------------------MAIN------------------------------------------*/
/**
 * [main]
 * @param  argc [pocet argumentov]
 * @param  argv [pole argumentov]
 * @return      [ak nastala chyba vrati 1, inak vrati 0]
 */
int main(int argc, char *argv[])
{
	setbuf(stdout,NULL);
    setbuf(stderr,NULL);
	// spracovanie argumentov
	int sizePersons, procTimeH, procTimeS, sailDuration, pierReturnTime, pierCapacity;
	int error = loadArgs(argc, argv, &sizePersons, &procTimeH, &procTimeS, &sailDuration, &pierReturnTime, &pierCapacity);
	if (error) return ERROR;
	//spracovanie argumentov
	initialize(pierCapacity);
	
	int pid;
	pid_t generator;

	if((pid = fork()) < 0)
	{	//error
		perror("fork: ");
		exit(0);
	}

	if(pid == 0)
	{	//child
		if( (pid = fork()) < 0)
		{
			perror("fork: ");
			exit(0);
		}
		if(pid == 0)	//vetva generuje procesy HACKER
		{
			generateProcess(sizePersons, procTimeH, "HACK", sailDuration, pierReturnTime);
			exit(0);
		}
		else			//vetva generuje procesy SERF
		{
			generateProcess(sizePersons, procTimeS, "SERF", sailDuration, pierReturnTime);
			exit(0);
		}
	}
	else 	//parent
		generator = pid;
	
	waitpid(generator, NULL, 0);

	cleanup();

	return SUCCES;
}

/*-------------------------------------MAIN------------------------------------------*/	
