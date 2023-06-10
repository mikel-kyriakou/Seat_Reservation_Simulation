#include "p3200228-p3190136-res.h" 
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

//dilwseis mutex, cond, metavliton kai pinakon theseon

pthread_mutex_t telMutex;
pthread_cond_t telCond;

pthread_mutex_t cashierMutex;
pthread_cond_t cashierCond;

pthread_mutex_t zoneAmutex;

pthread_mutex_t zoneBmutex;

pthread_mutex_t accountMutex;

pthread_mutex_t ioMutex;

pthread_mutex_t firstMutex;

pthread_mutex_t statsMutex;

int bankAccount=0;

int countTel = Ntel;

int countCash = Ncash;

int success = 0, cardFailure = 0, noSeats = 0; 

int wait_time = 0;

int service_time = 0;

int seed;

bool first = 0;

int arrayZoneA[Nseat * NzoneA] = {0};
int indexZoneA[NzoneA] = {0};
int arrayZoneB[Nseat * NzoneB] = {0}; 
int indexZoneB[NzoneB] = {0};


void print_info(int num) //xrisimopiite sto telos tis main gia na parousiasei diafores plirofories kai statistika. Parametron einai o arithmos ton pelaton
{

	printf("\nSeat Plan:\n");
	for(int i = 0; i < NzoneA; i++)
	{
		if(indexZoneA[i] > 0)
		{
			for(int j = 0; j<indexZoneA[i]; j++)
			{
				printf("Zone A / Row %d / Seat %d / Client %d\n", i+1, j+1, arrayZoneA[i]);
			}
		}
		
	}
	
	for(int i = 0; i < NzoneB; i++)
	{
		if(indexZoneB[i] > 0)
		{
			for(int j = 0; j<indexZoneB[i]; j++)
			{
				printf("Zone B / Row %d / Seat %d / Client %d\n", i+1, j+1, arrayZoneB[i]);
			}
		}
	}
	
	printf("\nStats:\n");
	
	printf("Total earnings: %d \n", bankAccount);
	
	printf("Successful reservations: %.2f%% \n", (double)(success*100)/num);
	
	printf("No availability: %.2f%% \n", (double)(noSeats*100)/num);
	
	printf("Card failure: %.2f%% \n", (double)(cardFailure*100)/num);
	
	printf("Average wait time: %.2fsec\n", (double)wait_time/num);
	
	printf("Average service time: %.2fsec\n", (double)service_time/num);

	
}

int get_rand(int *seed, int max) //epistrefi arithmo apo to 0 mexri to max-1
{
	return (rand_r(seed)%max);
}

int checkZone(int amount, bool isAlpha) //eleghos theseon kai epistrofi index arxikis thesis. An den brei epistrefi -1
{
	int i, j, counter=0;
	
		
	sleep(get_rand(&seed, tseathigh - tseatlow + 1) + tseatlow);
	
	
	if(isAlpha)
	{
		for(i=0; i<NzoneA; i++)
		{
			
			if(indexZoneA[i] + amount <= Nseat)
			{
				return i*Nseat + indexZoneA[i];
			}
		}
	}
	else
	{
		for(i=0; i<NzoneB; i++)
		{
			if(indexZoneB[i] + amount <= Nseat)
			{
				return i*Nseat + indexZoneB[i];
			}
		}
	}
	
	return -1;
}



void *service(void *number) //sinartisi gia to nima tou pelati
{
	int clientId = *(int *) number;
	int rc;
	int seatFound;
	int i;
	int seat_amount;
	int random_AB;
	int random_cardsuccess;
	int cashier_search;
	int random_wait_tel;
	int clientWait=0;
	struct timespec start, stop, startService, stopService;
	
	//eleghos gia to an ine o protos pelatis. an einai kanei tin metavliti first=1 kai sinehizei xoris na perimenei. an den einai perimenei gia ligo.
	
	rc = pthread_mutex_lock(&firstMutex);
	if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(first) is %d\n", rc);exit(-1);}
	
	if(first == 0)
	{
		first = 1;
	}
	else
	{
	random_wait_tel = get_rand(&seed, treshigh) + treslow;
		
	sleep(random_wait_tel); //anamoni gia epomeno tilefonima
	}
	
	rc = pthread_mutex_unlock(&firstMutex);
	if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(first) is %d\n", rc);exit(-1);}
	
	//pernoume tin xroniki stigmi pou ginete to tilefonima
	
	if(clock_gettime(CLOCK_REALTIME, &startService)== -1)
	{
		exit(-1);
	}
	
	if(clock_gettime(CLOCK_REALTIME, &start)== -1)
	{
		exit(-1);
	}
	
	//emfanisi minimatos
	
	rc = pthread_mutex_lock(&ioMutex);
	if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(io) is %d\n", rc);exit(-1);}
	
	printf("Client %d: Searching for tel\n", clientId);
	
	rc = pthread_mutex_unlock(&ioMutex);
	if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(io) is %d\n", rc);exit(-1);}
	
	//eleghos gia diathesimo tilefoniti
	
	rc = pthread_mutex_lock(&telMutex);
	if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(tel) is %d\n", rc);exit(-1);}
	
	while(countTel == 0) //an den iparhi tilefonitis perimenoume
	{
		rc = pthread_mutex_lock(&ioMutex);
		if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(io) is %d\n", rc);exit(-1);}
		
		printf("Client %d: No available tel\n", clientId);
		rc = pthread_mutex_unlock(&ioMutex);
		if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(io) is %d\n", rc);exit(-1);}
		
		/////////////
		
		rc = pthread_cond_wait(&telCond, &telMutex);
		if(rc != 0){printf("ERROR: return code from pthread_cond_wait(tel) is %d\n", rc);exit(-1);}
	}
	
	countTel--;
	
	rc = pthread_mutex_unlock(&telMutex);
	if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(tel) is %d\n", rc);exit(-1);}
	
	// pernoume ti xroniki stigmi pou sindethikame me tilefoniti
	
	if(clock_gettime(CLOCK_REALTIME, &stop)== -1)
	{
		exit(-1);
	}
	
	clientWait += (int)stop.tv_sec - (int)start.tv_sec; // ipologizoume kai prosthetoume sto xrono anamonis pelati ton xrono anamonis gia tilefoniti
	
	// emfanisi minimaton

	rc = pthread_mutex_lock(&ioMutex);
	if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(io) is %d\n", rc);exit(-1);}
	
	printf("Client %d: Tel connected\n", clientId);
	
	rc = pthread_mutex_unlock(&ioMutex);
	if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(io) is %d\n", rc);exit(-1);}
	
	// pernoume tixeous arithmous gia epilogi zonis kai arithmo theseon
	
	random_AB = get_rand(&seed, 10);
	seat_amount = get_rand(&seed, Nseathigh - Nseatlow + 1) + Nseatlow;
	
	if(random_AB < PzoneA*10)//eleghos zonis A
	{
		
		//emfanisi minimaton
		
		rc = pthread_mutex_lock(&ioMutex);
		if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(io) is %d\n", rc);exit(-1);}

		printf("Tel to Client %d: Checking for seats...\n", clientId);

		rc = pthread_mutex_unlock(&ioMutex);
		if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(io) is %d\n", rc);exit(-1);}
		
		//klidoma mutex theseon zonis A
		rc = pthread_mutex_lock(&zoneAmutex);
		if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(zoneAmutex) is %d\n", rc);exit(-1);}
			
		//klisi sinartisis checkZone gia na vrei theseis
		
		seatFound = checkZone(seat_amount, true); //elegxos diathesimon theseon 
		
		if(seatFound < 0) //an den vrei thesis pelati
		{
			//xeklidoma mutex theseon
			rc = pthread_mutex_unlock(&zoneAmutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(zoneAmutex) is %d\n", rc);exit(-1);}
	
			//emfanisi minimaton
			rc = pthread_mutex_lock(&ioMutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(io) is %d\n", rc);exit(-1);}
	
			printf("Tel to Client %d: Reservation cancelled because of no availability of seats\n", clientId);
			printf("Client %d: Exiting...\n", clientId);
	
			rc = pthread_mutex_unlock(&ioMutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(io) is %d\n", rc);exit(-1);}
			
			//apodesmefsi tilefoniti kai enimerosi me tin cond_signal
			rc = pthread_mutex_lock(&telMutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(tel) is %d\n", rc);exit(-1);}
			
			countTel++;
			
			rc = pthread_cond_signal(&telCond);
			if(rc != 0){printf("ERROR: return code from pthread_cond_signal(tel) is %d\n", rc);exit(-1);}
			
			rc = pthread_mutex_unlock(&telMutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(tel) is %d\n", rc);exit(-1);}
			
			//pernoume xroniki stigmi pou teliose o pelatis
			if(clock_gettime(CLOCK_REALTIME, &stopService)== -1)
			{
				exit(-1);
			}
			
			//enimerosi statistikon  gia xrono anamonis kai exipiretisis kai apotixia evresis theseon
			rc = pthread_mutex_lock(&statsMutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(stats) is %d\n", rc);exit(-1);}
			
			noSeats++;
			wait_time += clientWait;
			service_time += (int)stopService.tv_sec - (int)startService.tv_sec;
			
			rc = pthread_mutex_unlock(&statsMutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(stats) is %d\n", rc);exit(-1);}

			pthread_exit(NULL);//termatismos pelati
		}
		
		//An brei arketes thesis pelati, enimeroni ton pinaka theseon kai xeklidoni to mutex theseon
		for(i=seatFound; i< seatFound + seat_amount; i++)
		{
			arrayZoneA[i] = clientId;
		}
		
		indexZoneA[seatFound/Nseat] += seat_amount;
		
		rc = pthread_mutex_unlock(&zoneAmutex);
		if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(zoneAmutex) is %d\n", rc);exit(-1);}
		
	}
	else // eleghos zonis B
	{
	
		//emfanisi minimaton
		
		rc = pthread_mutex_lock(&ioMutex);
		if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(io) is %d\n", rc);exit(-1);}

		printf("Tel to Client %d: Checking for seats...\n", clientId);

		rc = pthread_mutex_unlock(&ioMutex);
		if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(io) is %d\n", rc);exit(-1);}
		
		//klidoma mutex zonis B
		rc = pthread_mutex_lock(&zoneBmutex);
		if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(zoneBmutex) is %d\n", rc);exit(-1);}
		
		seatFound = checkZone(seat_amount, false);//elegxos diathesimon theseon 
		
		if(seatFound < 0) //an den vrei thesis pelati
		{
			//xeklidoma mutex zonis B
			rc = pthread_mutex_unlock(&zoneBmutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(zoneBmutex) is %d\n", rc);exit(-1);}
			
			//emfanisi minimaton
			rc = pthread_mutex_lock(&ioMutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(io) is %d\n", rc);exit(-1);}
	
			printf("Tel to Client %d: Reservation cancelled because of no availability of seats\n", clientId);
			printf("Client %d: Exiting...\n", clientId);
	
			rc = pthread_mutex_unlock(&ioMutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(io) is %d\n", rc);exit(-1);}
			
			//apodesmefsi tilefoniti
			rc = pthread_mutex_lock(&telMutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(tel) is %d\n", rc);exit(-1);}
			
			countTel++;
			
			rc = pthread_cond_signal(&telCond);
			if(rc != 0){printf("ERROR: return code from pthread_cond_signal(tel) is %d\n", rc);exit(-1);}
			
			rc = pthread_mutex_unlock(&telMutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(tel) is %d\n", rc);exit(-1);}

			//pernoume ti xroniki stigmi pou telioni i exipiretisi
			if(clock_gettime(CLOCK_REALTIME, &stopService)== -1)
			{
				exit(-1);
			}
			
			//enimerosi statistikon
			rc = pthread_mutex_lock(&statsMutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(stats) is %d\n", rc);exit(-1);}
			
			noSeats++;
			wait_time += clientWait;
			service_time += (int)stopService.tv_sec - (int)startService.tv_sec;
			
			rc = pthread_mutex_unlock(&statsMutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(stats) is %d\n", rc);exit(-1);}
			
			pthread_exit(NULL);//termatismos pelati
		}
		
		//An brei arketes thesis pelati, enimeroni ton pinaka theseon kai xeklidoni to mutex theseon
		
		for(i=seatFound; i<seatFound + seat_amount; i++)
		{
			arrayZoneB[i] = clientId;
		}
		
		indexZoneB[seatFound/Nseat] += seat_amount;
		
		rc = pthread_mutex_unlock(&zoneBmutex);
		if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(zoneBmutex) is %d\n", rc);exit(-1);}
	}
	
	
	//apodesmefsi tilefoniti afou sindethikame me cashier kai cond_signal afou iparxi diathesimos tilefonitis
	
	countTel++;
	
	rc = pthread_cond_signal(&telCond);
	if(rc != 0){printf("ERROR: return code from pthread_cond_signal(tel) is %d\n", rc);exit(-1);}
	
	rc = pthread_mutex_unlock(&telMutex);
	if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(tel) is %d\n", rc);exit(-1);}
		
	//emfanisi minimaton
	
	rc = pthread_mutex_lock(&ioMutex);
	if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(io) is %d\n", rc);exit(-1);}
	
	printf("Client %d: Searching for cashier\n", clientId);
	
	rc = pthread_mutex_unlock(&ioMutex);
	if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(io) is %d\n", rc);exit(-1);}
	
	//pernoume ti xroniki stigmi pou xekina i anamoni gia cashier
	

	if(clock_gettime(CLOCK_REALTIME, &start)== -1)
	{
		exit(-1);
	}
	
	//klidoma mutex cashier
	
	rc = pthread_mutex_lock(&cashierMutex);
	if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(cashier) is %d\n", rc);exit(-1);}
		
	while(countCash == 0) //an den iparxi diathesimos cashier
	{
		rc = pthread_mutex_lock(&ioMutex);
		if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(io) is %d\n", rc);exit(-1);}
	
		printf("Client %d: No available cashier\n", clientId);
		rc = pthread_mutex_unlock(&ioMutex);
		if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(io) is %d\n", rc);exit(-1);}
		
		///////////////////////////////////////////////////////////////////////////
	
		rc = pthread_cond_wait(&cashierCond, &cashierMutex);
		if(rc != 0){printf("ERROR: return code from pthread_cond_wait(cashier) is %d\n", rc);exit(-1);}
	}
	
	countCash--; //molis vroume cashier ton desmevoume kai xeklidonoume to mutex
	
	rc = pthread_mutex_unlock(&cashierMutex);
	if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(cahsier) is %d\n", rc);exit(-1);}
	
	
	
	//pernoume ti xroniki stigmi pou telioni i anamoni
	
	if(clock_gettime(CLOCK_REALTIME, &stop)== -1)
	{
		exit(-1);
	}
	
	clientWait += (int)stop.tv_sec - (int)start.tv_sec; //prosthetoume stin anamoni pelati tin anamoni gia cashier
	
	//enimerosi statistikon gia tin anamoni tou pelati
	
	rc = pthread_mutex_lock(&statsMutex);
	if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(stats) is %d\n", rc);exit(-1);}
	
	wait_time += clientWait;
	
	rc = pthread_mutex_unlock(&statsMutex);
	if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(stats) is %d\n", rc);exit(-1);}
	
	//emfanisi minimaton
	
	rc = pthread_mutex_lock(&ioMutex);
	if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(io) is %d\n", rc);exit(-1);}
	
	printf("Client %d: Connected with the cashier\n", clientId);		
	printf("Client %d: Wait few secs to check the payment\n", clientId);
	
	rc = pthread_mutex_unlock(&ioMutex);
	if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(io) is %d\n", rc);exit(-1);}
	
	//pernoume tixeo arithmo gia anamoni gia pliromi
	
	cashier_search = get_rand(&seed, tcashhigh - tcashlow + 1) + tcashlow;
	sleep(cashier_search);
	
	//pernoume tixeo arithmo gia na doume an petixe i pliromi
	
	random_cardsuccess=get_rand(&seed,10);
	
	if (random_cardsuccess<Pcardsucces*10)//se peritosi pou petixe i pliromi
	{	
		//enimerosi statistikon gia epitixes kratisis
		rc = pthread_mutex_lock(&statsMutex);
		if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(stats) is %d\n", rc);exit(-1);}
		
		success++;
		
		rc = pthread_mutex_unlock(&statsMutex);
		if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(stats) is %d\n", rc);exit(-1);}
	
		if (random_AB<PzoneA*10)//an o pelatis kratise thesi stin zoni A
		{
			//pistonoume ton logariasmo tis eterias
			rc = pthread_mutex_lock(&accountMutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(account) is %d\n", rc);exit(-1);}

			bankAccount=bankAccount+seat_amount*CzoneA;

			rc = pthread_mutex_unlock(&accountMutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(account) is %d\n", rc);exit(-1);}
		
			//emfanisi minimaton
			rc = pthread_mutex_lock(&ioMutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(io) is %d\n", rc);exit(-1);}

			printf("Client %d: Payment completed!Your seats are in zone A, row %d, number ", clientId,seatFound/Nseat +1);
			for (i=1; i<=seat_amount;i++)
			{
				printf("%d, " ,seatFound%10+i);
			
			}
			
			printf("and the total cost is %d\n",seat_amount*CzoneA);
			
			rc = pthread_mutex_unlock(&ioMutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(io) is %d\n", rc);exit(-1);}
			
		}
		else //an o pelatis kratise thesi stin zoni B
		{
			//pistosi logariasmou eterias
			rc = pthread_mutex_lock(&accountMutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(account) is %d\n", rc);exit(-1);}

			bankAccount=bankAccount+seat_amount*CzoneB;

			rc = pthread_mutex_unlock(&accountMutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(account) is %d\n", rc);exit(-1);}
		
			//emfanisi minimaton
			rc = pthread_mutex_lock(&ioMutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(io) is %d\n", rc);exit(-1);}

			printf("Client %d: Payment completed!Your seats are in zone B, row %d, number ", clientId, seatFound/Nseat+1);
			
			for (i=1; i<=seat_amount;i++)
			{
				printf("%d, " ,seatFound%10+i);
			
			}
			
			printf("and the total cost is %d\n",seat_amount*CzoneB);
			
			rc = pthread_mutex_unlock(&ioMutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(io) is %d\n", rc);exit(-1);}
		
		
		}
		
	}
	else //an den petixe i pliromi
	{
		if (random_AB<PzoneA*10)//an kratise thesis sti zoni A
		
		{	
			//klidonoume to mutex theseon kai enimeronoume tis thesis tou pelati os diathesimes
			rc = pthread_mutex_lock(&zoneAmutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(zoneAmutex) is %d\n", rc);exit(-1);}
			
			for(i=seatFound; i<seatFound+seat_amount;i++)
			{
			
				arrayZoneA[i] = 0;
			}
			
			indexZoneA[seatFound/Nseat] -= seat_amount;
			
			rc = pthread_mutex_unlock(&zoneAmutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(zoneAmutex) is %d\n", rc);exit(-1);}
			
		}
		else //an kratise thesis sti zoni B
		{	
			//klidonoume to mutex theseon kai enimeronoume tis thesis tou pelati os diathesimes
			rc = pthread_mutex_lock(&zoneBmutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(zoneBmutex) is %d\n", rc);exit(-1);}
		
			for(i=seatFound; i<seatFound+seat_amount;i++)
			{
			
				arrayZoneB[i] = 0;
			}
			
			indexZoneB[seatFound/Nseat] -= seat_amount;
				
			rc = pthread_mutex_unlock(&zoneBmutex);
			if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(zoneBmutex) is %d\n", rc);exit(-1);}
		
		}
		
		//emfanisi minimaton
		rc = pthread_mutex_lock(&ioMutex);
		if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(io) is %d\n", rc);exit(-1);}

		printf("Client %d: Reservation cancelled because card transaction failed !\n", clientId);
		
		rc = pthread_mutex_unlock(&ioMutex);
		if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(io) is %d\n", rc);exit(-1);}
		
		//enimarosi statistikon gia apotixia pliromis
		
		rc = pthread_mutex_lock(&statsMutex);
		if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(stats) is %d\n", rc);exit(-1);}
		
		cardFailure++;
		
		rc = pthread_mutex_unlock(&statsMutex);
		if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(stats) is %d\n", rc);exit(-1);}
		
	}
	
	//apodesmefsi cashier kai enimerosi me cond_signal gia diathesimo cashier
	pthread_mutex_lock(&cashierMutex);
	if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(cashier) is %d\n", rc);exit(-1);}
		
	countCash++;
	
	rc = pthread_cond_signal(&cashierCond);
	if(rc != 0){printf("ERROR: return code from pthread_cond_signal(cashier) is %d\n", rc);exit(-1);}
	
	rc = pthread_mutex_unlock(&cashierMutex);
	if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(cashier) is %d\n", rc);exit(-1);}		
	
	//emfanisi minimaotn
	
	rc = pthread_mutex_lock(&ioMutex);
	if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(io) is %d\n", rc);exit(-1);}

	printf("Client %d: Exiting...\n", clientId);
	
	rc = pthread_mutex_unlock(&ioMutex);
	if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(io) is %d\n", rc);exit(-1);}
	
	//pernoume ti xroniki stigmi pou telioni i exipiretisi
			
	if(clock_gettime(CLOCK_REALTIME, &stopService)== -1)
	{
		exit(-1);
	}
	
	//enimerosi  statistikon gia xrono exipiretisis
	rc = pthread_mutex_lock(&statsMutex);
	if(rc != 0){printf("ERROR: return code from pthread_mutex_lock(stats) is %d\n", rc);exit(-1);}
	
	service_time += (int)stopService.tv_sec - (int)startService.tv_sec;
	
	rc = pthread_mutex_unlock(&statsMutex);
	if(rc != 0){printf("ERROR: return code from pthread_mutex_unlock(stats) is %d\n", rc);exit(-1);}
	
	pthread_exit(NULL); //termatismos pelati
}

int main(int argc, char *argv[])
{
	int Ncust = atoi(argv[1]);
	seed = atoi(argv[2]);
	int rc;
	int i;
	pthread_t *clients = malloc(Ncust * sizeof(pthread_t));
	int arrayId[Ncust];
	
	//dimiourgia klidomaton mutex
	rc = pthread_mutex_init(&cashierMutex, NULL);
	if(rc != 0)
	{
		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       	exit(-1);
	}
	
	rc = pthread_mutex_init(&telMutex, NULL);
	if(rc != 0)
	{
		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       	exit(-1);
	}
	
	rc = pthread_mutex_init(&zoneAmutex, NULL);
	if(rc != 0)
	{
		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       	exit(-1);
	}
	
	rc = pthread_mutex_init(&zoneBmutex, NULL);
	if(rc != 0)
	{
		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       	exit(-1);
	}
	
	rc = pthread_mutex_init(&accountMutex, NULL);
	if(rc != 0)
	{
		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       	exit(-1);
	}	
	
	rc = pthread_mutex_init(&ioMutex, NULL);
	if(rc != 0)
	{
		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       	exit(-1);
	}
	
	rc = pthread_mutex_init(&firstMutex, NULL);
	if(rc != 0)
	{
		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       	exit(-1);
	}
	
	rc = pthread_mutex_init(&statsMutex, NULL);
	if(rc != 0)
	{
		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       	exit(-1);
	}
	
	
	//dimiourgia conditions
	rc = pthread_cond_init(&cashierCond, NULL);
	if(rc != 0)
	{
		printf("ERROR: return code from pthread_cond_init() is %d\n", rc);
       	exit(-1);
	}
	
	rc = pthread_cond_init(&telCond, NULL);
	if(rc != 0)
	{
		printf("ERROR: return code from pthread_cond_init() is %d\n", rc);
       	exit(-1);
	}
	
	//dimiourgia nimaton pelaton
	for(i = 0; i < Ncust; i++)
	{
		arrayId[i] = i+1;
		printf("Main: Creating client %d\n", arrayId[i]);
		
		rc = pthread_create(&clients[i], NULL, service, &arrayId[i]);
		if(rc != 0)
		{
			printf("ERROR: return code from pthread_create() is %d\n", rc);
       		exit(-1);
		}	
	}
	
	//join nimaton pelaton
	for(i = 0; i < Ncust; i++)
	{
		rc = pthread_join(clients[i], NULL);
		if(rc != 0)
		{
			printf("ERROR: return code from pthread_join() is %d\n", rc);
       		exit(-1);
		}
		
	}
	
	print_info(Ncust); //klisi sinartisis gia emfanisi diaforon pliroforion
	
	free(clients); //apodesmefsi mnimis
	return 1;
}
