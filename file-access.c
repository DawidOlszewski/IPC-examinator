#include "file-access.h"

void path_from_id(char *path, int id)
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
    Question* parsed_question = (Question *)mmap(NULL, shared_memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_fd, 0);

    // Close shared memory fd
    close(shared_memory_fd);

    return parsed_question;
}

void parse_question_from_file(char *path, Question *parsed_question, int question_id)
{
    FILE *ptr = fopen(path, "r");
    if (ptr == NULL)
    {
        printf("No such file");
        exit(EXIT_FAILURE);
    }

    // get question content
    char buf[BUFFER_SIZE];
    fgets(buf, BUFFER_SIZE, ptr);
    strcpy(parsed_question->question_content, buf);

    // get all answers
    for (int i = 0; i < 4; i++)
    {
        fgets(buf, BUFFER_SIZE, ptr);
        parsed_question->answers[i].identifier = buf[0];

        fgets(buf, BUFFER_SIZE, ptr);
        strcpy(parsed_question->answers[i].answer_content, buf);
    }

    // get correct answer index
    fgets(buf, BUFFER_SIZE, ptr);
    parsed_question->correct_answer = atoi(buf);
    
    parsed_question->id = question_id;
    free(ptr);
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
    int status;

    if (childPID == -1)
    {
        perror("Error while forking");
        exit(EXIT_FAILURE);
    }
    if (childPID == 0)
    {

        char *path = (char *)malloc(sizeof(char) * PATH_LENGTH);
        path_from_id(path, question_id);

        parse_question_from_file(path, parsed_question, question_id);

        exit(0);
    }
    else
    {
        // wait until the child process finished
        wait(&status);

        // detach the shared memory segment
        shm_unlink(name);
        return parsed_question;
    }
}
