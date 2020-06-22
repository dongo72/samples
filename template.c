#include <glib.h>
#include <gio/gio.h>

#define MAX_BUFFER_SIZE 128


//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// console input
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
void console_input(void) {
	int len = 0;
	char buffer[MAX_BUFFER_SIZE];
	memset(buffer, 0, sizeof(buffer));

	while ( fgets(buffer, sizeof(buffer), stdin) != NULL ) {
		if( (len = sscanf(buffer, "%s", buffer)) == EOF ) break;	// quit with empty input

		// ##############################################################################
		// default print

		printf("%s\n", buffer);
		for(int i=0; i<strlen(buffer); i++) printf("%02X ", buffer[i]); printf("\n\n");

		// ##############################################################################
	}

}


//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// file input
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
void file_input(void) {
	int len = 0;
	FILE *fp = NULL;

	char buffer[MAX_BUFFER_SIZE];
	memset(buffer, 0, sizeof(buffer));

	fp = fopen("test.txt", "rt");
	while ( fgets(buffer, sizeof(buffer), fp) != NULL ) {
		if( (len = sscanf(buffer, "%s", buffer)) == EOF ) continue;	// skip with empty line

		// ##############################################################################
		// default print

		printf("%s\n", buffer);
		for(int i=0; i<strlen(buffer); i++) printf("%02X ", buffer[i]); printf("\n\n");

		// ##############################################################################

	}

	fclose(fp);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// folder navigation with recursion
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
int indent = 0;
void folder_input_recur(char* target) {

	if ( chdir(target) != 0 ) return;

//	FILE *fp = fopen("dgjang.txt", "w");
//	fclose(fp);


	// open folder
	DIR *dp = NULL;
	if ( (dp = opendir(".")) == NULL ) return;

	struct dirent *de;
	while ( (de = readdir(dp)) != NULL ) {

		// #############  for indent  #############
		for(int i=0; i<indent; i++) printf("\t");

		if ( de->d_type == DT_REG ) printf("%s\n", de->d_name);
		else if ( de->d_type == DT_DIR ) {
			printf("[%s]\n", de->d_name);
			if ( strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0 ) {
				indent++;
				folder_input_recur(de->d_name);
			}
		}
		else printf("type unknown: %s\n", de->d_name);
	}

	closedir(dp);

	// #############  for indent  #############
	indent--;

	chdir("..");
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// folder navigation with queue
//
//////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_ITEM_SIZE 100
void folder_input_list(char* target) {

	// init folder queue
	char dirs[MAX_ITEM_SIZE][MAX_BUFFER_SIZE];
	memset(dirs, 0, sizeof(dirs));

	// chdir to target
	if ( chdir(target) != 0 ) return;

	// copy first dir to folder queue
	getcwd(dirs[0], sizeof(dirs[0]));
	int ndirs = 1;

	// folder queue loop
	for(int i=0; i<ndirs; i++) {
		printf("### start of folder %s\n", dirs[i]);

		// open folder
		DIR *dp = NULL;
		if ( (dp = opendir(dirs[i])) == NULL ) return;

		struct dirent *de;
		while ( (de = readdir(dp)) != NULL ) {

			if ( de->d_type == DT_REG ) printf("%s\n", de->d_name);
			else if ( de->d_type == DT_DIR ) {
				printf("[%s]\n", de->d_name);
				if ( strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0 ) {
					sprintf(dirs[ndirs++], "%s/%s", dirs[i], de->d_name);
				}
			}
			else printf("type unknown: %s\n", de->d_name);
		}

		closedir(dp);

		printf("### end of folder %s\n", dirs[i]);
	}
}


void string_tokenize() {
	char* ptr = gps;
	ptr = strtok(ptr, "#");
	printf("%s %d\n", ptr, strlen(ptr));
	while ( ( ptr = strtok(NULL, "#")) != NULL ) {
		printf("%s %d\n", ptr, strlen(ptr));

	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// socket server in c native
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
void socket_server_native() {
	int					server_socket;
	int					client_socket;

	struct sockaddr_in	server_addr;
	struct sockaddr_in	client_addr;
	int					client_addr_size;

	char				buffer[MAX_BUFFER_SIZE] = {0x00, };
	int					buffer_len;

//	memset(&server_addr, 0, sizeof(server_addr));
//	memset(&client_addr, 0, sizeof(client_addr));

	server_socket = socket(PF_INET, SOCK_STREAM, 0);

	server_addr.sin_family = AF_INET;					// IPv4
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);	// 32bit IPv4
	server_addr.sin_port = htons(4000);					// port num

	// bind socket with server address
	if ( bind(server_socket, (struct sockaddr*) &server_addr, sizeof(server_addr) ) < 0 ) {
		printf("bind error\n");
		exit(1);
	}
	printf("bind success\n");

	// listen with 5 queue
	if ( listen(server_socket, 5) < 0 ) {
		printf("listen fail\n");
		exit(1);
	}
	printf("listen success\n");

	// connect with client addr
	client_addr_size = sizeof(client_addr);
	if ( (client_socket = accept(server_socket, (struct sockaddr*) &client_addr, &client_addr_size ) ) < 0 ) {
		printf("accept fail\n");
	}
	printf("%s:%d accepted\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));


	// ##############################################################################
	//
	//


	for(int i=0; i<100; i++) {
		memset(buffer, 0, sizeof(buffer));
//		if ( ( buffer_len = read(client_socket, buffer, sizeof(buffer)-1) ) < 0 ) {	// for null terminating
		if ( ( buffer_len = recv(client_socket, buffer, sizeof(buffer)-1, 0 ) ) < 0 ) {
			printf("read error\n");
		}
		printf("len: %d, msg: %s", buffer_len, buffer);

		if (strncmp(buffer, "quit", 4) == 0 ) break;


	}


//	char msg[] = "Hello this is server\n";
//	write(client_socket, msg, sizeof(msg));
//	printf("send message\n");

	//
	//
	// ##############################################################################


	close(client_socket);
	close(server_socket);
	printf("\n\nclose socket\n");


}


//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// socket server with glib without thread terminated
//
//////////////////////////////////////////////////////////////////////////////////////////////////////

GMainLoop *loop = NULL;

gboolean callback(GSocketService* service, GSocketConnection *conn, GObject* *object, gpointer user_data) {
	GError			*error = NULL;

	int				len;
	char			buffer[MAX_BUFFER_SIZE] = {0x00, };

	GInputStream	*istream = g_io_stream_get_input_stream(G_IO_STREAM(conn));
	GOutputStream	*ostream = g_io_stream_get_output_stream(G_IO_STREAM(conn));

	// ##############################################################################
	//
	//

	// 1. receive filename
	len = g_input_stream_read(istream, buffer, sizeof(buffer)-1, NULL, &error);
	buffer[len] = '\0';

//	char filename[MAX_BUFFER_SIZE] = {0x00, };
//	sprintf(filename, "%s/%s", "")
//	FILE *fp = fopen("")

	printf("Filename: %s\n", buffer);

	// 2. open file and send
	for(int i=0; i<10; i++) {
		// send file by line
		sprintf(buffer, "%d th line: %s", i, "File Contents\n");
		g_output_stream_write(ostream, buffer, strlen(buffer), NULL, &error);

		// receive response
		len = g_input_stream_read(istream, buffer, sizeof(buffer)-1, NULL, &error);
		buffer[len] = '\0';

		if ( strcmp(buffer, "ACK") == 0 ) {

		} else if ( strcmp(buffer, "ERR") == 0 ) {

		} else {
			int line;
			sscanf(buffer, "%d", &line);
			i = line;
		}

	}

	g_output_stream_write(ostream, buffer, strlen(buffer), NULL, &error);

	//
	//
	// ##############################################################################

	g_main_loop_quit(loop);
	return FALSE;
}

void socket_server_glib() {

	GError		*error = NULL;
	GSocketService	*service = g_socket_service_new();

	g_socket_listener_add_inet_port(G_SOCKET_LISTENER(service), 9876, NULL, &error);
	g_signal_connect(service, "incoming", G_CALLBACK(callback), NULL);
	g_socket_service_start(service);

	loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);

	// after loop close
	g_socket_service_stop(service);
	g_socket_listener_close(G_SOCKET_LISTENER(service));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// socket server with glib without thread not terminate
//
//////////////////////////////////////////////////////////////////////////////////////////////////////

gboolean callback_thread(GSocketService* service, GSocketConnection *conn, GObject* *object, gpointer user_data) {
	GError			*error = NULL;

	int				len;
	char			buffer[MAX_BUFFER_SIZE] = {0x00, };

	GInputStream	*istream = g_io_stream_get_input_stream(G_IO_STREAM(conn));
	GOutputStream	*ostream = g_io_stream_get_output_stream(G_IO_STREAM(conn));

	// ##############################################################################


	// 1. receive filename
	len = g_input_stream_read(istream, buffer, sizeof(buffer)-1, NULL, &error);
	buffer[len] = '\0';

	g_output_stream_write(ostream, buffer, strlen(buffer), NULL, &error);


	// ##############################################################################

	return FALSE;
}



void socket_server_glib_thread() {

	GError		*error = NULL;
	GSocketService	*service = g_threaded_socket_service_new(20);

	g_socket_listener_add_inet_port(G_SOCKET_LISTENER(service), 4000, NULL, &error);
	g_signal_connect(service, "run", G_CALLBACK(callback_thread), NULL);
	g_socket_service_start(service);

	// main loop
	loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);

	// after loop close
//	g_socket_service_stop(service);
//	g_socket_listener_close(G_SOCKET_LISTENER(service));
}



void external_program_call() {

	FILE* pp = popen("SINAGE.EXE", "w");

	for(int i=0; i<10; i++) {
		fprintf(pp, "%d, %s", i, "Hello World\n");
	}

	pclose(pp);
}

pthread_mutex_t mutex;
pthread_cond_t cond;
int no_threads = 0;
int flag = 0;

void *thread_func1(void* data) {

	printf("Thread Function\n");
	for(int i=0; i<100; i++) {
		pthread_mutex_lock(&mutex);

		printf("ping : %d", i);
		flag = 1;

		pthread_cond_signal(&cond);
		pthread_cond_wait(&cond, &mutex);

		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

void *thread_func2(void* data) {
	printf("Thread Function\n");
	for(int i=0; i<100; i++) {
		pthread_mutex_lock(&mutex);

		while ( flag == 0 ) pthread_cond_wait(&cond, &mutex);

		printf("ping : %d", i);

		pthread_cond_signal(&cond);

		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

void thread_native() {
	pthread_t thread_id1, thread_id2;
	pthread_create(&thread_id1, NULL, thread_func1, NULL);
	pthread_create(&thread_id2, NULL, thread_func2, NULL);

	pthread_join(thread_id1, NULL);
	pthread_join(thread_id2, NULL);

}


GMutex g_mutex;
GCond g_cond;

gpointer do_write(gpointer data) {
	for(int i=0; i<100; i++) {
		g_mutex_lock(&g_mutex);

		printf("ping : %d", i);
		flag = 1;

		g_cond_signal(&g_cond);
		g_cond_wait(&g_cond, &g_mutex);

		g_mutex_unlock(&g_mutex);
	}
	return NULL;
}

gpointer do_read(gpointer data) {
	for(int i=0; i<100; i++) {
		g_mutex_lock(&g_mutex);

		while ( flag == 0 ) g_cond_wait(&g_cond, &g_mutex);

		printf("ping: %d", i);

		g_cond_signal(&g_cond);

		g_mutex_unlock(&g_mutex);
	}
	return NULL;
}

void thread_glib() {
	GThread *thread_id1, *thread_id2;

	thread_id1 = g_thread_new(NULL, do_write, NULL);
	thread_id2 = g_thread_new(NULL, do_read, NULL);

	g_thread_join(thread_id1);
	g_thread_join(thread_id2);
}




//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// socket input
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
void socket_client_native(void) {

	int					client_socket;
	struct sockaddr_in	server_addr;

	// create socket
	client_socket = socket(PF_INET, SOCK_STREAM, 0);

	// set server address
	//	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(4000);

	// connect socket with server address
	if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr) ) == -1 ) {
		printf("connect error\n");
		exit(1);
	}
	printf("connect success\n");


	// ##############################################################################

	char buffer[MAX_BUFFER_SIZE] = {0x00, };
	int len;

	/*
	for(int i=0; i<10; i++) {
		sprintf(buffer, "%d, Hello World\n", i);
		write(client_socket, buffer, strlen(buffer) );
		//send(client_socket, buffer, strlen(buffer), 0 );

		//printf("%s\n", buffer);
	}

	sprintf(buffer, "quit");
	write(client_socket, buffer, strlen(buffer) );
	//send(client_socket, buffer, strlen(buffer), 0 );


//	if ( ( buffer_len = read(client_socket, buffer, sizeof(buffer)-1) ) < 0 ) {
//		printf("read error\n");
//	}
//	printf("%d, %s\n", buffer_len, buffer);
*/


	while ( fgets(buffer, sizeof(buffer), stdin) != NULL ) {
		if( (len = sscanf(buffer, "%s", buffer)) == EOF ) break;	// quit with empty input
		printf("input %s\n", buffer);

		write(client_socket, buffer, len);

//		recv_len = read(client_socket, recevBuffer, sizeof(recevBuffer));
//		printf("%d, %s\n", recv_len, recevBuffer);


		// ##############################################################################
	}

	close(client_socket);
	printf("close success\n");

}

void socket_client_glib(void) {

	GError				*error = NULL;
	GSocketConnection	*conn = NULL;

	GSocketClient		*client = g_socket_client_new();
	conn = g_socket_client_connect_to_host(client, (gchar*)"localhost",	4000, NULL,	&error);

	GOutputStream *ostream = g_io_stream_get_output_stream(G_IO_STREAM(conn));
//	GInputStream *istream = g_io_stream_get_input_stream(G_IO_STREAM(conn));


	// ##############################################################################

//	char message[MAX_BUFFER_SIZE] = {0x00, };
//	for(int i=0; i<10; i++) {
//		sprintf(message, "%d, %s", i, "Hello World\n");
//		g_output_stream_write(ostream, message,	strlen(message), NULL, &error);
//		sleep(0.01);
//	}
//
//	sprintf(message, "%s", "quit");
//	g_output_stream_write(ostream,message, strlen(message), NULL, &error);


	char buffer[MAX_BUFFER_SIZE] = {0x00, };
	int len;

	while ( fgets(buffer, sizeof(buffer), stdin) != NULL ) {
		if( (len = sscanf(buffer, "%s", buffer)) == EOF ) break;	// quit with empty input
		printf("input %s\n", buffer);

		g_output_stream_write(ostream, buffer,	strlen(buffer), NULL, &error);

//		recv_len = read(client_socket, recevBuffer, sizeof(recevBuffer));
//		printf("%d, %s\n", recv_len, recevBuffer);


		// ##############################################################################
	}


//	gsize read_count = g_input_stream_read(istream, message, sizeof(message)-1, NULL, &error);
//	message[read_count] = '\0';

	// ##############################################################################

	g_object_unref(client);

}
