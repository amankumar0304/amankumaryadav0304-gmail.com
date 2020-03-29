#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<pthread.h>
#define MIN(x, y) (((x) < (y)) ? (x) : (y)) 
int t_complete = 0;
struct station
{
	int in_passengers;
	int out_passengers;
	pthread_mutex_t lock;
	pthread_cond_t train_arrived_cond;
  	pthread_cond_t passengers_seated_cond;
  	pthread_cond_t train_is_full_cond;

};

void station_init(struct station *station)
{
  station->out_passengers = 0;
  station->in_passengers = 0;
   pthread_mutex_init(&(station->lock), NULL);
  pthread_cond_init(&(station->train_arrived_cond), NULL);
  pthread_cond_init(&(station->passengers_seated_cond), NULL);
  pthread_cond_init(&(station->train_is_full_cond), NULL);
}
void station_load_train(struct station *station,int count)
{
	pthread_mutex_lock(&(station)->lock);
	while((station->out_passengers>0) && count>0)
	{
	pthread_cond_signal(&(station->train_arrived_cond));
	count--;
	pthread_cond_wait(&(station->passengers_seated_cond),&(station->lock));
	}
	if(station->in_passengers>0)
	{
		pthread_cond_wait(&(station->train_is_full_cond),&(station->lock));
	}
	pthread_mutex_unlock(&(station->lock));
}
void station_wait_for_train(struct station *station)
{
  pthread_mutex_lock(&(station->lock));

  station->out_passengers++;
  pthread_cond_wait(&(station->train_arrived_cond), &(station->lock));
  station->out_passengers--;
  station->in_passengers++;

  pthread_mutex_unlock(&(station->lock));

  pthread_cond_signal(&(station->passengers_seated_cond));
}
void station_on_board(struct station *station)
{
  pthread_mutex_lock(&(station->lock));

  station->in_passengers--;

  pthread_mutex_unlock(&(station->lock));

  if (station->in_passengers == 0)
  	pthread_cond_broadcast(&(station->train_is_full_cond));
}


void *passenger_thread(void *args)
{
	struct station *station=(struct station *)args;
	station_wait_for_train(station);
	return NULL;
}

struct load_train_args {
	struct station *station;
	int free_seats;
};

int load_train_returned = 0;

void* load_train_thread(void *args)
{
	struct load_train_args *ltargs = (struct load_train_args*)args;
	station_load_train(ltargs->station, ltargs->free_seats);
	load_train_returned = 1;
	return NULL;
}

int main()
{
	struct station station;
	station_init(&station);
	int i;
	int total_passengers,maximum_passengers=100;
	printf("Enter the total no of passengers waiting at platform:");
	scanf("%d",&total_passengers);
	if(maximum_passengers<total_passengers)
	{
	printf("Maximum passenger limit exceeded!!\n");
	exit(1);
	}
	if(total_passengers<=0)
	{
		printf("Train cannot depart without passengers!!\n");
		exit(1);
	}
	int passengers_left = total_passengers;
	for (i = 0; i < total_passengers; i++) {
		pthread_t tid;
		int ret = pthread_create(&tid, NULL, passenger_thread, &station);
	}
	int total_passengers_boarded = 0;
	const int max_free_seats = 50;
	int pass = 0;
	while (passengers_left > 0) {
		
	int free_seats = random() % max_free_seats;

		printf("Train entering station with free seats:%d\n", free_seats);
		load_train_returned = 0;
		struct load_train_args args = { &station, free_seats };
		int threads_done =  MIN(passengers_left, free_seats);
		passengers_left -= threads_done;
		total_passengers_boarded += threads_done;
		printf("Train departed station with: %d\n",threads_done);

		pass++;
		sleep(1);
	}
		if (total_passengers_boarded == total_passengers) {
		printf("Passengers boarded successfully!!\n");
		return 0;
	} 
}
