#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_FILENAME_LENGTH 1000
#define MAX_COMMIT_MESSAGE_LENGTH 2000
#define MAX_LINE_LENGTH 1000
#define MAX_MESSAGE_LENGTH 1000
#define MAX_COMMAND_LENGTH 1000
#define MAX_NAME_LENGTH 1000

int init(int argc, char *const argv[]);
int configs(int argc, char *const argv[]);

struct dirent *search_in_directory(char *search_file, char *address);

int check_for_add(char *filename, char *address);
int add(int argc, char *const argv[]);

int reset(int argc, char *const argv[]);

int commit(int argc, char *const argv[]);

int branch(int argc, char *const argv[]);

int log_func(int argc, char *const argv[]);

void print_command(int argc, char *const argv[])
{
    for (int i = 0; i < argc; i++)
    {
        fprintf(stdout, "%s ", argv[i]);
    }
    fprintf(stdout, "\n");
}

int init(int argc, char *const argv[])
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        return 1;
    }
    FILE *file_name = fopen("global_config_user_name", "r");
    FILE *file_email = fopen("global_config_user_email", "r");
    if ((file_name == NULL) && (file_email == NULL))
    {
        fprintf(stderr, "Error no user.name exists!\n");
        fprintf(stderr, "Error no user.email exists!");
        return 1;
    }
    else if ((file_name == NULL) && (file_email != NULL))
    {
        fprintf(stderr, "Error no user.name exists!");
        return 1;
    }
    else if ((file_name != NULL) && (file_email == NULL))
    {
        fprintf(stderr, "Error no user.email exists!");
        return 1;
    }
    else
    {
        char tmp_cwd[1024];
        bool exists = false;
        struct dirent *entry;
        do
        {
            DIR *dir = opendir(".");
            if (dir == NULL)
            {
                fprintf(stderr, "Error opening current directory");
                return 1;
            }
            while ((entry = readdir(dir)) != NULL)
            {
                if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".changiz") == 0)
                {
                    exists = true;
                }
            }
            closedir(dir);

            if (getcwd(tmp_cwd, sizeof(tmp_cwd)) == NULL)
            {
                return 1;
            }

            if (strcmp(tmp_cwd, "/") != 0)
            {
                if (chdir("..") != 0)
                {
                    return 1;
                }
            }

        } while (strcmp(tmp_cwd, "/") != 0);

        if (chdir(cwd) != 0)
        {
            return 1;
        }

        if (!exists)
        {
            if (mkdir(".changiz", 0755) != -1)
            {
                fprintf(stdout, "Initialized empty changiz repository");
                mkdir(".changiz/stage", 0755);
                mkdir(".changiz/masterbranch", 0755);
                mkdir(".changiz/masterbranch/commit", 0755);
                mkdir(".changiz/masterbranch/branches", 0755);
                FILE *fp = fopen(".changiz/save_staging_names", "w");
                fclose(fp);
                FILE *ID = fopen(".changiz/id_number", "w");
                fprintf(ID, "1");
                fclose(ID);
                FILE *log = fopen(".changiz/log_file", "w");
                fclose(log);
                return 1;
            }
        }
        else
        {
            fprintf(stderr, "changiz repository has already initialized");
        }
    }
    return 0;
}

int configs(int argc, char *const argv[])
{
    if (strcmp(argv[2], "-global") == 0)
    {
        if (strcmp(argv[3], "user.name") == 0)
        {
            FILE *file_name = fopen("global_config_user_name", "w");
            fprintf(file_name, "%s", argv[4]);
            fprintf(stdout, "global user.name config succsessfully");
            fclose(file_name);
            FILE *file_rn_name = fopen("current_user_name", "w");
            fprintf(file_rn_name, "%s", argv[4]);
            fclose(file_rn_name);
            return 1;
        }
        else if (strcmp(argv[3], "user.email") == 0)
        {
            FILE *file_email = fopen("global_config_user_email", "w");
            fprintf(file_email, "%s", argv[4]);
            fprintf(stdout, "global user.email config succsessfully");
            fclose(file_email);
            FILE *file_rn_email = fopen("current_user_email", "w");
            fprintf(file_rn_email, "%s", argv[4]);
            fclose(file_rn_email);
            return 1;
        }
    }
    else if (strcmp(argv[2], "user.name") == 0)
    {
        FILE *file_rn_name = fopen("current_user_name", "w");
        fprintf(file_rn_name, "%s", argv[3]);
        fprintf(stdout, "user.name config succsessfully");
        fclose(file_rn_name);
        return 1;
    }
    else if (strcmp(argv[2], "user.email") == 0)
    {
        FILE *file_rn_email = fopen("current_user_email", "w");
        fprintf(file_rn_email, "%s", argv[3]);
        fprintf(stdout, "user.email config succsessfully");
        fclose(file_rn_email);
        return 1;
    }
    return 0;
}

struct dirent *search_in_directory(char *search_file, char *address)
{
    struct dirent *entry;
    DIR *inside_dir = opendir(address);
    while ((entry = readdir(inside_dir)) != NULL)
    {
        if (strcmp(entry->d_name, search_file) == 0)
        {
            return entry;
        }
    }
    return NULL;
}

int check_for_add(char *filename, char *address)
{
    struct dirent *check;
    DIR *dir_stage = opendir(address);
    while ((check = readdir(dir_stage)) != NULL)
    {
        if (strcmp(check->d_name, filename) == 0)
        {
            char file_location[MAX_FILENAME_LENGTH] = "";
            strcpy(file_location, address);
            strcat(file_location, "/");
            strcat(file_location, check->d_name);

            char staged_file_location[MAX_FILENAME_LENGTH] = ".changiz/stage/";
            strcat(staged_file_location, address);
            strcat(staged_file_location, "/");
            strcat(staged_file_location, check->d_name);

            if (check->d_type == DT_REG)
            {
                char str1[10000];
                char str2[10000];

                FILE *new_file = fopen(file_location, "r");
                FILE *check_file = fopen(staged_file_location, "r");

                fscanf(new_file, "%[^\0]s", str1);
                fscanf(check_file, "%[^\0]s", str2);
                if (strcmp(str1, str2) != 0)
                {
                    return 0;
                }
                else if (strcmp(str1, str2) == 0)
                {
                    return 1;
                }
            }

            else if (check->d_type == DT_DIR)
            {
                DIR *inside_dir = opendir(file_location);
                struct dirent *check_inside;
                while ((check_inside = readdir(inside_dir)) != NULL)
                {
                    if ((strcmp(check_inside->d_name, ".") == 0) || (strcmp(check_inside->d_name, "..") == 0) || (strcmp(check_inside->d_name, ".DS_Store") == 0))
                    {
                        continue;
                    }
                    struct dirent *entry = search_in_directory(check_inside->d_name, staged_file_location);
                    if (entry == NULL)
                    {
                        return 0;
                    }
                    if (check_for_add(check_inside->d_name, file_location) == 0)
                    {
                        return 0;
                    }
                }
                return 1;
            }
        }
    }
    return 0;
}
int add(int argc, char *const argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "please specify a file\n");
        return 1;
    }
    if (strcmp(argv[2], "-redo") == 0)
    {
        struct dirent *entry_stage;
        DIR *dir_stage = opendir(".changiz/stage");
        if (dir_stage == NULL)
        {
            fprintf(stderr, "Error opening current directory\n");
            return 1;
        }
        while ((entry_stage = readdir(dir_stage)) != NULL)
        {
            if ((strcmp(entry_stage->d_name, ".") == 0) || (strcmp(entry_stage->d_name, "..") == 0) || (strcmp(entry_stage->d_name, ".DS_Store") == 0))
            {
                continue;
            }
            char command_redo[MAX_COMMAND_LENGTH] = "";
            sprintf(command_redo, "rsync -r %s %s", entry_stage->d_name, ".changiz/stage");
            system(command_redo);
        }
        fprintf(stdout, "All files and directories added successfully\n");
        return 1;
    }
    if (strcmp(argv[2], "-n") == 0)
    {
        char save_name[1000][1000];
        int name_counter = 0;
        struct dirent *staged_entry;
        DIR *dir_first = opendir(".");
        while ((staged_entry = readdir(dir_first)) != NULL)
        {
            if ((strcmp(staged_entry->d_name, ".") == 0) || (strcmp(staged_entry->d_name, "..") == 0) ||
                (strcmp(staged_entry->d_name, ".DS_Store") == 0))
            {
                continue;
            }
            struct dirent *find_entry = search_in_directory(staged_entry->d_name, ".changiz/stage/");
            if (find_entry != NULL)
            {
                if (find_entry->d_type == DT_REG)
                {
                    if (check_for_add(find_entry->d_name, ".") == 0)
                    {
                        strcpy(save_name[name_counter], find_entry->d_name);
                        strcat(save_name[name_counter], " (unstage)");
                        fprintf(stdout, "%s \n", save_name[name_counter]);
                        name_counter++;
                    }
                    else if (check_for_add(find_entry->d_name, ".") == 1)
                    {
                        strcpy(save_name[name_counter], find_entry->d_name);
                        strcat(save_name[name_counter], " (stage)");
                        fprintf(stdout, "%s \n", save_name[name_counter]);
                        name_counter++;
                    }
                }
                if (find_entry->d_type == DT_DIR)
                {
                    if (check_for_add(staged_entry->d_name, ".") == 0)
                    {
                        strcpy(save_name[name_counter], staged_entry->d_name);
                        strcat(save_name[name_counter], " (unstage)");
                        fprintf(stdout, "%s \n", save_name[name_counter]);
                        name_counter++;
                    }
                    else if (check_for_add(staged_entry->d_name, ".") == 1)
                    {
                        strcpy(save_name[name_counter], staged_entry->d_name);
                        strcat(save_name[name_counter], " (stage)");
                        fprintf(stdout, "%s \n", save_name[name_counter]);
                        name_counter++;
                    }
                }
            }

            else
            {
                strcpy(save_name[name_counter], staged_entry->d_name);
                strcat(save_name[name_counter], " (unstage)");
                fprintf(stdout, "%s \n", save_name[name_counter]);
                name_counter++;
            }
        }
        return 1;
    }
    int f = 0;
    if (strcmp(argv[2], "-f") == 0)
    {
        f++;
    }
    for (int i = 2 + f; i < argc; i++)
    {
        int exist = 0;
        DIR *dir = opendir(".");
        if (dir == NULL)
        {
            fprintf(stderr, "Error opening current directory\n");
            return 1;
        }
        closedir(dir);
        char filename[MAX_FILENAME_LENGTH] = "";
        char dir_name[MAX_FILENAME_LENGTH] = "";
        char *ptr_slash = strrchr(argv[i], '/');
        if (ptr_slash != NULL)
        {
            strcpy(filename, ptr_slash + 1);
            strncpy(dir_name, argv[i], ptr_slash - argv[i]);
        }
        else
        {
            strcpy(filename, argv[i]);
            strcpy(dir_name, ".");
        }
        struct dirent *entry;
        DIR *dir_check = opendir(dir_name);
        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, filename) == 0)
            {
                exist = 1;
                int is_staged = 0;
                struct dirent *stage_entry = search_in_directory(filename, ".changiz/stage");
                if (stage_entry != NULL)
                {
                    is_staged = check_for_add(argv[i], ".");
                }
                if (!is_staged)
                {
                    if (entry->d_type == DT_REG)
                    {
                        DIR *dir_stage = opendir(".changiz/stage");
                        char command[MAX_COMMAND_LENGTH] = "";
                        sprintf(command, "rsync -r %s %s/%s/", argv[i], ".changiz/stage", dir_name);
                        system(command);
                        FILE *fp = fopen(".changiz/save_staging_names", "a");
                        fprintf(fp, "%s|", argv[i]);
                        if (argc < 4)
                        {
                            fprintf(fp, "\n");
                        }
                        fclose(fp);
                        fprintf(stdout, "file %s added successfully\n", argv[i]);
                        closedir(dir);
                        break;
                    }
                    else if (entry->d_type == DT_DIR)
                    {
                        DIR *dir_stage = opendir(".changiz/stage");
                        char command[MAX_COMMAND_LENGTH] = "";
                        sprintf(command, "rsync -r %s %s/%s/", argv[i], ".changiz/stage", dir_name);
                        system(command);
                        FILE *fp = fopen(".changiz/save_staging_names", "a");
                        fprintf(fp, "%s|", argv[i]);
                        if (argc < 4)
                        {
                            fprintf(fp, "\n");
                        }
                        fclose(fp);
                        fprintf(stdout, "directory %s added successfully\n", argv[i]);
                        closedir(dir);
                        break;
                    }
                }
                else
                {
                    fprintf(stderr, "file is already staged\n");
                    break;
                }
            }
        }
        if (!exist)
        {
            fprintf(stdout, "file or directory doesn't exist\n");
        }
    }
    return 0;
}

int reset(int argc, char *const argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "please specify a file\n");
        return 1;
    }
    if (strcmp(argv[2], "-undo") == 0)
    {
        int line_counter = 0;
        int count_name = 0;
        char save_file[1000] = "";
        char save_line[1000][1000];
        char save_name[1000][1000];
        FILE *fp = fopen(".changiz/save_staging_names", "r");
        fscanf(fp, "%[^\0]s", save_file);
        fclose(fp);
        if (strlen(save_file) == 0)
        {
            fprintf(stdout, "Nothing exists to reset\n");
            return 0;
        }
        char *ptr_line = strtok(save_file, "\n");
        while (ptr_line != NULL)
        {
            strcpy(save_line[line_counter], ptr_line);
            line_counter++;
            ptr_line = strtok(NULL, "\n");
        }
        char *ptr_name = strtok(save_line[line_counter - 1], "|");
        while (ptr_name != NULL)
        {
            struct dirent *exist = search_in_directory(ptr_name, ".changiz/stage");
            if (exist == NULL)
            {
                fprintf(stderr, "No such a file or directory exist");
                FILE *edit = fopen(".changiz/save_staging_names", "w");
                for (int i = 0; i < line_counter - 1; i++)
                {
                    fprintf(edit, "%s\n", save_line[i]);
                }
                fclose(edit);
                return 0;
            }
            char command[MAX_COMMAND_LENGTH] = "";
            sprintf(command, "rm -r .changiz/stage/%s", ptr_name);
            system(command);
            count_name++;
            ptr_name = strtok(NULL, "|");
        }
        FILE *fp2 = fopen(".changiz/save_staging_names", "w");
        for (int i = 0; i < line_counter - 1; i++)
        {
            fprintf(fp, "%s\n", save_line[i]);
        }
        fclose(fp2);
        fprintf(stdout, "files or directories undo successfully\n");
        return 0;
    }
    int f = 0;
    if (strcmp(argv[2], "-f") == 0)
    {
        f++;
    }
    for (int i = 2 + f; i < argc; i++)
    {
        int exist = 0;
        struct dirent *entry;
        DIR *dir = opendir(".changiz/stage");
        if (dir == NULL)
        {
            fprintf(stderr, "Nothing exist to reset\n");
            return 1;
        }
        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, argv[i]) == 0)
            {
                exist = 1;
                if (entry->d_type == DT_REG)
                {
                    char command[MAX_COMMAND_LENGTH] = "";
                    sprintf(command, "rm -r .changiz/stage/%s", argv[i]);
                    system(command);
                    fprintf(stdout, "file %s reset successfully\n", argv[i]);
                    closedir(dir);
                    break;
                }
                else if (entry->d_type == DT_DIR)
                {
                    char command[MAX_COMMAND_LENGTH] = "";
                    sprintf(command, "rm -r .changiz/stage/%s", argv[i]);
                    system(command);
                    fprintf(stdout, "directory %s reset successfully\n", argv[i]);
                    closedir(dir);
                    break;
                }
            }
        }
        if (!exist)
        {
            fprintf(stdout, "file or directory doesn't exist to reset\n");
        }
    }
    return 0;
}

int commit(int argc, char *const argv[])
{
    if (argc < 4)
    {
        fprintf(stdout, "please enter a valid command");
        return 1;
    }
    if ((strlen(argv[3]) > 72) || (strlen(argv[3]) == 0))
    {
        fprintf(stdout, "please enter a valid message");
        return 1;
    }

    FILE *count = fopen(".changiz/save_staging_names", "r");
    if (count == NULL)
    {
        fprintf(stderr, "Nothing exist to commit");
        return 1;
    }
    char stage_string[MAX_NAME_LENGTH] = "";
    fscanf(count, "%[^\0]s", stage_string);
    int counter = 0;
    char *token = strtok(stage_string, "|");
    while (token != NULL)
    {
        token = strtok(NULL, "|");
        counter++;
    }
    counter--;
    fclose(count);
    FILE *delete = fopen(".changiz/save_staging_names", "w");
    fclose(delete);

    char str_id[100] = "";
    FILE *ID = fopen(".changiz/id_number", "r");
    fscanf(ID, "%[^\0]s", str_id);
    fclose(ID);

    char location[MAX_COMMAND_LENGTH] = ".changiz/masterbranch/commit/";
    strcat(location, str_id);
    int id = atoi(str_id);
    id++;

    FILE *ID2 = fopen(".changiz/id_number", "w");
    fprintf(ID2, "%d", id);
    fclose(ID2);
    mkdir(location, 0755);

    char command[MAX_COMMAND_LENGTH] = "";
    sprintf(command, "mv %s %s", ".changiz/stage/*", location);
    system(command);

    time_t cur = time(NULL);
    strcat(location, "/");
    strcat(location, "data");

    char author_name[MAX_NAME_LENGTH] = "";
    FILE *name = fopen("current_user_name", "r");
    fscanf(name, "%[^\0]s", author_name);
    fclose(name);

    char author_email[MAX_NAME_LENGTH] = "";
    FILE *email = fopen("current_user_email", "r");
    fscanf(email, "%[^\0]s", author_email);
    fclose(email);

    FILE *data = fopen(location, "w");
    fprintf(data, "commit id: %s\nComment: %s\nDate: %sAuthor: %s %s\ncount of commits: %d", str_id, argv[3], ctime(&cur), author_name, author_email, counter);
    fclose(data);

    FILE *author = fopen(".changiz/author_list", "a");
    fprintf(author, "(%s) %s\n", str_id, author_name);
    fclose(author);

    fprintf(stdout, "commited successfully\n\n");
    fprintf(stdout, "Commit id: %s\nComment: %s\nDate: %s", str_id, argv[3], ctime(&cur));
    return 1;
}

int branch(int argc, char *const argv[])
{
    if (argc == 3)
    {
        struct dirent *entry = search_in_directory(argv[2], ".changiz/masterbranch/branches");
        if (entry == NULL)
        {
            char dest_location[MAX_COMMAND_LENGTH] = ".changiz/masterbranch/branches/";
            strcat(dest_location, argv[2]);
            mkdir(dest_location, 0755);

            char last_commit[1000] = "";
            FILE *ID = fopen(".changiz/id_number", "r");
            fscanf(ID, "%[^\0]s", last_commit);
            fclose(ID);
            int last_id = atoi(last_commit);
            last_id--;
            sprintf(last_commit, "%d", last_id);
            char src_location[MAX_COMMAND_LENGTH] = ".changiz/masterbranch/commit/";
            strcat(src_location, last_commit);

            char command[MAX_COMMAND_LENGTH] = "";
            sprintf(command, "cp -r %s %s", src_location, dest_location);
            system(command);
            fprintf(stdout, "Branch '%s' set up to track remote branch 'masterbranch'", argv[2]);
            return 1;
        }
        else
        {
            fprintf(stderr, "A branch with this name already exist\n");
            return 0;
        }
    }
    if (argv[2] == NULL)
    {
        struct dirent *entry;
        DIR *branch_check = opendir(".changiz/masterbranch/branches");
        while ((entry = readdir(branch_check)) != NULL)
        {
            fprintf(stdout, "->%s\n", entry->d_name);
            return 1;
        }
        fprintf(stdout, "No branch exist\n");
        return 0;
    }
    fprintf(stderr, "please enter a valid command");
    return 0;
}

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
            struct dirent *search = search_in_directory(argv[3], ".changiz/masterbranch/branches");
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
        strcpy(src_location, ".changiz/masterbranch/");
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

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stdout, "please enter a valid command");
        return 1;
    }
    print_command(argc, argv);

    if (strcmp(argv[1], "init") == 0)
    {
        return init(argc, argv);
    }
    else if (strcmp(argv[1], "config") == 0)
    {
        return configs(argc, argv);
    }
    else if (strcmp(argv[1], "add") == 0)
    {
        return add(argc, argv);
    }
    else if (strcmp(argv[1], "reset") == 0)
    {
        return reset(argc, argv);
    }
    else if (strcmp(argv[1], "commit") == 0)
    {
        return commit(argc, argv);
    }
    else if (strcmp(argv[1], "branch") == 0)
    {
        return branch(argc, argv);
    }
    else if (strcmp(argv[1], "log") == 0)
    {
        return log_func(argc, argv);
    }
}