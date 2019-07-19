int main(void)
{

        GError *error = NULL;

        // create a new connection
        GSocketConnection *connection = NULL;
        GSocketClient *client = g_socket_client_new();

        // connect to the host
        connection = g_socket_client_connect_to_host(client,
                                                    (gchar*)"localhost",
                                                    1500, // your port goest here
                                                    NULL,
                                                    &error);

        // check error
        if (error != NULL) {
                g_print("Connection failed\n");
                return -1;
        } else {
                g_print("Connection successful\n");
        }


        char [][256] = {
                {"ID01", "10", "TEAM1", "ABCDE" },
                {"ID02", "200", "TEAM2", "QAZQWE" },
                {"ID03", "6", "TEAM3", "THANKS" },
                {"ID04", "3456", "TEAM4", "BLUE SKY" },
        };
        
                char message[1024+1];
        gsize read_count = 0;
        GOutputStream *ostream = g_io_stream_get_output_stream(G_IO_STREAM(connection));
        GInputStream *istream = g_io_stream_get_input_stream(G_IO_STREAM(connection));

        while ( TRUE ) {
                // get user input
                fgets(message, 1024, stdin);
                message[strlen(message)-1] = '\0'; // delete cr
                if (strlen(message) == 0 ) break;


                g_output_stream_write(ostream,
                        message,                // message go here
                        strlen(message),        // length of message
                        NULL,
                        &error);

                // check error
                if (error != NULL) {
                        g_print("Sending Data Failed\n");
                        return 1;
                }


                // replay back impl
                read_count = g_input_stream_read(istream,
                        message,
                        1024,
                        NULL,
                        &error);

                if (error != NULL) {
                        g_print("Sending Data Failed\n");
                        return 1;
                }

                if ( read_count > 0 ) {
                        message[read_count] = '\0';
                        g_print("%ld byte Returned: %s\n", read_count, message);
                }

        }
        g_print("Message: client terminated\n");


        return 0;

}
