#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#include <sys/wait.h>

#define SOCKET_NAME "/tmp/DemoSocket"
#define BUFFER_SIZE 128

#define PATH_LENGTH 64
#define QUESTION_PATH "./questions/"

#define MAX_CLIENT_SUPPORTED 32

#define max(x, y) x > y ? x : y

typedef struct Answer
{
    char identifier;
    char answer_content[BUFFER_SIZE];
} Answer;

typedef struct Question
{
    int id;
    char question_content[BUFFER_SIZE];
    Answer answers[4];
    int correct_answer;
} Question;

/*An array of File descriptors which the server process
 * is maintaining in order to talk with the connected clients.
 * Master skt FD is also a member of this array*/
int monitored_fd_set[MAX_CLIENT_SUPPORTED];

/* Each connected client's intermediate result is
 * maintained in this client array. */

// Remove all the FDs, if any, from the the array
static void intitiaze_monitor_fd_set()
{
    for (int i = 0; i < MAX_CLIENT_SUPPORTED; i++)
        monitored_fd_set[i] = -1;
}

// Add a new FD to the monitored_fd_set array
static void add_to_monitored_fd_set(int skt_fd)
{
    for (int i = 0; i < MAX_CLIENT_SUPPORTED; i++)
    {

        if (monitored_fd_set[i] != -1)
            continue;
        monitored_fd_set[i] = skt_fd;
        break;
    }
}

// Remove the FD from monitored_fd_set array
static void
remove_from_monitored_fd_set(int skt_fd)
{

    for (int i = 0; i < MAX_CLIENT_SUPPORTED; i++)
    {

        if (monitored_fd_set[i] != skt_fd)
            continue;

        monitored_fd_set[i] = -1;
        break;
    }
}

// Copy monitored sockets to fd_set
static void
refresh_fd_set(fd_set *fd_set_ptr)
{

    FD_ZERO(fd_set_ptr);
    int i = 0;
    for (; i < MAX_CLIENT_SUPPORTED; i++)
    {
        if (monitored_fd_set[i] != -1)
        {
            FD_SET(monitored_fd_set[i], fd_set_ptr);
        }
    }
}

int check(int response, char *message)
{
    if (response == -1)
    {
        perror(message);
        exit(EXIT_FAILURE);
    }
    return response;
}

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

int setup_server()
{
    struct sockaddr_un name; // Socket type and socket name
    int connection_socket;
    // In case some process is already listening on our socket name
    unlink(SOCKET_NAME);

    // create master file descriptor in stream type connection
    connection_socket = check(
        socket(AF_UNIX, SOCK_STREAM, 0),
        "creating connection socket failed");

    // Initializing connection socket struct
    memset(&name, 0, sizeof(struct sockaddr_un));

    // Providing socket family and socket name
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);

    // Bind socket to socket name, every request sent to socket name will be redirected to our server
    check(
        bind(connection_socket, (const struct sockaddr *)&name, sizeof(struct sockaddr_un)),
        "bind failed");

    check(
        listen(connection_socket, MAX_CLIENT_SUPPORTED),
        "listen failed");

    return connection_socket;
}

int add_client(int connection_socket)
{
    // Master socket call, new client connects
    printf("New connection recieved, accept the connection\n");

    int data_socket = check(accept(connection_socket, NULL, NULL),
                            "accept failed");

    printf("Connection accepted from client\n");

    add_to_monitored_fd_set(data_socket);

    return data_socket;
}

int get_ready_fd(fd_set *readfds, int max_fd)
{
    for (int i = 0; i < max_fd; i++)
        if (FD_ISSET(i, readfds))
            return i;
}

int main(int argc, char *argv[])
{
    int data_socket;
    int result;
    int data;
    int ret;
    char buffer[BUFFER_SIZE];
    fd_set readfds;
    intitiaze_monitor_fd_set();

    int connection_socket = setup_server();
    int max_fd = connection_socket;

    printf("Server is set up\n");

    add_to_monitored_fd_set(connection_socket);
    Question *q = get_question(1);
    printf(q->question_content);
    printf("%d", q->correct_answer);
    for (int i = 0; i < 4; i++)
    {
        printf(q->answers[i].answer_content);
    }
    while (1)
    {

        // Copy the entire monitored FDs to readfds
        refresh_fd_set(&readfds);
        printf("Waiting for select system call...\n");
        // Blocking system call, waiting for select call
        select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(connection_socket, &readfds))
        {
            // Select call to master fd, new client connection
            data_socket = add_client(connection_socket);

            // Keep track of biggest fd
            max_fd = max(max_fd, data_socket);
        }
        else // Connected client made selected call
        {
            // Find the client which has send us the call
            data_socket = get_ready_fd(&readfds, max_fd);

            // Prepare the buffer to recv the data
            memset(buffer, 0, BUFFER_SIZE);

            // Blocking system call, waiting for data from client
            ret = read(data_socket, buffer, BUFFER_SIZE);

            // Read returns zero if socket disconnects
            if (ret == 0)
            {
                printf("Client disconnected\n");
                remove_from_monitored_fd_set(data_socket);
                continue;
            }

            for (int i = 0; i < max_fd; i++)
            {
                // Send data to every client except master and data sender
                if (monitored_fd_set[i] == -1 || monitored_fd_set[i] == connection_socket || monitored_fd_set[i] == data_socket)
                    continue;

                check(
                    write(monitored_fd_set[i], buffer, BUFFER_SIZE),
                    "write to recv failed");
            }
        }
    }

    // Close the connection socket
    close(connection_socket);

    FD_ZERO(&readfds);
    printf("Connection closed..\n");

    // Unlink the socket
    unlink(SOCKET_NAME);
    exit(EXIT_SUCCESS);
}