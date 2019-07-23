#include <stdio.h>
#include <glib.h>
#include <gio/gio.h>

gboolean incoming_callback(GSocketService *service,
                           GSocketConnection *connection,
                           GObject *source_object,
                           gpointer user_data)
{
        g_print("Received Connection from Client\n");

        GInputStream *istream = NULL;
        GOutputStream *ostream = NULL;
        char message[1024+1];
        GError *error = NULL;
        gsize read_count = 0;
        gboolean read_finished = FALSE;

        ////////////////////////////////////////////////////////////////////////////////////////
        //   1. create input stream
        ////////////////////////////////////////////////////////////////////////////////////////
        istream = g_io_stream_get_input_stream(G_IO_STREAM(connection));
        ostream = g_io_stream_get_output_stream(G_IO_STREAM(connection));

        ////////////////////////////////////////////////////////////////////////////////////////
        //   2. receive and processing
        ////////////////////////////////////////////////////////////////////////////////////////
                while ( TRUE ) {

                // read input stream
                read_count = g_input_stream_read(istream, message, 1024, NULL, &error);
//              read_finished =  g_input_stream_read_all(istream, message, 1024, &read_count, NULL, &error) ;

                // mark end of string
                message[read_count] = '\0';

                // error handling
                if ( error != NULL ) {
                        g_print("Connect to port Failed\n");
                        return 1;
                }

                // processing
                if ( read_count > 0 )
                        g_print("%ld byte Received: %s\n", read_count, message);
                else if ( read_count == 0 )
                        break;

                g_output_stream_write(ostream, message, read_count, NULL, &error);
                // error handling
                if ( error != NULL ) {
                        g_print("Connect to port Failed\n");
                        return 1;
                }
        }

        g_print("Message: connection terminated\n");
  
        g_main_loop_quit(loop);
        return FALSE;
}

GMainLoop *loop = NULL;

int main(int argc, char **argv)
{

        GError *error = NULL;

        ////////////////////////////////////////////////////////////////////////////////////////
        //   1. create new socket service
        ////////////////////////////////////////////////////////////////////////////////////////

        // non threaded socket service
        // GSocketService* service = g_socket_service_new();

        // threaded socket service
        GSocketService *service = g_threaded_socket_service_new(10);

        ////////////////////////////////////////////////////////////////////////////////////////
        //   2. add listener
        ////////////////////////////////////////////////////////////////////////////////////////

        /*
        // get localhost inet address
        GInetAddress *inet_addr = g_inet_address_new_any(G_SOCKET_FAMILY_IPV4);
        if ( ! inet_addr ) inet_addr = g_inet_address_new_any(G_SOCKET_FAMILY_IPV6);

        // get socket address
        GSocketAddress *sock_addr = g_inet_socket_address_new(inet_addr, 1500);

        // add listener
        GSocketAddress *effective_addr;
        g_socket_listener_add_address(G_SOCKET_LISTENER(service),
                                     sock_addr,
                                     G_SOCKET_TYPE_STREAM,
                                     G_SOCKET_PROTOCOL_TCP,
                                     NULL,
                                     &effective_addr,
                                     &error);
        */
        
        // helper function for g_socket_listener_add_address
        g_socket_listener_add_inet_port( G_SOCKET_LISTENER(service),
                        1500,   // port
                        NULL,
                        &error);

        ////////////////////////////////////////////////////////////////////////////////////////
        //   3. check error
        ////////////////////////////////////////////////////////////////////////////////////////

        // error check
        if ( error != NULL ) {
                g_print("Connect to port Failed\n");
                return 1;
        } else {
                g_print("Connect to port Successed\n");
        }

        ////////////////////////////////////////////////////////////////////////////////////////
        //   4. add signal
        ////////////////////////////////////////////////////////////////////////////////////////

        // listen to the incoming signal
        /*
        g_signal_connect(service,                       // instance to connect
                        "incoming",                     // a string of the form "signal-name::detail"
                        G_CALLBACK(incoming_callback),  // the GCallback to connect
                        NULL);                          // data to pass to c_handler calls
        */

        // listen to the run signal for threaded_service
        g_signal_connect(service,                       // instance to connect
                        "run",                  // a string of the form "signal-name::detail"
                        G_CALLBACK(incoming_callback),  // the GCallback to connect
                        NULL);                          // data to pass to c_handler calls

        ////////////////////////////////////////////////////////////////////////////////////////
        //   5. start socket service
        ////////////////////////////////////////////////////////////////////////////////////////

        // start the socket service
        g_socket_service_start(service);
        
        
        ////////////////////////////////////////////////////////////////////////////////////////
        //   6. enter main loop
        ////////////////////////////////////////////////////////////////////////////////////////

        // enter main loop
        g_print("Waiting for client\n");
        loop = g_main_loop_new(NULL, FALSE);
        g_main_loop_run(loop);

        ////////////////////////////////////////////////////////////////////////////////////////
        //   7. stop service
        ////////////////////////////////////////////////////////////////////////////////////////

        // stop service when out of the main loop
        g_socket_service_stop(service);
  
        // close listener
        g_socker_listener_close(G_SOCKET_LISTENER(service))

        return 0;

}
