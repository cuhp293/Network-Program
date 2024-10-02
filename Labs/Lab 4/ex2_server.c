#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024

typedef struct {
    char question[256];
    char ans[4][64];
    int correct_ans;
} Quiz;

// Q&A
Quiz quizz[10] = {
    {"Q1. What is the capital of Japan?", {"Berlin", "Beijing", "Tokyo", "Kyoto"}, 2},
    {"Q2. 10 / 2 = ?", {"5", "6", "7", "8"}, 0},
    {"Q3. Who wrote Romeo and Juliet?", {"Charles Dickens", "William Shakespeare", "Mark Twain", "Jane Austen"}, 1},
    {"Q4. 8 * 2 = ?", {"14", "15", "16", "17"}, 2},
    {"Q5. What is the chemical symbol for water?", {"O2", "H2O", "CO2", "NaCl"}, 1},
    {"Q6. 2 + 2 = ?", {"1", "2", "3", "4"}, 3},
    {"Q7. Which country is known as the Land of the Rising Sun?", {"China", "Japan", "South Korea", "Thailand"}, 1},
    {"Q8. What is the smallest prime number?", {"1", "2", "3", "5"}, 1},
    {"Q9. Who painted the Mona Lisa?", {"Vincent van Gogh", "Pablo Picasso", "Leonardo da Vinci", "Claude Monet"}, 2},
    {"Q10. Which element has the atomic number 1?", {"Helium", "Oxygen", "Hydrogen", "Nitrogen"}, 2}
};

void shuffle_ans(int *index, int size) {
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = index[i];
        index[i] = index[j];
        index[j] = tmp;
    }
}

// Signal handler to prevent zombie processes
void sigchld_handler(int sig) {
    (void)sig; // Ignore unused parameter warning
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_client(int connfd) {
    char buffer[BUFFER_SIZE];
    int n, score = 0;

    // Send questions
    for (int i = 0; i < 10; i++) {
        Quiz curr_quiz = quizz[i];
        int index[4] = {0, 1, 2, 3};
        shuffle_ans(index, 4);

        sprintf(buffer, "%s\n", curr_quiz.question);
        for (int j = 0; j < 4; j++) {
            sprintf(buffer + strlen(buffer), "%d. %s\n", j + 1, curr_quiz.ans[index[j]]);
        }
        send(connfd, buffer, strlen(buffer), 0);

        memset(buffer, 0, BUFFER_SIZE);
        n = recv(connfd, buffer, BUFFER_SIZE, 0);
        if (n <= 0) {
            break;
        }
        
        int cli_ans = atoi(buffer) - 1;
        if (index[cli_ans] == curr_quiz.correct_ans) {
            score++;
        }
    }

    sprintf(buffer, "You get %d points\n", score);
    printf("Score: %d\n", score);
    send(connfd, buffer, strlen(buffer), 0);

    // Close the client socket
    close(connfd);
}

int main() {
    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pid_t pid;

    // Create the listening socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Setup the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the listening socket to the specified port
    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(listenfd, 5) < 0) {
        perror("Listen failed");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // Handle SIGCHLD to prevent zombie processes
    signal(SIGCHLD, sigchld_handler);

    printf("Server is listening on port %d...\n", PORT);

    // Server loop to accept multiple clients
    while (1) {
        // Accept an incoming connection
        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &addr_len);
        if (connfd < 0) {
            perror("Accept failed");
            continue;
        }

        // Fork a child process to handle the client
        pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            close(connfd);
        } else if (pid == 0) {
            // Child process: handle the client
            close(listenfd);  // Close the listening socket in the child process
            handle_client(connfd);
            close(connfd);
            exit(0);
        } else {
            // Parent process: continue accepting new clients
            close(connfd);  // Close the client socket in the parent process
        }
    }

    return 0;
}
