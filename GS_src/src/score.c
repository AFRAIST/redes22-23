
#include "score.h"
#include "serv_game.h"

u32 total_scores = 0;


int score_function(u32 n_succ){
    float score_value = (float)n_succ / g_serv_game->trials * 100;
    return round(score_value);
}

DIR *dir1 = NULL;
int count_scores(){
    int count = 0;

    const char dir_path[PATH_SIZE] = "sv_data/score";

    if (access(dir_path, F_OK) != 0) {
        return count;
    } 

    dir1 = opendir(dir_path);
    if(dir1 == NULL){
        return count;
    }

    // Read each directory entry one by one
    struct dirent* iter;
    while ((iter = readdir(dir1)) != NULL) {
        // Increment the counter for each entry
        if (strcmp(iter->d_name, ".") == 0 || strcmp(iter->d_name, "..") == 0) {
            continue;
        }
        count++;
    }

    // Close the directory stream
    closedir(dir1);
    dir1 = NULL;
    return count;
}

DIR *dir2;
int remove_lowest_alphabetic(){

    dir2 = opendir("sv_data/score");
    if(dir2 == NULL){
        perror("There is no File");
        return EXIT_FAILURE;
    }

    char lowest_name[PATH_SIZE];
    char full_link[PATH_SIZE]; 
    struct dirent* iter;

    lowest_name[0] = '\0';

    while ((iter = readdir(dir2)) != NULL) {
    // Skip the "." and ".." entries
        if (strcmp(iter->d_name, ".") == 0 || strcmp(iter->d_name, "..") == 0) {
            continue;
        }

        // Compare the name of the current file with the lowest alphabetic name so far
        if (lowest_name[0] == '\0' || strcmp(iter->d_name, lowest_name) < 0) {
        // Update the lowest alphabetic name and dirent structure
            strcpy(lowest_name,iter->d_name);
        }
    }

    sprintf(full_link, "sv_data/score/%s", lowest_name);

    if (remove(full_link) != 0) {
        perror("Error deleting file");
        return EXIT_FAILURE;
    }
    VerbosePrintF("Deleted file: %s\n", full_link);

    closedir(dir2);

    return EXIT_SUCCESS;
}

Result filter_scoreboard(){
    total_scores = count_scores();

    if(total_scores > TOP_SCORE){
        while(total_scores > TOP_SCORE){
            if(remove_lowest_alphabetic() == EXIT_FAILURE){
                perror("Error");
                return EXIT_FAILURE;
            }
            else
                total_scores--;
            
        }
    } 
    return EXIT_SUCCESS;
}

Result save_score(struct output *outp, const char* word){

    const char *dir = "sv_data/score";
    u32 n_succ = (g_serv_game->trials - g_serv_game->errors);
    if (access(dir, F_OK) == 0) {
        printf("Directory %s exists\n", dir);
    } else {
        if(mkdir(dir, S_IRWXU) != 0) perror(E_SERVER_ERROR);
    }

    FILE *fp;
    int score = score_function(n_succ);
    time_t t = time(NULL); 
    struct tm current_time = *localtime(&t);

    char date[DATE_SIZE];
    strftime(date, sizeof(date), "%d%m%Y_%H%M%S", &current_time);

    printf("Date: %s\n", date);

    char file_name[PATH_SIZE];
    snprintf(file_name, sizeof(file_name), "sv_data/score/%03i_%06zu_%s.txt", score, outp->plid, date);
    printf("%lu",strlen(file_name));

    fp = fopen(file_name, "w");

    // Check if the file was successfully created
    if (fp == NULL) {
        perror("Error while opening the file.\n");
        return EXIT_FAILURE;
    }

    if (flock(fileno(fp), LOCK_EX) == -1) {
        perror("Flock.\n");
        return EXIT_FAILURE;
    }

    int str_spisze = strlen(word);
    

    fprintf(fp, "%03i  %06zu  %s %*u             %u",score, outp->plid, word,SPACES-str_spisze ,n_succ, g_serv_game->trials);

    if (flock(fileno(fp), LOCK_UN) == -1) {
        perror("Flock.\n");
        return EXIT_FAILURE;
    }
    fclose(fp);

    if(filter_scoreboard() == EXIT_FAILURE){
        perror("[ERR] Something Went Wrong");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}

Result get_scoreboard(ScoreEntry* scoreboard_list){
    struct dirent **filelist;
    int n_entries, i = 0;

    const char *dir_path = "sv_data/score";
    if (access(dir_path, F_OK) == 0) {
        printf("Directory %s exists\n", dir_path);
    } else {
        perror(E_SERVER_ERROR);
        return EXIT_FAILURE;
    }

    if(filter_scoreboard() == EXIT_FAILURE){
        perror("[ERR] Something Went Wrong");
        return EXIT_FAILURE;
    }

    DIR* dir = opendir(dir_path);
    FILE* fp;
    char path[PATH_SIZE*2];
    if(dir == NULL){
        perror("There is no File");
        return -1;
    }

    n_entries = scandir("sv_data/score/" , &filelist , 0 , alphasort) ;
    if ( n_entries < 0 ){
        return EXIT_FAILURE;
    }
    else{
        while (n_entries--) {
        // Skip the "." and ".." entries
            if (filelist[n_entries]->d_name[0] == '.') {
                continue;
            }

            sprintf(path, "sv_data/score/%s", filelist[n_entries]->d_name);
            if ((fp = fopen(path, "r")) == NULL) {
                perror("[ERR] Error opening file");
                return EXIT_FAILURE;
            }
            else {
                if (flock(fileno(fp), LOCK_SH) == -1) {
                    perror("Flock.\n");
                    return EXIT_FAILURE;
                }

                if(fgets(scoreboard_list[i].score_str, sizeof(scoreboard_list[i].score_str), fp) == NULL){
                    perror("[ERR] Error reading the score file");
                    return EXIT_FAILURE;
                };
            
                if (flock(fileno(fp), LOCK_UN) == -1) {
                    perror("Flock.\n");
                    return EXIT_FAILURE;
                }
                fclose(fp);
                i++;

            }

    }
    }

    return EXIT_SUCCESS;

}