#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#include <glib.h>
#include <gio/gio.h>

/*
 * constructor 함수 : main 이전에 실행됨, GCC only
 *
 * eclipse에서 cygwin gdb 디버깅할 경우
 * gdb를 통한 프로그램의 콘솔 출력 버퍼가 정상동작 하지 않고
 * 실행 종료 후 한꺼번에 출력되는 경우가 있음.
 * 이를 피하기 위해 stdout, stderr의 buffering을 끄는 함수.
 * 만일 buffer가 꼭 필요하다면 아래 함수를 주석처리 할 것.
 */
void __attribute__((constructor)) console_setting_for_eclipse_debugging( void ){
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
}


#define MAX_ITEM_SIZE 100
#define MAX_BUFFER_SIZE 128

typedef struct Bus Bus;
typedef struct Station Station;
typedef struct Road Road;

struct Bus {
	char	name[6];	// bus name
	int		dist;		// distance
	Bus*	prev;		// previous bus
	Bus*	next;		// next bus
	int		speed;
	int		period;
	int		time[3];
};

struct Station {
	char	name[6];
	int		dist;
	Bus*	arrival;
	int		limit;
	Bus*	sign_bus;
	int		sign_time;

};


typedef struct Road {
//	int			time[3];
	Bus*		b[MAX_ITEM_SIZE];
	Station*	s[MAX_ITEM_SIZE];
	int 		num_bus;
	int			num_station;
} Road;

Road road;


/////////////////////////////////////////////////////////////////////

Bus* new_bus(char* name, int dist)
{
	Bus* b = (Bus*) malloc(sizeof(Bus));;
	strcpy(b->name, name);
	b->dist = dist;
	b->speed = 0;
	b->prev = b->next = NULL;
	b->period = 0;
	b->time[0] = b->time[1] = b->time[2] = 0;
	return b;
}

Station* new_station(char* name, int dist, int limit)
{
	Station* s = (Station*) malloc(sizeof(Station));
	strcpy(s->name, name);
	s->dist = dist;
	s->limit = limit;
	s->arrival = NULL;
	s->sign_bus = NULL;
	s->sign_time = 0;
	return s;
}

void free_road(Road* r)
{
	for(int i=0; i<r->num_bus; i++) free(r->b[i]);
	for(int i=0; i<r->num_station; i++) free(r->s[i]);
}

/////////////////////////////////////////////////////////////////////

int diff_time(int* nt, int* ot)
{
	return (nt[0]-ot[0])*60*60 + (nt[1]-ot[1])*60 + (nt[2]-ot[2]);
}

int find_period(Road* r, int dist)
{
	for(int i=r->num_station-1; i>=0; i--) {
		if (r->s[i]->dist <= dist ) return i;
	}
	return 0;

//	for(int i=0; i<r->num_station; i++) {
//		if ( r->s[i]->dist >= dist ) return i;
//	}
//	return r->num_station-1;
}


void find_bus_and_update(Road* r, char* name, int dist, int dt)
{
	for(int i=0; i<r->num_bus; i++) {
		if (strcmp(r->b[i]->name, name) == 0 ) {
			int dd = dist - r->b[i]->dist;
			r->b[i]->speed = dd/dt;
			r->b[i]->dist = dist;
			r->b[i]->period = find_period(r, dist);
		}
	}
}

Bus* find_bus(Road* r, char* name)
{
	for(int i=0; i<r->num_bus; i++) {
		if (strcmp(r->b[i]->name, name) == 0 ) {
			return r->b[i];
		}
	}
	return NULL;
}

void simulate_all_bus(Road* r, int dt)
{
	for(int i=0; i<r->num_bus; i++) {
		int si = r->b[i]->period;
		int speed = r->b[i]->speed;
		if ( r->b[i]->speed > r->s[si]->limit ) speed = r->s[si]->limit;

		r->b[i]->dist += speed*dt;
		r->b[i]->period = find_period(r, r->b[i]->dist);
	}
}

void simulate_bus(Road* r, Bus* b, int t[3])
{
	int dt = diff_time(t, b->time);
	for(int i=0; i<3; i++) b->time[i] = t[i];

	int si = b->period;
	int speed = b->speed;
	if ( b->speed > r->s[si]->limit ) speed = r->s[si]->limit;

	b->dist += speed*dt;
	b->period = find_period(r, b->dist);
}

/*
void update_locations(Road* r, char* buffer)
{
	int		t[3];
	char	name[5];
	char*	save_ptr;
	int		dist;
	int		num_bus = 0;
	int		dt = 0;

	char*	ptr = strtok_r(buffer, "#", &save_ptr);
	sscanf(buffer, "%d:%d:%d", &t[0], &t[1], &t[2]);

	if ( r->num_bus == 0 ) { // first location
		for(int i=0; i<3; i++) r->time[i] = t[i];
	} else {
		dt = diff_time(t, r->time);
		for(int i=0; i<3; i++) r->time[i] = t[i];
	}

	num_bus = 0;
	while ( ( ptr = strtok_r(NULL, "#", &save_ptr) ) != NULL ) {
		sscanf(ptr, "%[^,] %*c %d", name, &dist);

		if ( r->num_bus == 0 ) { // first location
			r->b[num_bus] = new_bus(name, dist);
			r->b[num_bus++]->period = find_period(r, dist);
		} else {
			find_bus_and_update(r, name, dist, dt);
			num_bus++;
		}
	}

	// location lost
	if ( num_bus == 0 ) {
		simulate_all_bus(r, dt);
	}

	// first location
	if (r->num_bus == 0 ) r->num_bus = num_bus;
}
*/

void update_location(Road* r, char* name, char* buffer)
{
	int		t[3];
	int		dist;
	char*	save_ptr;
	int		dt = 0;

	char*	ptr = strtok_r(buffer, "#", &save_ptr);
	sscanf(buffer, "%d:%d:%d", &t[0], &t[1], &t[2]);

	Bus* b = find_bus(r, name);

	int location_updated = 0;
	while ( ( ptr = strtok_r(NULL, "#", &save_ptr) ) != NULL ) {

		sscanf(ptr, "%d", &dist);
		location_updated = 1;
		if ( b == NULL ) { // first location
			r->b[r->num_bus] = new_bus(name, dist);
			for(int i=0; i<3; i++) r->b[r->num_bus]->time[i] = t[i];
			r->b[r->num_bus]->period = find_period(r, dist);
			r->num_bus++;

		} else {
			dt = diff_time(t, b->time);
			for(int i=0; i<3; i++) b->time[i] = t[i];

			int dd = dist - b->dist;
			b->speed = dd/dt;
			b->dist = dist;
			b->period = find_period(r, dist);
		}
	}


	// location lost
	if ( b != NULL && location_updated == 0 ) {
		simulate_bus(r, b, t);
	}
}

void add_station(Road* r, char* buffer)
{
	char name[4];
	int	dist, limit;

	sscanf(buffer, "%[^#] %*c %d # %d", name, &dist, &limit);
	limit = (limit*1000) / (60*60);	// km/h --> m/s
	r->s[r->num_station++] = new_station(name, dist, limit);
}

/////////////////////////////////////////////////////////////////////

int compare_dist(const void* a, const void* b)
{
	Bus* b1 = *(Bus**) a;
	Bus* b2 = *(Bus**) b;

	if (b1->dist < b2->dist) return -1;
	else if (b1->dist > b2->dist ) return 1;
	else return 0;
}

int compare_name(const void* a, const void* b)
{
	Bus* b1 = *(Bus**) a;
	Bus* b2 = *(Bus**) b;

	if (strcmp(b1->name, b2->name) < 0 ) return -1;
	else if (strcmp(b1->name, b2->name) > 0 ) return 1;
	else return 0;

}

void order_bus_by_dist(Road* r)
{
	qsort(r->b, r->num_bus, sizeof(Bus*), compare_dist);
}

void order_bus_by_name(Road* r)
{
	qsort(r->b, r->num_bus, sizeof(Bus*), compare_name);
}


/////////////////////////////////////////////////////////////////////


void find_prepost(Road* r)
{
	order_bus_by_dist(r);
	r->b[0]->prev = NULL;
	r->b[0]->next = r->b[1];
	for(int i=1; i<r->num_bus-1; i++) {
		r->b[i]->prev = r->b[i-1];
		r->b[i]->next = r->b[i+1];
	}
	r->b[r->num_bus-1]->prev = r->b[r->num_bus-2];
	r->b[r->num_bus-1]->next = NULL;
}

void find_arrival(Road* r)
{
	order_bus_by_dist(r);
	for(int i=0; i<r->num_station; i++) {
		int found = -1;
		for(int j=r->num_bus-1; j >= 0; j--) {
			if ( r->b[j]->dist <= r->s[i]->dist ) {
				found = j;
				break;
			}
		}
		if ( found == -1 ) r->s[i]->arrival = NULL;
		else r->s[i]->arrival = r->b[found];
	}
}

void find_signage(Road* r)
{
	order_bus_by_dist(r);
	for(int i=0; i<r->num_station; i++) {
		int bus_idx = -1;
		int min_time = INT_MAX;
		int temp_time = 0;

		for(int j=0; j < r->num_bus; j++) {

			if ( r->b[j]->dist <= r->s[i]->dist ) {
				// first period
				int p = r->b[j]->period;

				int speed = r->b[j]->speed;
				if ( r->s[p]->limit < speed ) speed = r->s[p]->limit;
				temp_time = (r->s[p+1]->dist - r->b[j]->dist ) / speed;

				// remained period
				for(int k=p+1; k<i; k++) {
					speed = r->b[j]->speed;
					if ( r->s[k]->limit < speed ) speed = r->s[k]->limit;
					temp_time += (r->s[k+1]->dist - r->s[k]->dist) / speed;
				}

				// store min time & bus index
				if (min_time > temp_time) {
					min_time = temp_time;
					bus_idx = j;
				}
			}
		}
		r->s[i]->sign_bus = r->b[bus_idx];
		r->s[i]->sign_time = min_time;
	}

}

int find_time_to_station(Road* r, int b, int s)
{
	if ( r->b[b]->dist > r->s[s]->dist ) return -1;
	else {
		int p = r->b[b]->period;

		// set max speed
		int speed = r->b[b]->speed;
		if (r->s[p]->limit < speed ) speed = r->s[p]->limit;

		int temp_time = (r->s[p+1]->dist - r->b[b]->dist ) /speed;

		for(int k=p+1; k<s; k++) {
			speed = r->b[b]->speed;
			if ( r->s[k]->limit < speed ) speed = r->s[k]->limit;
			temp_time += (r->s[k+1]->dist - r->s[k]->dist )/speed;
		}
		return temp_time;
	}
}

/////////////////////////////////////////////////////////////////////

void print_road_debug(Road* r)
{
	order_bus_by_dist(r);
	fprintf(stdout, "BUS:\n");
	for(int i=0; i<r->num_bus; i++) {
		fprintf(stdout, "%s, %05d, %d\n", r->b[i]->name, r->b[i]->dist, r->b[i]->speed);
	}
	fprintf(stdout, "\n");
/*
	fprintf(stdout, "STATION:\n");
	for(int i=0; i<r->num_station; i++) {
		fprintf(stdout, "%s, %05d, %03d\n", r->s[i]->name, r->s[i]->dist, r->s[i]->limit);
	}
	fprintf(stdout, "\n");
*/

}

void print_prepost(Road *r, FILE *out)
{
	order_bus_by_name(r);
	if ( out == stdout ) printf("PREPOST.TXT\n");
	for(int i=0; i<r->num_bus; i++) {
		fprintf(out, "%02d:%02d:%02d#", r->b[i]->time[0], r->b[i]->time[1], r->b[i]->time[2]);
		fprintf(out, "%s#", r->b[i]->name);
		if ( r->b[i]->next == NULL ) fprintf(out, "NOBUS,00000#");
		else fprintf(out, "%s,%05d#", r->b[i]->next->name, r->b[i]->next->dist - r->b[i]->dist);
		if ( r->b[i]->prev == NULL ) fprintf(out, "NOBUS,00000\n");
		else fprintf(out, "%s,%05d\n", r->b[i]->prev->name, r->b[i]->dist - r->b[i]->prev->dist);
	}
	if ( out == stdout ) printf("\n");
}

void print_arrival(Road* r, FILE* out)
{
	order_bus_by_dist(r);
	if ( out == stdout ) printf("ARRIVAL.TXT\n");
	for(int i=0; i<r->num_station; i++) {
		fprintf(out, "%02d:%02d:%02d#", r->b[0]->time[0], r->b[0]->time[1], r->b[0]->time[2]);
		fprintf(out, "%s#", r->s[i]->name);
		if ( r->s[i]->arrival == NULL ) fprintf(out, "NOBUS,00000\n");
		else fprintf(out, "%s,%05d\n", r->s[i]->arrival->name, r->s[i]->dist - r->s[i]->arrival->dist);
	}
	if ( out == stdout ) printf("\n");
}



void print_signage(Road* r, FILE* out)
{
	order_bus_by_dist(r);
	if ( out == stdout ) printf("SIGNAGE.TXT\n");
	for(int i=0; i<r->num_station; i++) {
		fprintf(out, "%02d:%02d:%02d#", r->b[0]->time[0], r->b[0]->time[1], r->b[0]->time[2]);
		fprintf(out, "%s#", r->s[i]->name);
		if ( r->s[i]->arrival == NULL ) fprintf(out, "NOBUS,00:00:00\n");
		else {
			int sign_time = r->b[0]->time[0]*60*60 + r->b[0]->time[1]*60 + r->b[0]->time[2] + r->s[i]->sign_time;
			int h = (sign_time)/(60*60);
			int m = (sign_time / 60) % 60;
			int s = (sign_time) % 60;
			fprintf(out, "%s,%02d:%02d:%02d\n", r->s[i]->sign_bus->name, h, m, s);
		}
	}
	if ( out == stdout ) printf("\n");

}


void int_to_time(int t, int* time)
{
	time[0] = (t)/(60*60);
	time[1] = (t / 60) % 60;
	time[2] = (t) % 60;
}

int time_to_int(int t[])
{
	return t[0]*60*60 + t[1]*60 + t[2];
}

/*
void process(Road* road)
{
	int		len;
	char	buffer[MAX_BUFFER_SIZE];
	FILE*	location = fopen("INFILE/LOCATION.TXT", "rt");
	FILE*	station = fopen("INFILE/STATION.TXT", "rt");
	FILE*	prepost = fopen("OUTFILE/PREPOST.TXT", "wt");
	FILE*	arrival = fopen("OUTFILE/ARRIVAL.TXT", "wt");
	FILE*	signage = popen("./SIGNAGE.EXE", "wt");


	// load station
	while ( fgets(buffer, sizeof(buffer), station ) != NULL ) {
		if ( ( len = sscanf(buffer, "%s", buffer) ) == EOF ) break;
		add_station(road, buffer);
	}
//	print_road_debug(&road);

	// load location
	while ( fgets(buffer, sizeof(buffer), location ) != NULL ) {
		if ( ( len = sscanf(buffer, "%s", buffer) ) == EOF ) break;

		if (strcmp("PRINT", buffer) != 0 ) {
			update_locations(road, buffer);
		} else {
			find_prepost(road);
			find_arrival(road);
			find_signage(road);

			print_road_debug(road);
			print_prepost(road, stdout);
			print_arrival(road, stdout);
			print_signage(road, stdout);
		}
	}

	free_road(road);

	fclose(location);
	fclose(station);
	fclose(prepost);
	fclose(arrival);
	fclose(signage);

}
*/
GMutex mutex;

gboolean incoming_callback(GSocketService *service,
                           GSocketConnection *connection,
                           GObject *source_object,
                           gpointer user_data)
{

	int		len;
	char	buffer[MAX_BUFFER_SIZE];
	char	client[MAX_BUFFER_SIZE];

	Road*	r = &road;

	GError* error = NULL;
	GInputStream *is = g_io_stream_get_input_stream(G_IO_STREAM(connection));;
	GOutputStream* os = g_io_stream_get_output_stream(G_IO_STREAM(connection));

	memset(buffer, 0, sizeof(buffer));
	len = g_input_stream_read(is, buffer, sizeof(buffer)-1, NULL, &error);
	len = sscanf(buffer, "%s", client);

//	if (strcmp(client, "BUS03") == 0  || strcmp(client, "MOBILE") == 0) return FALSE;

	if (strncmp(client, "BUS", strlen("BUS")) == 0 ) {					// from BUS
		while (TRUE) {
			memset(buffer, 0, sizeof(buffer));
			len = g_input_stream_read(is, buffer, sizeof(buffer)-1, NULL, &error);
			if ( ( len = sscanf(buffer, "%s", buffer) ) == EOF ) {
				break;
			}

			if (strcmp(buffer, "PRINT") != 0 ) {

				g_mutex_lock(&mutex);
				update_location(r, client, buffer);
				print_road_debug(r);
				g_mutex_unlock(&mutex);

			} else {
				// do nothing
			}

		}
	} else if ( strncmp(client, "MOBILE", strlen("MOBILE") ) == 0 ) {	// from MOBILE
		while (TRUE) {
			memset(buffer, 0, sizeof(buffer));
			len = g_input_stream_read(is, buffer, sizeof(buffer)-1, NULL, &error);
			if ( ( len = sscanf(buffer, "%s", buffer) ) == EOF ) {
				break;
			}

			if (strcmp(buffer, "PRINT") == 0 ) {
				g_mutex_lock(&mutex);
				find_prepost(r);
				find_arrival(r);
				find_signage(r);

				print_prepost(r, stdout);
				print_arrival(r, stdout);
				print_signage(r, stdout);
				g_mutex_unlock(&mutex);

			} else if (strncmp(buffer, "STA", strlen("STA")) == 0 ) {
				char	station[MAX_BUFFER_SIZE];
				int		dist;
				sscanf(buffer, "%[^#] %*c %d", station, &dist);
				//sscanf(buffer, "%5s %*c %d", station, &dist);

				// find nearest bus station a from passenger and arrival time t_a
				int no_station = 0;
				int move_time = 0;
				for (int i=0; i<r->num_station-1; i++) {
					if ( r->s[i]->dist <= dist && r->s[i+1]->dist > dist ) {
						if ( r->s[i+1]->dist - dist <= dist - r->s[i]->dist) {
							no_station = i+1;
							move_time = (r->s[i+1]->dist - dist);
						} else {
							no_station = i;
							move_time = (dist - r->s[i]->dist);
						}
						break;
					}
				}

				int target_station = 0;
				for(int i=0; i<r->num_station; i++) {
					if ( strcmp(r->s[i]->name, station) == 0 ) {
						target_station = i;
						break;
					}
				}

				printf("%s, %d, %d, %d\n", station, dist, no_station, move_time);

				// find buses behind a when t_a later

				int min = INT_MAX;
				int target_bus = -1;
				for(int i=0; i<r->num_bus; i++) {
					int start = find_time_to_station(r, i, no_station);
					if ( start < 0 ) continue;
					int end = find_time_to_station(r, i, target_station);
					if ( min > end ) {
						min = end;
						target_bus = i;
					}
				}
				int target_time = time_to_int(r->b[target_bus]->time) + min;
				int time[3];
				int_to_time(target_time, time);
				char new_time[MAX_BUFFER_SIZE];
				sprintf(new_time, "%d:%d:%d", time[0], time[1], time[2]);

				printf("%02d:%02d:%02d", time[0], time[1], time[2]);

				g_output_stream_write(os, new_time, strlen(new_time), NULL, &error);
			}

		}

	}

	return FALSE;
}


#define MAX_SOCKET_SERVICE	10
#define PORT_NUMBER			9876
GMainLoop	*loop = NULL;


int main(int argc, char *argv[])
{

	// load station
	int		len;
	char	buffer[MAX_BUFFER_SIZE];
	FILE*	station = fopen("INFILE/STATION.TXT", "rt");

	while ( fgets(buffer, sizeof(buffer), station ) != NULL ) {
		if ( ( len = sscanf(buffer, "%s", buffer) ) == EOF ) break;
		add_station(&road, buffer);
	}
	fclose(station);

	// socket
	GError *error = NULL;
	GSocketService *service = g_threaded_socket_service_new(MAX_SOCKET_SERVICE);
	g_socket_listener_add_inet_port( G_SOCKET_LISTENER(service), PORT_NUMBER, NULL, &error);

	g_signal_connect(service, "run", G_CALLBACK(incoming_callback), NULL);
	g_socket_service_start(service);

	loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);


	g_socket_service_stop(service);
	g_socket_listener_close(G_SOCKET_LISTENER(service));

	return 0;
}
