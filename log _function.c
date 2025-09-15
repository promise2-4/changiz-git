int log_func(int argc, char *const argv[])
{
    int till = 0;
    char branch[MAX_FILENAME_LENGTH] = "commit/";
    char check_author[MAX_NAME_LENGTH] = "";
    int last_id;
    FILE *ID = fopen(".changiz/id_number", "r");
    fscanf(ID, "%d", &last_id);
    fclose(ID);
    last_id--;
    if (argc > 2)
    {
        if (strcmp(argv[2], "-n") == 0)
        {
            if (argv[3] == NULL)
            {
                fprintf(stderr, "please enter a valid command");
                return 1;
            }
            till = last_id - atoi(argv[3]);
        }
        if (strcmp(argv[2], "-branch") == 0)
        {
            if (argv[3] == NULL)
            {
                fprintf(stderr, "please enter a valid command");
                return 1;
            }
            struct dirent *search = search_in_directory(argv[3], ".changiz/branches");
            if (search == NULL)
            {
                fprintf(stderr, "This branch doesn't exist");
                return 1;
            }
            strcpy(branch, argv[3]);
            strcat(branch, "/");
        }
        if (strcmp(argv[2], "-author") == 0)
        {
            FILE *find_author = fopen(".changiz/author_list", "r");
            fscanf(find_author, "%[^\0]s", check_author);
            fclose(find_author);
        }
    }
    for (int i = last_id; i > till; i--)
    {
        if (argc > 2 && strcmp(argv[2], "-author") == 0)
        {
            char look_for[MAX_NAME_LENGTH] = "";
            sprintf(look_for, "(%d) %s", i, argv[3]);
            char *find = strstr(check_author, look_for);
            if (find == NULL)
            {
                continue;
            }
        }
        char str_id[100] = "";
        sprintf(str_id, "%d", i);
        char src_location[MAX_NAME_LENGTH] = "";
        strcpy(src_location, ".changiz/branches/masterbranch/");
        strcat(src_location, branch);
        strcat(src_location, str_id);
        strcat(src_location, "/data");
        if (argc > 2 && strcmp(argv[2], "-search") == 0)
        {
            int flag_exist = 0;
            for (int j = 3; j < argc; j++)
            {
                FILE *search_file = fopen(src_location, "r");
                char search_temp[MAX_MESSAGE_LENGTH] = "";
                fscanf(search_file, "%[^\0]s", search_temp);
                fclose(search_file);
                char *find_word = strstr(search_temp, argv[i]);
                if (find_word != NULL)
                {
                    flag_exist = 1;
                    break;
                }
            }
            if (!flag_exist)
            {
                continue;
            }
        }
        if (argc > 2 && strcmp(argv[2], "-since") == 0)
        {
            FILE *search_date = fopen(src_location, "r");
            char date_temp[MAX_MESSAGE_LENGTH] = "";
            fscanf(search_date, "%[^\0]s", date_temp);
            fclose(search_date);
            struct stat file_stat;
            time_t creation_time;
            if (stat(date_temp, &file_stat) == 0)
            {
                creation_time = file_stat.st_mtime;
            }
            struct tm time_temp;
            if (strptime(argv[3], "%Y-%m-%d %H:%M:%S", &time_temp) == NULL)
            {
                return -1;
            }
            time_t time_compare = mktime(&time_temp);
            double difference = difftime(creation_time, time_compare);
            if (difference < 0)
            {
                continue;
            }
        }
        if (argc > 2 && strcmp(argv[2], "-before") == 0)
        {
            FILE *search_date = fopen(src_location, "r");
            char date_temp[MAX_MESSAGE_LENGTH] = "";
            fscanf(search_date, "%[^\0]s", date_temp);
            fclose(search_date);
            struct stat file_stat;
            time_t creation_time;
            if (stat(date_temp, &file_stat) == 0)
            {
                creation_time = file_stat.st_mtime;
            }
            struct tm time_temp;
            if (strptime(argv[3], "%Y-%m-%d %H:%M:%S", &time_temp) == NULL)
            {
                return -1;
            }
            time_t time_compare = mktime(&time_temp);
            double difference = difftime(creation_time, time_compare);
            if (difference > 0)
            {
                continue;
            }
        }
        char temp[MAX_MESSAGE_LENGTH] = "";
        FILE *data = fopen(src_location, "r");
        fscanf(data, "%[^\r]s", temp);
        fclose(data);
        FILE *log = fopen(".changiz/log_file", "a");
        fprintf(log, "%s\n", temp);
        char on_branch[MAX_NAME_LENGTH] = "";
        if (strcmp(branch, "commit/") == 0)
        {
            strcpy(on_branch, "masterbranch/");
        }
        fprintf(log, "on branch: /%s\n\n", on_branch);
        fclose(log);
    }
    FILE *log_check = fopen(".changiz/log_file", "r");
    if (log_check == NULL)
    {
        fprintf(stderr, "Nothing match with your request,please try agian");
        return 0;
    }
    fclose(log_check);
    char command[MAX_COMMAND_LENGTH] = "";
    strcat(command, "cat");
    strcat(command, " ");
    strcat(command, ".changiz/log_file");
    system(command);
    FILE *delete = fopen(".changiz/log_file", "w");
    fclose(delete);
    return 0;
}
