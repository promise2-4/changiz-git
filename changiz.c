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
struct dirent *HEAD_finder(char *address);

int check_for_add(char *filename, char *address);
int add(int argc, char *const argv[]);

int reset(int argc, char *const argv[]);

int commit(int argc, char *const argv[]);
int set(int argc, char *const argv[]);
void find_shortcut(char *shortcut_name, int *flag_not_exist);
int replace(int argc, char *const argv[]);
int remove_func(int argc, char *const argv[]);

int branch(int argc, char *const argv[]);

int log_func(int argc, char *const argv[]);

int checkout(int argc, char *const argv[]);

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
    FILE *file_name = fopen("config/global_config_user_name", "r");
    FILE *file_email = fopen("config/global_config_user_email", "r");
    if ((file_name == NULL) && (file_email == NULL))
    {
        fprintf(stderr, "Error no user.name exists!\n");
        fprintf(stderr, "Error no user.email exists!");
        mkdir("config", 0755);
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
                mkdir(".changiz/branches", 0755);
                mkdir(".changiz/data_saver", 0755);
                mkdir(".changiz/branches/masterbranch", 0755);
                FILE *current = fopen(".changiz/current_location", "w");
                fprintf(current, "masterbranch");
                fclose(current);
                FILE *commit_id = fopen(".changiz/current_commit_id", "w");
                fprintf(commit_id, "1");
                fclose(commit_id);
                FILE *fp = fopen(".changiz/save_staging_names", "w");
                fclose(fp);
                FILE *ID = fopen(".changiz/id_number", "w");
                fprintf(ID, "1");
                fclose(ID);
                FILE *log = fopen(".changiz/log_file", "w");
                fclose(log);
                FILE *author = fopen(".changiz/author_list", "w");
                fclose(author);
                FILE *branch = fopen(".changiz/branch_list", "w");
                fclose(branch);
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
            FILE *file_name = fopen("config/global_config_user_name", "w");
            fprintf(file_name, "%s", argv[4]);
            fprintf(stdout, "global user.name config succsessfully");
            fclose(file_name);
            FILE *file_rn_name = fopen("config/current_user_name", "w");
            fprintf(file_rn_name, "%s", argv[4]);
            fclose(file_rn_name);
            return 1;
        }
        else if (strcmp(argv[3], "user.email") == 0)
        {
            FILE *file_email = fopen("config/global_config_user_email", "w");
            fprintf(file_email, "%s", argv[4]);
            fprintf(stdout, "global user.email config succsessfully");
            fclose(file_email);
            FILE *file_rn_email = fopen("config/current_user_email", "w");
            fprintf(file_rn_email, "%s", argv[4]);
            fclose(file_rn_email);
            return 1;
        }
    }
    else if (strcmp(argv[2], "user.name") == 0)
    {
        FILE *file_rn_name = fopen("config/current_user_name", "w");
        fprintf(file_rn_name, "%s", argv[3]);
        fprintf(stdout, "user.name config succsessfully");
        fclose(file_rn_name);
        return 1;
    }
    else if (strcmp(argv[2], "user.email") == 0)
    {
        FILE *file_rn_email = fopen("config/current_user_email", "w");
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
struct dirent *HEAD_finder(char *address)
{
    struct dirent *entry;
    DIR *inside_address = opendir(address);
    int max_id = 0;
    while ((entry = readdir(inside_address)) != NULL)
    {
        int temp = atoi(entry->d_name);
        if (temp > max_id)
        {
            max_id = temp;
        }
    }
    char str_id[MAX_NAME_LENGTH] = "";
    sprintf(str_id, "%d", max_id);
    while ((entry = readdir(inside_address)) != NULL)
    {
        if (strcmp(entry->d_name, str_id) == 0)
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
            struct dirent *exist = search_in_directory(ptr_name, ".changiz/stage/");
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
    if (argc != 4)
    {
        fprintf(stdout, "please enter a valid command");
        return 1;
    }
    if ((strlen(argv[3]) > 72) || (strlen(argv[3]) == 0))
    {
        fprintf(stdout, "please enter a valid message");
        return 1;
    }
    if (strcmp(argv[2], "-s") == 0)
    {
        int flag_not_exist = 0;
        find_shortcut(argv[3], &flag_not_exist);
        if (flag_not_exist == 1)
        {
            return 0;
        }
    }

    FILE *count = fopen(".changiz/save_staging_names", "r");
    if (fgetc(count) == EOF)
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

    char temp_current_branch[MAX_NAME_LENGTH] = "";
    char branch[MAX_NAME_LENGTH] = "";
    FILE *current = fopen(".changiz/current_location", "r");
    fscanf(current, "%[^\0]s", temp_current_branch);
    strcpy(branch, temp_current_branch);
    char current_location[MAX_COMMAND_LENGTH] = ".changiz/branches/";
    strcat(current_location, branch);

    char temp_current_id[MAX_NAME_LENGTH] = "";
    FILE *commit_id = fopen(".changiz/current_commit_id", "r");
    fscanf(commit_id, "%[^\0]s", temp_current_id);
    fclose(commit_id);
    int temp_less_id = atoi(temp_current_id);
    temp_less_id--;
    sprintf(temp_current_id, "%d", temp_less_id);
    strcat(current_location, "/");
    strcat(current_location, temp_current_id);
    temp_less_id++;
    sprintf(temp_current_id, "%d", temp_less_id);

    char dest_location[MAX_NAME_LENGTH] = "";
    char *ptr_slash = strrchr(current_location, '/');
    if (ptr_slash != NULL)
    {
        strncpy(dest_location, current_location, ptr_slash - current_location);
    }
    strcat(dest_location, "/");
    strcat(dest_location, str_id);

    mkdir(dest_location, 0755);
    int id = atoi(str_id);
    if (id > 1)
    {
        char command_copy[MAX_COMMAND_LENGTH] = "";
        sprintf(command_copy, "cp -r %s/* %s", current_location, dest_location);
        system(command_copy);
    }
    id++;

    FILE *ID2 = fopen(".changiz/id_number", "w");
    fprintf(ID2, "%d", id);
    fclose(ID2);

    char command[MAX_COMMAND_LENGTH] = "";
    sprintf(command, "mv .changiz/stage/* %s", dest_location);
    system(command);

    char author_name[MAX_NAME_LENGTH] = "";
    FILE *name = fopen("config/current_user_name", "r");
    fscanf(name, "%[^\0]s", author_name);
    fclose(name);

    char author_email[MAX_NAME_LENGTH] = "";
    FILE *email = fopen("config/current_user_email", "r");
    fscanf(email, "%[^\0]s", author_email);
    fclose(email);

    time_t cur = time(NULL);
    char current_time[1000] = "";
    strcpy(current_time, ctime(&cur));

    FILE *log = fopen(".changiz/log_file", "r");
    char last_log[100000] = "";
    fscanf(log, "%[^\0]s", last_log);
    fclose(log);

    FILE *creat_log = fopen(".changiz/log_file", "w");
    fprintf(creat_log, "Commit id: %s\nComment: %s\nDate: %sAuthor: %s %s\nCount of commits: %d\nOn branch: %s\n\n", str_id, argv[3], current_time, author_name, author_email, counter, branch);
    fprintf(creat_log, "°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°\n\n");
    fclose(creat_log);

    char creat_data_file[MAX_FILENAME_LENGTH] = ".changiz/data_saver/";
    strcat(creat_data_file, str_id);
    FILE *write_data = fopen(creat_data_file, "w");
    fprintf(write_data, "Commit id: %s\nComment: %s\nDate: %sAuthor: %s %s\nCount of commits: %d\nOn branch: %s\n", str_id, argv[3], current_time, author_name, author_email, counter, branch);
    fclose(write_data);

    FILE *complete_log = fopen(".changiz/log_file", "a");
    fprintf(complete_log, "%s", last_log);
    fclose(complete_log);

    FILE *author = fopen(".changiz/author_list", "a");
    fprintf(author, "(%s) %s\n", str_id, author_name);
    fclose(author);

    FILE *save_branch = fopen(".changiz/branch_list", "a");
    fprintf(save_branch, "(%s) %s\n", str_id, branch);
    fclose(save_branch);

    FILE *add_commit_id = fopen(".changiz/current_commit_id", "w");
    int temp_id = atoi(temp_current_id);
    temp_id++;
    fprintf(add_commit_id, "%d", temp_id);
    fclose(add_commit_id);

    fprintf(stdout, "commited successfully\n\n");
    fprintf(stdout, "›Commit id: %s\n›Comment: %s\n›Date: %s", str_id, argv[3], ctime(&cur));
    return 1;
}

int set(int argc, char *const argv[])
{
    if (argc < 3 || argv[5] == NULL)
    {
        fprintf(stderr, "please enter a valid command\n");
        return 0;
    }
    FILE *shortcut = fopen(".changiz/saving_shortcuts", "a");
    fprintf(shortcut, "%s\n%s\n", argv[5], argv[3]);
    fclose(shortcut);
    fprintf(stdout, "Shortcut added successfully");
    return 1;
}

void find_shortcut(char *shortcut_name, int *flag_not_exist)
{
    FILE *shortcut = fopen(".changiz/saving_shortcuts", "r");
    char file_str[MAX_FILENAME_LENGTH] = "";
    char shortcut_line[MAX_LINE_LENGTH][MAX_LINE_LENGTH];
    int line_counter = 0;

    fscanf(shortcut, "%[^\0]s", file_str);
    fclose(shortcut);
    char *line = strtok(file_str, "\n");
    while (line != NULL)
    {
        strcpy(shortcut_line[line_counter], line);
        line_counter++;
        line = strtok(NULL, "\n");
    }
    int flag = 0;
    for (int i = 0; i < line_counter; i++)
    {
        if (strcmp(shortcut_line[i], shortcut_name) == 0)
        {
            strcpy(shortcut_name, shortcut_line[i + 1]);
            flag = 1;
            break;
        }
    }
    if (!flag)
    {
        fprintf(stderr, "this shortcut doesn't exist,please enter a valid command\n");
        *flag_not_exist = 1;
        return;
    }
    return;
}

int replace(int argc, char *const argv[])
{
    if (argc < 3 || argv[5] == NULL)
    {
        fprintf(stderr, "please enter a valid command\n");
        return 0;
    }
    FILE *shortcut = fopen(".changiz/saving_shortcuts", "r");
    char file_str[MAX_FILENAME_LENGTH] = "";
    char shortcut_line[MAX_LINE_LENGTH][MAX_LINE_LENGTH];
    int line_counter = 0;

    fscanf(shortcut, "%[^\0]s", file_str);
    fclose(shortcut);
    char *line = strtok(file_str, "\n");
    while (line != NULL)
    {
        strcpy(shortcut_line[line_counter], line);
        line_counter++;
        line = strtok(NULL, "\n");
    }
    int flag = 0;
    for (int i = 0; i < line_counter; i++)
    {
        if (strcmp(shortcut_line[i], argv[5]) == 0)
        {
            strcpy(shortcut_line[i + 1], argv[3]);
            flag = 1;
            break;
        }
    }
    if (!flag)
    {
        fprintf(stderr, "this shortcut doesn't exist,please enter a valid command");
        return 0;
    }
    FILE *edited = fopen(".changiz/saving_shortcuts", "w");
    for (int j = 0; j < line_counter; j++)
    {
        fprintf(edited, "%s\n", shortcut_line[j]);
    }
    fclose(edited);
    fprintf(stdout, "Shortcut changed successfully");
    return 1;
}

int remove_func(int argc, char *const argv[])
{
    if (argv[3] == NULL)
    {
        fprintf(stderr, "please enter a valid command\n");
        return 0;
    }
    FILE *shortcut = fopen(".changiz/saving_shortcuts", "r");
    char file_str[MAX_FILENAME_LENGTH] = "";
    char shortcut_line[MAX_LINE_LENGTH][MAX_LINE_LENGTH];
    int line_counter = 0;

    fscanf(shortcut, "%[^\0]s", file_str);
    fclose(shortcut);
    char *line = strtok(file_str, "\n");
    while (line != NULL)
    {
        strcpy(shortcut_line[line_counter], line);
        line_counter++;
        line = strtok(NULL, "\n");
    }
    int flag = 0;
    for (int i = 0; i < line_counter; i++)
    {
        if (strcmp(shortcut_line[i], argv[3]) == 0)
        {
            flag = 1;
            if (i == line_counter - 2)
            {
                break;
            }
            for (int j = i; j < line_counter; j++)
            {
                strcpy(shortcut_line[j], shortcut_line[j + 2]);
            }
            break;
        }
    }
    if (!flag)
    {
        fprintf(stderr, "this shortcut doesn't exist,please enter a valid command\n");
        return 0;
    }
    strcpy(shortcut_line[line_counter - 1], "\0");
    strcpy(shortcut_line[line_counter - 2], "\0");
    FILE *edited = fopen(".changiz/saving_shortcuts", "w");
    for (int z = 0; z < line_counter - 2; z++)
    {
        fprintf(edited, "%s\n", shortcut_line[z]);
    }
    fclose(edited);
    fprintf(stdout, "Shortcut removed successfully");
    return 1;
}

int branch(int argc, char *const argv[])
{
    if (argc == 3)
    {
        struct dirent *entry = search_in_directory(argv[2], ".changiz/branches");
        if (entry == NULL)
        {
            char dest_location[MAX_COMMAND_LENGTH] = ".changiz/branches/";
            strcat(dest_location, argv[2]);
            mkdir(dest_location, 0755);

            char last_commit[1000] = "";
            FILE *ID = fopen(".changiz/id_number", "r");
            fscanf(ID, "%[^\0]s", last_commit);
            strcat(dest_location, "/");
            strcat(dest_location, last_commit);
            mkdir(dest_location, 0755);
            fclose(ID);

            char temp_current[MAX_NAME_LENGTH] = "";
            FILE *current = fopen(".changiz/current_location", "r");
            fscanf(current, "%[^\0]s", temp_current);
            fclose(current);
            char make_current[MAX_NAME_LENGTH] = ".changiz/branches/";
            char temp_commit_id[MAX_NAME_LENGTH] = "";
            strcat(make_current, temp_current);
            FILE *commit_id = fopen(".changiz/current_commit_id", "r");
            fscanf(commit_id, "%[^\0]s", temp_commit_id);
            fclose(commit_id);
            strcat(make_current, "/");
            strcat(make_current, temp_commit_id);

            char command[MAX_COMMAND_LENGTH] = "";
            sprintf(command, "cp -r %s/* %s", make_current, dest_location);
            system(command);
            fprintf(stdout, "Branch '%s' set up to track remote branch 'masterbranch'", argv[2]);

            FILE *ID_plus = fopen(".changiz/id_number", "w");
            int last_id = atoi(last_commit);
            last_id++;
            fprintf(ID_plus, "%d", last_id);

            FILE *add_branch = fopen(".changiz/branch_list", "a");
            fprintf(add_branch, "(%s) %s\n", last_commit, argv[2]);
            fclose(add_branch);

            last_id--;
            FILE *add_commit_id = fopen(".changiz/current_commit_id", "w");
            fprintf(add_commit_id, "%d", last_id);
            fclose(add_commit_id);

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
        DIR *branch_check = opendir(".changiz/branches");
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
    if (argc == 2)
    {
        FILE *log_check = fopen(".changiz/log_file", "r");
        if (log_check == NULL)
        {
            fprintf(stderr, "Nothing match with your request,please try agian");
            return 0;
        }
        fclose(log_check);
        char command[MAX_COMMAND_LENGTH] = "";
        sprintf(command, "cat .changiz/log_file");
        system(command);
        return 1;
    }
    int till = 0;
    char check_author[MAX_NAME_LENGTH] = "";
    char check_branch[MAX_NAME_LENGTH] = "";
    int last_id;
    FILE *ID = fopen(".changiz/id_number", "r");
    fscanf(ID, "%d", &last_id);
    fclose(ID);
    last_id--;

    if (argc > 2)
    {
        if (strcmp(argv[2], "-n") == 0)
        {
            if (argc != 4)
            {
                fprintf(stderr, "please enter a valid command");
                return 1;
            }
            till = last_id - atoi(argv[3]);
        }
        if (strcmp(argv[2], "-branch") == 0)
        {
            if (argc != 4)
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
            FILE *find_branch = fopen(".changiz/branch_list", "r");
            fscanf(find_branch, "%[^\0]s", check_branch);
            fclose(find_branch);
        }
        if (strcmp(argv[2], "-author") == 0)
        {
            if (argc != 4)
            {
                fprintf(stderr, "please enter a valid command");
                return 1;
            }
            FILE *find_author = fopen(".changiz/author_list", "r");
            fscanf(find_author, "%[^\0]s", check_author);
            fclose(find_author);
        }
    }
    for (int i = last_id; i > till; i--)
    {
        if (argc > 2 && strcmp(argv[2], "-branch") == 0)
        {
            char look_for[MAX_NAME_LENGTH] = "";
            sprintf(look_for, "(%d) %s", i, argv[3]);
            char *find = strstr(check_branch, look_for);
            if (find == NULL)
            {
                continue;
            }
        }
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
        strcpy(src_location, ".changiz/data_saver/");
        strcat(src_location, str_id);
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
            fprintf(stderr, "here with commit id %d\n", i);
            struct stat file_stat;
            time_t creation_time;
            if (stat(src_location, &file_stat) == 0)
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
            struct stat file_stat;
            time_t creation_time;
            if (stat(src_location, &file_stat) == 0)
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
        char temp[10000] = "";
        FILE *data = fopen(src_location, "r");
        fscanf(data, "%[^\r]s", temp);
        fclose(data);
        FILE *log = fopen(".changiz/print_log", "a");
        fprintf(log, "%s\n\n", temp);
        fprintf(log, "°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°\n\n");
        fclose(log);
    }
    FILE *log_checker = fopen(".changiz/print_log", "r");
    if (log_checker == NULL)
    {
        fprintf(stderr, "Nothing match with your request,please try agian");
        return 0;
    }
    fclose(log_checker);
    char command_cat[MAX_COMMAND_LENGTH] = "";
    sprintf(command_cat, "cat .changiz/print_log");
    system(command_cat);
    FILE *delete = fopen(".changiz/print_log", "w");
    fclose(delete);
    return 0;
}

int checkout(int argc, char *const argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "please enter a valid command");
        return 1;
    }
    FILE *check_stage = fopen(".changiz/save_staging_names", "r");
    if (fgetc(check_stage) != EOF)
    {
        fprintf(stderr, "you need to commit changes first!");
        return 0;
    }
    if (atoi(argv[2]) == 0)
    {
        if (strcmp(argv[2], "HEAD") != 0)
        {
            struct dirent *exist_branch = search_in_directory(argv[2], ".changiz/branches");
            if (exist_branch == NULL)
            {
                fprintf(stderr, "This branch name doesn't exist");
                return 0;
            }
            else
            {
                char temp_current_branch[MAX_NAME_LENGTH] = "";
                FILE *current = fopen(".changiz/current_location", "r");
                fscanf(current, "%[^\0]s", temp_current_branch);
                fclose(current);
                char current_location[MAX_COMMAND_LENGTH] = ".changiz/branches/";
                strcat(current_location, temp_current_branch);

                char temp_current_id[MAX_NAME_LENGTH] = "";
                FILE *commit_id = fopen(".changiz/current_commit_id", "r");
                fscanf(commit_id, "%[^\0]s", temp_current_id);
                fclose(commit_id);
                int temp_less_id = atoi(temp_current_id);
                temp_less_id--;
                sprintf(temp_current_id, "%d", temp_less_id);
                strcat(current_location, "/");
                strcat(current_location, temp_current_id);
                temp_less_id++;
                sprintf(temp_current_id, "%d", temp_less_id);

                struct dirent *local_files;
                DIR *local = opendir(".");
                while ((local_files = readdir(local)) != NULL)
                {
                    if (local_files->d_name[0] == '.')
                    {
                        continue;
                    }
                    struct dirent *track = search_in_directory(local_files->d_name, current_location);
                    if (track == NULL)
                    {
                        fprintf(stderr, "You can't checkout because %s is not commit yet", local_files->d_name);
                        return 0;
                    }
                }
                closedir(local);
                char new_current[MAX_NAME_LENGTH] = ".changiz/branches/";
                strcat(new_current, argv[2]);
                struct dirent *head = HEAD_finder(new_current);
                strcat(new_current, "/");
                strcat(new_current, head->d_name);

                FILE *add_current = fopen(".changiz/current_location", "w");
                fprintf(add_current, "%s", argv[2]);
                fclose(add_current);

                FILE *add_commit_id = fopen(".changiz/current_commit_id", "w");
                fprintf(add_commit_id, "%s", head->d_name);
                fclose(add_commit_id);

                char command_rm[MAX_COMMAND_LENGTH] = "";
                sprintf(command_rm, "rm -r %s/*/!(.changiz) (config)", ".");
                system(command_rm);

                char command[MAX_COMMAND_LENGTH] = "";
                sprintf(command, "cp %s %s", new_current, ".");
                system(command);

                fprintf(stdout, "Checkout to branch %s done successfully", argv[2]);
                return 1;
            }
        }
        else
        {
            char temp_current[MAX_NAME_LENGTH] = "";
            FILE *current = fopen(".changiz/current_location", "r");
            fscanf(current, "%[^\0]s", temp_current);
            fclose(current);
            char make_current[MAX_NAME_LENGTH] = ".changiz/branches/";
            char temp_commit_id[MAX_NAME_LENGTH] = "";
            strcat(make_current, temp_current);

            struct dirent *head = HEAD_finder(make_current);
            char new_current[MAX_NAME_LENGTH] = "";
            strcpy(new_current, make_current);
            strcat(new_current, "/");
            strcat(new_current, head->d_name);

            FILE *commit_id = fopen(".changiz/current_commit_id", "w");
            fscanf(commit_id, "%[^\0]s", temp_commit_id);
            fclose(commit_id);
            strcat(make_current, "/");
            strcat(make_current, temp_commit_id);

            struct dirent *local_files;
            DIR *local = opendir(".");
            while ((local_files = readdir(local)) != NULL)
            {
                struct dirent *track = search_in_directory(local_files->d_name, make_current);
                if (track == NULL)
                {
                    fprintf(stderr, "You can't checkout because %s is not commit yet", local_files->d_name);
                    return 0;
                }
            }
            closedir(local);
            FILE *add_commit_id = fopen(".changiz/current_commit_id", "w");
            fprintf(add_commit_id, "%s", head->d_name);
            fclose(add_commit_id);

            char command_rm[MAX_COMMAND_LENGTH] = "";
            sprintf(command_rm, "rm -r %s/*/!(.changiz) (config)", ".");
            system(command_rm);

            char command[MAX_COMMAND_LENGTH] = "";
            sprintf(command, "cp %s %s", new_current, ".");
            system(command);

            fprintf(stdout, "Checkout to HEAD done successfully");
            return 1;
        }
    }
    else
    {
        char check_branch[MAX_FILENAME_LENGTH] = "";
        FILE *find_branch = fopen(".changiz/branch_list", "r");
        fscanf(find_branch, "%[^\0]s", check_branch);
        fclose(find_branch);

        char *find = strstr(check_branch, argv[2]);
        if (find == NULL)
        {
            fprintf(stderr, "This commit id doesn't exist");
            return 0;
        }
        else
        {
            char branch[MAX_NAME_LENGTH] = "";
            sscanf(find + 3, "%s", branch);
            printf("%s\n", branch);

            char temp_current_branch[MAX_NAME_LENGTH] = "";
            FILE *current = fopen(".changiz/current_location", "r");
            fscanf(current, "%[^\0]s", temp_current_branch);
            fclose(current);
            char current_location[MAX_COMMAND_LENGTH] = ".changiz/branches/";
            strcat(current_location, temp_current_branch);

            char temp_current_id[MAX_NAME_LENGTH] = "";
            FILE *commit_id = fopen(".changiz/current_commit_id", "r");
            fscanf(commit_id, "%[^\0]s", temp_current_id);
            fclose(commit_id);
            int temp_less_id = atoi(temp_current_id);
            temp_less_id--;
            sprintf(temp_current_id, "%d", temp_less_id);
            strcat(current_location, "/");
            strcat(current_location, temp_current_id);
            temp_less_id++;
            sprintf(temp_current_id, "%d", temp_less_id);

            struct dirent *local_files;
            DIR *local = opendir(".");
            while ((local_files = readdir(local)) != NULL)
            {
                if (local_files->d_name[0] == '.')
                {
                    continue;
                }
                struct dirent *track = search_in_directory(local_files->d_name, current_location);
                if (track == NULL)
                {
                    fprintf(stderr, "You can't checkout because %s is not commit yet", local_files->d_name);
                    return 0;
                }
            }
            closedir(local);

            struct dirent *delete_files;
            DIR *local_clean = opendir(".");
            while ((delete_files = readdir(local)) != NULL)
            {
                if (delete_files->d_name[0] == '.')
                {
                    continue;
                }
                char command_rm[MAX_COMMAND_LENGTH] = "";
                sprintf(command_rm, "rm -r %s", delete_files->d_name);
                system(command_rm);
            }
            closedir(local_clean);

            char new_current[MAX_FILENAME_LENGTH] = ".changiz/branches/";
            strcat(new_current, branch);
            strcat(new_current, "/");
            strcat(new_current, argv[2]);

            char command[MAX_COMMAND_LENGTH] = "";
            sprintf(command, "cp -r %s/* %s", new_current, ".");
            system(command);

            FILE *add_current = fopen(".changiz/current_location", "w");
            fprintf(add_current, "%s", branch);
            fclose(add_current);

            FILE *add_commit_id = fopen(".changiz/current_commit_id", "w");
            fprintf(add_commit_id, "%s", argv[2]);
            fclose(add_commit_id);

            fprintf(stdout, "Checkout to commit %s done successfully", argv[2]);
            return 1;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "please enter a valid command");
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
    else if (strcmp(argv[1], "set") == 0)
    {
        return set(argc, argv);
    }
    else if (strcmp(argv[1], "replace") == 0)
    {
        return replace(argc, argv);
    }
    else if (strcmp(argv[1], "remove") == 0)
    {
        return remove_func(argc, argv);
    }
    else if (strcmp(argv[1], "branch") == 0)
    {
        return branch(argc, argv);
    }
    else if (strcmp(argv[1], "log") == 0)
    {
        return log_func(argc, argv);
    }
    else if (strcmp(argv[1], "checkout") == 0)
    {
        return checkout(argc, argv);
    }
}