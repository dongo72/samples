#define MAX_BUFFER_SIZE 128

pthread_mutex_t mutex;
pthread_cond_t cond;
GMainLoop *loop = NULL;

gboolean callback(GSocketService* service, GSocketConnection *conn, GObject* *object, gpointer user_data) {
	GError			*error = NULL;

	int				len;
	char			buffer[MAX_BUFFER_SIZE] = {0x00, };

	GInputStream	*is = g_io_stream_get_input_stream(G_IO_STREAM(conn));
	GOutputStream	*os = g_io_stream_get_output_stream(G_IO_STREAM(conn));

	// ##############################################################################


	printf("Server - (2/5) Connection Accepted\n");

	while (TRUE) {
		memset(buffer, 0, sizeof(buffer));
		len = g_input_stream_read(is, buffer, sizeof(buffer)-1, NULL, &error);
		if ( ( len = sscanf(buffer, "%s", buffer) ) == EOF ) {
			break;
		}
		printf("Server - (3/5) Client Message: %s\n", buffer);


		strcat(buffer, " is also good");
		printf("Server - (4/5) Feedback Message: %s\n", buffer);
		g_output_stream_write(os, buffer, strlen(buffer), NULL, &error);
	}

	printf("Server - (5/5) Connection Closed\n");

	// ##############################################################################

//	g_main_loop_quit(loop);
	return FALSE;
}

gpointer server_thread_func(gpointer data)
{
	GError		*error = NULL;
	GSocketService	*service = g_socket_service_new();

	g_socket_listener_add_inet_port(G_SOCKET_LISTENER(service), 9876, NULL, &error);
	g_signal_connect(service, "incoming", G_CALLBACK(callback), NULL);
	g_socket_service_start(service);

	///////////////////////////////////

	printf("Server - (1/5) Ready\n");

	//////////////////////////////////

	loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);

	// after loop close
	g_socket_service_stop(service);
	g_socket_listener_close(G_SOCKET_LISTENER(service));
	return NULL;
}

gpointer server_thread_native_func(gpointer data)
{
	struct sockaddr_in	server_addr;
	int server_socket = socket(PF_INET, SOCK_STREAM, 0);
	server_addr.sin_family = AF_INET;					// IPv4
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);	// 32bit IPv4
	server_addr.sin_port = htons(9876);					// port num

	// bind socket with server address
	bind(server_socket, (struct sockaddr*) &server_addr, sizeof(server_addr) );

	// listen with 5 queue
	listen(server_socket, 5);

	printf("Server - (1/5) Ready\n");


	// ##############################################################################


	while (TRUE) {

		// connect with client addr
		struct sockaddr_in	client_addr;
		int client_addr_size = sizeof(client_addr);
		int client_socket = accept(server_socket, (struct sockaddr*) &client_addr, &client_addr_size );
		printf("Server - (2/5) Connection Accepted\n");
		// printf("%s:%d accepted\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

		char buffer[MAX_BUFFER_SIZE] = {0x00, };
		int	len;

		while ( TRUE ) {

			memset(buffer, 0, sizeof(buffer));
			len = read(client_socket, buffer, sizeof(buffer)-1 );

			if ( strcmp(buffer, "quit") == 0 ) break;	// quit with "quit

			printf("Server - (3/5) Client Message: %s\n", buffer);

	//		g_output_stream_write(os, buffer, strlen(buffer), NULL, &error);
			strcat(buffer, " is also good");
			printf("Server - (4/5) Feedback Message: %s\n", buffer);
			write(client_socket, buffer, sizeof(buffer));
		}

		close(client_socket);
		printf("Server - (5/5) Connection Closed\n");
	}

	close(server_socket);

	return NULL;
}

gpointer client_thread_func(gpointer data)
{
	GError				*error = NULL;

	GSocketClient		*client = g_socket_client_new();
	GSocketConnection	*conn = g_socket_client_connect_to_host(client, (gchar*)"localhost", 9876, NULL,	&error);

	GOutputStream *os = g_io_stream_get_output_stream(G_IO_STREAM(conn));
	GInputStream *is = g_io_stream_get_input_stream(G_IO_STREAM(conn));

	// ##############################################################################

	char buffer[MAX_BUFFER_SIZE] = {0x00, };
	char message[MAX_BUFFER_SIZE] = {0x00, };
	int len;

	printf("Client - (1/5) Connection Established\n");

	while ( fgets(buffer, sizeof(buffer), stdin) != NULL ) {
		if( (len = sscanf(buffer, "%s", buffer)) == EOF ) continue;	// ignore with empty input
		if ( strcmp(buffer, "quit") == 0 ) break;					// quit with "quit"

		printf("Client - (2/5) Input Message: %s\n", buffer);

		strcat(buffer, " is good");
		printf("Client - (3/5) Send Message: %s\n", buffer);
		g_output_stream_write(os, buffer, strlen(buffer), NULL, &error);

		gsize read_count = g_input_stream_read(is, message, sizeof(message)-1, NULL, &error);
		message[read_count] = '\0';
		printf("Client - (4/5) Feedback Message: %s\n", message);

	}

	g_output_stream_write(os, buffer, strlen(buffer), NULL, &error);
	printf("Client - (5/5) Connection Closed\n");

	// ##############################################################################

	g_object_unref(client);
	return NULL;
}

int main(int argc, char *argv[]){

    GThread* server_thread = g_thread_new(NULL, server_thread_native_func, NULL);
    GThread* client_thread = g_thread_new(NULL, client_thread_func, NULL);

    g_thread_join(server_thread);
    g_thread_join(client_thread);

	return 0;
}
