include <stdio.h>
#include <glib.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_NAME_SIZE 10
#define MAX_BUS_SIZE 100
#define MAX_STATION_SIZE 100
#define MAX_TIME_SIZE 100

/////////////////////////////////////////////////////////////////////////////////
// struct
/////////////////////////////////////////////////////////////////////////////////

typedef struct _bus_ {
        char type;
        char name[MAX_NAME_SIZE];
        int pos;
        int speed;

        // pre bus & post bus
        struct _bus_ *left;
        struct _bus_ *right;
} Bus;

typedef struct _station_ {
        char type;
        char name[MAX_NAME_SIZE];
        int pos;
        int speed;
} Station;

typedef struct _lane_status_ {
        // time
        int hour, min, sec;

        // bus
        Bus* bus[MAX_BUS_SIZE];
        int bus_count;

        // station
        Station* station[MAX_STATION_SIZE];
        int station_count;

        int missing;

} LaneStatus;

us* new_bus(char* name, int pos)
{
        Bus* bus = (Bus*) malloc(sizeof(Bus));
        memset(bus, 0, sizeof(Bus));

        bus->type = 'b';
        strcpy(bus->name, name);
        bus->pos = pos;

        return bus;
}

Station* new_station(char* name, int pos, int speed)
{
        Station* station = (Station*) malloc(sizeof(Station));
        memset(station, 0, sizeof(Station));

        station->type = 's';
        strcpy(station->name, name);
        station->pos = pos;
        station->speed = speed;

        return station;
}

/////////////////////////////////////////////////////////////////////////////////
// sorting
/////////////////////////////////////////////////////////////////////////////////

/*
void sort_bus_status_pos(LaneStatus *lane)
{
        Bus *temp;
        for(int i=0; i<lane->bus_count; i++) {
                for(int j=i+1; j<lane->bus_count; j++) {
                        if ( lane->bus[j]->pos < lane->bus[i]->pos ) {
                                temp = lane->bus[i];
                                lane->bus[i] = lane->bus[j];
                                lane->bus[j] = temp;
                        }
                }
        }
}

void sort_bus_status_name(LaneStatus *lane)
{
        Bus *temp;
        for(int i=0; i<lane->bus_count; i++) {
                for(int j=i+1; j<lane->bus_count; j++) {
                        if ( strcmp(lane->bus[j]->name, lane->bus[i]->name) == -1 ) {
                                temp = lane->bus[i];
                                lane->bus[i] = lane->bus[j];
                                lane->bus[j] = temp;
                        }
                }
        }
}
*/

void sort_bus_status_pos(Bus* bus[], int bus_count)
{
        Bus *temp;
        for(int i=0; i<bus_count; i++) {
                for(int j=i+1; j<bus_count; j++) {
                        if ( bus[j]->pos < bus[i]->pos ) {
                                temp = bus[i];
                                bus[i] = bus[j];
                                bus[j] = temp;
                        }
                }
        }
}

void sort_bus_status_name(Bus* bus[], int bus_count)
{
        Bus *temp;
        for(int i=0; i<bus_count; i++) {
                for(int j=i+1; j<bus_count; j++) {
                        if ( strcmp(bus[j]->name, bus[i]->name) == -1 ) {
                                temp = bus[i];
                                bus[i] = bus[j];
                                bus[j] = temp;
                        }
                }
        }

}

/////////////////////////////////////////////////////////////////////////////////
// file io, scanf, tok
/////////////////////////////////////////////////////////////////////////////////

oid update_bus_status(LaneStatus *lane, char* temp_buffer)
{
        int hour, min, sec;
        char name[MAX_NAME_SIZE];
        int pos;

        // backup previous bus
        Bus* bus[MAX_BUS_SIZE];

        // first token: PRINT or time
        char* ptr = strtok(temp_buffer, "#");
        sscanf(ptr, "%d:%d:%d", &hour, &min, &sec);


        // second token: bus pos
        ptr = strtok(NULL, "#");

        // if data is comming
        if ( ptr != NULL ) {
                int idx = 0;
                while ( ptr!= NULL ) {
                        sscanf(ptr, "%[^,] %*c %d", name, &pos);
                        bus[idx++] = new_bus(name, pos);
                        ptr = strtok(NULL, "#");
                }

                // calc speed
                if ( lane->bus_count > 0 ) {
                        sort_bus_status_name(lane->bus, lane->bus_count);
                        sort_bus_status_name(bus, idx);

                        int t = (hour-lane->hour)*3600 + (min-lane->min)*60 + (sec-lane->sec);  // in sec

                        for(int i=0; i<idx; i++) {
                                int d = bus[i]->pos - lane->bus[i]->pos;                        // in miter
                                bus[i]->speed = d / t * 3.6;                                    // in km/h
                        }
                }


                // free previous bus
                for(int i=0; i<lane->bus_count; i++) free(lane->bus[i]);
                lane->bus_count = 0;

                // store current bus
                for(int i=0; i<idx; i++) lane->bus[i] = bus[i];
                lane->bus_count = idx;
        } else {
                lane->missing++;
        }

        // update time
        lane->hour = hour; lane->min = min; lane->sec = sec;
}

oid load_station(LaneStatus* lane, char* temp_buffer)
{
        char name[MAX_NAME_SIZE];
        int pos, speed;

        int idx = lane->station_count;
        sscanf(temp_buffer, "%[^,] %*c %d # %d", name, &pos, &speed);
        Station *station = new_station(name, pos, speed);
        lane->station[idx++] = station;
        lane->station_count = idx;
}


/////////////////////////////////////////////////////////////////////////////////
// array & struct navigation
/////////////////////////////////////////////////////////////////////////////////

void calc_relative_pos(LaneStatus *lane)
{
        sort_bus_status_pos(lane->bus, lane->bus_count);
        for(int i=0; i<lane->bus_count; i++) {
                if ( i == 0 ) {
                        lane->bus[i]->left = NULL;
                        lane->bus[i]->right = lane->bus[i+1];
                } else if ( i == lane->bus_count -1 ) {
                        lane->bus[i]->left = lane->bus[i-1];
                        lane->bus[i]->right = NULL;
                } else {
                        lane->bus[i]->left = lane->bus[i-1];
                        lane->bus[i]->right = lane->bus[i+1];
                }
        }
}


void write_relative_pos(LaneStatus *lane)
{
        sort_bus_status_name(lane->bus, lane->bus_count);
        for(int i=0; i<lane->bus_count; i++) {
                Bus* b = lane->bus[i];
                printf("%02d:%02d:%02d#%s#", lane->hour, lane->min, lane->sec, b->name);
                if ( b->left == NULL ) printf("NOBUS,00000#");
                else printf("%s,%05d#", b->left->name, b->pos - b->left->pos);
                if ( b->right == NULL ) printf("NOBUS,00000\n");
                else printf("%s,%05d\n", b->right->name, b->right->pos - b->pos);
        }
}


void write_nearest_bus(LaneStatus *lane)
{
        sort_bus_status_pos(lane->bus, lane->bus_count);
        for(int i=0; i<lane->station_count; i++) {
                printf("%s#", lane->station[i]->name);
                int j;
                for(j=lane->bus_count-1; j>=0; j--) {
        //              printf("\n");
        //              printf("%d, %d, %d, %d\n", i, j, lane->bus[j]->pos, lane->station[i]->pos);
        //              printf("\n");
                        if (lane->bus[j]->pos < lane->station[i]->pos ) break;
                }
                if ( j < 0 ) printf("%s,%05d\n", "NOBUS", 0);
                else printf("%s,%05d\n", lane->bus[j]->name, lane->station[i]->pos - lane->bus[j]->pos);
        }
}

void calc_current_pos(LaneStatus* lane)
{
        for(int i=0; i<lane->bus_count; i++) {
                int pos = lane->bus[i]->pos;
                float left = lane->missing;;

                for(int j=0; j<lane->station_count-1; j++) {
                        // find first period
                        if ( pos >= lane->station[j]->pos && pos < lane->station[j+1]->pos ) {
                                int dist = lane->station[j+1]->pos - pos;
                                int speed = (lane->bus[i]->speed > lane->station[j]->speed) ? lane->bus[i]->speed : lane->station[j]->speed;
                                float time  = dist / speed;
                                if ( left < time ) {
                                        pos += speed * left;
                                } else {
                                        left = left - time;
                                        pos = lane->station[j+1]->pos;
                                }
                        }
                }
                lane->bus[i]->pos = pos;
        }
}
/////////////////////////////////////////////////////////////////////////////////
// free & print
/////////////////////////////////////////////////////////////////////////////////

void free_lane_status(LaneStatus *lane)
{
        for(int i=0; i<lane->bus_count; i++) free(lane->bus[i]);
        for(int i=0; i<lane->station_count; i++) free(lane->station[i]);
        lane->bus_count = lane->station_count = 0;
}

void print_bus_status(LaneStatus *lane)
{
        sort_bus_status_name(lane->bus, lane->bus_count);
        printf("###### BUS STATUS ######\n");
        printf("%02d:%02d:%02d\n", lane->hour, lane->min, lane->sec);
        for(int i=0; i<lane->bus_count; i++) printf("%s,%05d,%03d\n", lane->bus[i]->name, lane->bus[i]->pos, lane->bus[i]->speed);
        printf("\n");
        printf("\n");
}

void print_station(LaneStatus *lane)
{
        printf("###### STATION ######\n");
        for(int i=0; i<lane->station_count; i++) printf("%s,%05d,%03d\n", lane->station[i]->name, lane->station[i]->pos, lane->station[i]->speed);
        printf("\n");
        printf("\n");
}

/////////////////////////////////////////////////////////////////////////////////
// main
/////////////////////////////////////////////////////////////////////////////////

int main(void)
{
        char temp_buffer[MAX_BUFFER_SIZE+1];
        FILE *f = NULL;


        // data variable
        LaneStatus lane;
        memset(&lane, 0, sizeof(LaneStatus));

        f = fopen("./station.txt", "r");
        while ( fgets(temp_buffer, MAX_BUFFER_SIZE, f) != NULL ) {
                temp_buffer[strlen(temp_buffer)-1] = '\0';

                load_station(&lane, temp_buffer);
        }
        fclose(f);


        f = fopen("./location.txt", "r");
        while ( fgets(temp_buffer, MAX_BUFFER_SIZE, f) != NULL ) {

                // remove return char
                temp_buffer[strlen(temp_buffer)-1] = '\0';

                if ( strcmp(temp_buffer, "PRINT") == 0 ) break;
                else if ( strcmp(temp_buffer, "") == 0 ) continue;
                else update_bus_status(&lane, temp_buffer);
        }
        fclose(f);

        print_bus_status(&lane);
        print_station(&lane);

        // problem 1
        calc_relative_pos(&lane);
        write_relative_pos(&lane);

        // problem 2
        write_nearest_bus(&lane);

        // problem 3
        calc_current_pos(&lane);
        print_bus_status(&lane);

        free_lane_status(&lane);
        return 0;
}
