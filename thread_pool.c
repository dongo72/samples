#define WAIT 5
#define MAX_THREADS 10
#define MAX_UNUSED_THREADS -1

/*
gint test_thread_sort_compare_func(gconstpointer a, gconstpointer b, gpointer user_data)
{
        guint32 id1, id2;
        id1 = GPOINTER_TO_UINT(a);
        id2 = GPOINTER_TO_UINT(b);

        return (id1>id2 ? +1 : id1 == id2 ? 0 : -1);
}
*/
void test_thread_sort_entry_func(gpointer data, gpointer user_data)
{
        //guint thread_id = GPOINTER_TO_INT(data);
        guint thread_id = *(int*)data;


        gboolean is_sorted = GPOINTER_TO_INT(user_data);;

        printf("-------> entered thread: %2.2d, is_sorted: %d\n", thread_id, is_sorted);
        g_usleep(WAIT*1000);
        printf("-------> ended thread: %2.2d, is_sorted: %d\n", thread_id, is_sorted);

}
int main(void)
{
        // create thread pool with user data := sorted

        guint           max_threads = 5;
        gboolean        sort = 1;
        GThreadPool *pool = g_thread_pool_new( (GFunc) test_thread_sort_entry_func,
                                  GINT_TO_POINTER(sort),         // user_data
                                  max_threads,                   // max_threads
                                  FALSE,                        // exclusive
                                  NULL);                        // error
        // g_thread_pool_set_max_unused_threads(MAX_UNUSED_THREADS);

        /*
        if (sort)
                g_thread_pool_set_sort_function(pool,
                                                test_thread_sort_compare_func,
                                                GUINT_TO_POINTER(69));

        */

        // push thread into thread pool with data := thread id

        int             thread_data[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
        guint           limit = 10;
        for(int i=0; i<limit; i++)
        {
                // push with int type data
//              guint id = g_random_int_range(1, limit) + 1;
//              guint id = thread_data[i];
//              g_thread_pool_push(pool, GUINT_TO_POINTER(id), NULL);


                gpointer id_data = &thread_data[i];
                g_thread_pool_push(pool, id_data, NULL);
                guint id = *(int*)(id_data);


                printf("===> pushed new thread with id:%d, number of thread:%d, unprocessed:%d\n", id, g_thread_pool_get_num_threads(pool), g_thread_pool_unprocessed(pool));
        }

         g_thread_pool_free(pool, FALSE/* immediate */, TRUE/* wait */);

        return 0;

}
