#include "file-access.h"

void create_path_from_id(char *path, int id)
{
    // Convert id to string
    int num_length = (int)((ceil(log10(id)) + 1));
    char id_repr[num_length];
    sprintf(id_repr, "%d", id);

    strcat(path, QUESTION_PATH);
    strcat(path, id_repr);
    strcat(path, ".txt");
}

Question *allocate_shared_memory(const char *name)
{
    int shared_memory_fd;
    int shared_memory_size;

    shared_memory_size = sizeof(Question);

    // In case of unexpected error in parser clear shared memory
    shm_unlink(name);

    shared_memory_fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, S_IRWXU | S_IRWXG);
    if (shared_memory_fd < 0)
    {
        perror("Error allocating shared memory");
        exit(EXIT_FAILURE);
    }

    printf("Created shared memory %s\n", name);

    // Expand shared memory to shared_memory_size
    ftruncate(shared_memory_fd, shared_memory_size);

    // Map parsed question to shared memory
    Question *parsed_question = (Question *)mmap(NULL, shared_memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_fd, 0);

    // Close shared memory fd
    close(shared_memory_fd);

    return parsed_question;
}

void parse_question_from_file(char *path, Question *parsed_question, int question_id)
{
    FILE *file_ptr = fopen(path, "r");
    if (file_ptr == NULL)
    {
        printf("No such file");
        exit(EXIT_FAILURE);
    }

    // get question content
    char buf[BUFFER_SIZE];
    fgets(buf, BUFFER_SIZE, file_ptr);
    strcpy(parsed_question->question_content, buf);

    // get all answers
    for (int i = 0; i < 4; i++)
    {
        fgets(buf, BUFFER_SIZE, file_ptr);
        parsed_question->answers[i].identifier = buf[0];

        fgets(buf, BUFFER_SIZE, file_ptr);
        strcpy(parsed_question->answers[i].answer_content, buf);
    }

    // get correct answer index
    fgets(buf, BUFFER_SIZE, file_ptr);
    parsed_question->correct_answer = atoi(buf);

    parsed_question->id = question_id;
    free(file_ptr);
}

Question *get_question(int question_id)
{
    const char *name = "QUESTION_OBJECT";

    Question *parsed_question = allocate_shared_memory(name);

    if (parsed_question == NULL)
    {
        perror("Error while mapping");
        exit(EXIT_FAILURE);
    }

    pid_t childPID = fork();

    if (childPID == -1)
    {
        perror("Error while forking");
        exit(EXIT_FAILURE);
    }
    if (childPID == 0)
    {

        char *path = (char *)calloc(PATH_LENGTH, sizeof(char));
        create_path_from_id(path, question_id);

        parse_question_from_file(path, parsed_question, question_id);
        free(path);
        exit(EXIT_SUCCESS);
    }
    else
    {
        // wait until the child process finished
        wait(NULL);

        // detach the shared memory segment
        shm_unlink(name);
        return parsed_question;
    }
}

void save_final_scoreboard()
{
    pid_t childPID = fork();

    if (childPID == -1)
    {
        perror("Error while forking");
        exit(EXIT_FAILURE);
    }
    if (childPID == 0)
    {
        FILE *file_ptr = fopen(RESULTS_FILE_PATH, "w");
        if (file_ptr == NULL)
        {
            printf("No such file");
            exit(EXIT_FAILURE);
        }
        char formatted_time[DATE_FORMAT_LENGTH];
        get_iso_time(formatted_time);
        fprintf(file_ptr, "Game finished on %s\nFinal score: \n%s", formatted_time, scoreBoard);
        fclose(file_ptr);
        exit(0);
    }
    else
    {
        // wait until the child process finished
        wait(NULL);

        return;
    }
}

int count_question_files()
{

    int file_count = 0;
    DIR *dirp;
    struct dirent *entry;

    dirp = opendir(QUESTION_PATH); 
    if(dirp == NULL) {
        perror("Question directory not found!");
        exit(EXIT_FAILURE);
    }
    while ((entry = readdir(dirp)) != NULL){
        if (entry->d_type == DT_REG){ 
            file_count++;
        }
    }
    closedir(dirp);
    return file_count;
}

