int search_file(char* base, char* target, char path[MAX_DATA_LENGTH]) {
	GDir *dir;
	GError *error;
	const gchar *filename;
	gchar *fullpath;

	dir = g_dir_open(base, 0, &error);
	while ( ( filename = g_dir_read_name(dir) ) ) {

		fullpath = g_build_filename(base, filename, (gchar*)NULL);

		if ( g_file_test(fullpath, G_FILE_TEST_IS_DIR) == TRUE)
			search_file(fullpath, target, path);
		else
			if (strcmp (filename, target) == 0 ) strcpy(path, fullpath);
    g_free(fullpath);
}

	g_dir_close(dir);
	return 0;
}
