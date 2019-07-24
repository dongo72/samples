int compare_func(gconstpointer a, gconstpointer b) {
        /*
        ComData *left = (ComData*)a;
        ComData * right = (ComData*)b;

        if ( left->num > right->num ) return 1;
        else if ( left->num == right->num ) return 0;
        else return -1;
        */

           int* x = (int*)a;
           int* y = (int*)b;
           return *x - *y;
}

int main(void)
{
        int data_array[7];
        for (int i=0; i<7; i++) data_array[i] = 7-i+1;
        int data = 0;
        GArray* array = g_array_sized_new(FALSE, FALSE, sizeof(int), 10);
        g_array_append_val(array, data);
        g_array_append_vals(array, data_array , 7);
        for(int i=0; i<array->len; i++) printf("%d ", g_array_index(array, int, i));
        printf("\n");
        g_array_sort(array, compare_func);
        for(int i=0; i<array->len; i++) printf("%d ", g_array_index(array, int, i));
        printf("\n");

        char test[] = "abcdef";
        test[0] = 'A';
        GString *s = g_string_new(test);
        printf("%s\n", s->str);

        g_string_free(s, TRUE);

        return 0;
}
