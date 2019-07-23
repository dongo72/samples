#include <stdio.h>
#include <glib.h>
#include <unistd.h>

#define WAIT 5
#define MAX_THREADS 5
#define MAX_UNUSED_THREADS -1

/*
int global = 1;

gpointer thread_func(gpointer data)
{
        int n = GPOINTER_TO_INT(data);
        for(int i=0; i<100; i++)
                printf("%d%d%d%d%d%d%d%d global = %d\n",n,n,n,n,n,n,n,n,global++);
        return NULL;
}


int main(void) {

        GThread *threads[MAX_THREADS];
        for(int i=0; i<MAX_THREADS; i++) {
                threads[i] = g_thread_new(NULL, thread_func, GINT_TO_POINTER(i));
        }

        for(int i=0; i<MAX_THREADS; i++) g_thread_join(threads[i]);

        return 0;
}
*/
struct com_data
{
        int a;
        int b;
};

GMutex mutex;
GCond cond;
int no_threads = 0;
struct com_data mydata;

gpointer do_write(gpointer data)
{
        mydata.a = 0;
        mydata.b = 0;

        while ( TRUE ) {
                g_mutex_lock(&mutex);

                mydata.a += 2;
                mydata.b += 2;

                g_cond_signal(&cond);

                g_mutex_unlock(&mutex);

                printf("%4d,  %4d Generated\n", mydata.a, mydata.b);
                sleep(1);
        }
        return NULL;
}

gpointer do_read(gpointer data)
{
        while ( TRUE ) {
                g_mutex_lock(&mutex);

                g_cond_wait(&cond, &mutex);

                printf("%4d + %4d = %4d\n", mydata.a, mydata.b, mydata.a + mydata.b);
                printf("=============================\n");

                g_mutex_unlock(&mutex);

        }

        return NULL;
}
int main(void)
{
        GThread *threads[MAX_THREADS];


        int a = 1;
        int b = 2;

        threads[0] = g_thread_new(NULL, do_write, (gpointer) &a);
        threads[1] = g_thread_new(NULL, do_read, (gpointer) &b);

        for(int i=0; i<2; i++) g_thread_join(threads[i]);


        // some code

        return 0;
}
