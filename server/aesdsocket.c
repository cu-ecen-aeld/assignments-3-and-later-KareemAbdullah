#include "aesdsocket.h"

static volatile bool g_keep_running = true;

static void vSocketSigHandler(int signal_number)
{
    if ((SIGINT == signal_number) || (SIGTERM == signal_number))
    {
        g_keep_running = false;
        syslog(LOG_INFO, "Caught signal, exiting");
    }
}

void vLogFailAndExit(char *error_string, int retval_error)
{
    fprintf(stderr, "%s: %s\n", error_string, strerror(errno));
    closelog();
    exit(retval_error);
}

int main(int argc, char **argv)
{
    int server_fd = -1, client_fd = -1;
    struct sockaddr client_address;
    struct addrinfo hints;
    struct addrinfo *servinfo = NULL;
    char client_ip[INET6_ADDRSTRLEN];
    char buffer[MAX_BUF] = {0};
    FILE *file;
    socklen_t client_len = sizeof(client_address);

    openlog("aesdsocket", LOG_PID, LOG_USER);
    syslog(LOG_INFO, "Starting server");

    if (argc > 1 && strcmp(argv[1], "-d") == 0)
    {
        vDaemonize();
    }

    struct sigaction sigaction_l;

    memset(&sigaction_l, 0U, sizeof(struct sigaction));
    sigaction_l.sa_handler = vSocketSigHandler;

    if (0U != sigaction(SIGINT, &sigaction_l, NULL))
    {
        vLogFailAndExit("ERROR registering for SIGINT", EXIT_FAILURE);
    }

    if (0U != sigaction(SIGTERM, &sigaction_l, NULL))
    {
        vLogFailAndExit("ERROR registering for SIGTERM", EXIT_FAILURE);
    }
    // open socket
    if (0U == (server_fd = socket(PF_INET, SOCK_STREAM, 0)))
    {
        vLogFailAndExit("socket failed", -1);
    }
    // get address info
    memset(&hints, 0U, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;

    if (0u != getaddrinfo(NULL, PORT, &hints, &servinfo))
    {
        vLogFailAndExit("failed getting server address", -1);
    }
    //! bind
    if (0U != bind(server_fd, servinfo->ai_addr, sizeof(struct sockaddr)))
    {
        vLogFailAndExit("bind failed", -1);
    }
    //! listen to connection
    if (0U != (listen(server_fd, SERVER_BACK_LOG)))
    {
        vLogFailAndExit("listen failed", -1);
    }

    while (g_keep_running)
    {
        if ((client_fd = accept(server_fd, &client_address, &client_len)) < 0)
        {
            if (g_keep_running)
                vLogFailAndExit("accept failed", -1);
        }
        inet_ntop(client_address.sa_family, pvGetClientAddr(&client_address), client_ip, sizeof(client_ip));
        syslog(LOG_INFO, "Accepted connection from %s", client_ip);
        file = fopen(FILE_WRITE_PATH, "ab+");
        if (file < 0)
        {
            vLogFailAndExit("Failed to open file", -1);
        }

        int bytes_received;
        while ((bytes_received = recv(client_fd, buffer, MAX_BUF, 0)) > 0)
        {
            for (int i = 0; i < bytes_received; i++)
            {
                if (buffer[i] == '\n')
                {
                    fputs("\n", file);
                }
                else
                {
                    fputc(buffer[i], file);
                }
            }
            if (bytes_received < sizeof(buffer))
                break; // Received a complete packet
        }

        fclose(file);

        FILE *response = fopen(FILE_WRITE_PATH, "r");
        if (response == NULL)
        {
            vLogFailAndExit("Failed to open file for response", -1);
        }

        fseek(response, 0, SEEK_END);
        long file_size = ftell(response);
        fseek(response, 0, SEEK_SET);

        char *response_data = malloc(file_size);
        if (response_data == NULL)
        {
            fclose(response);
            vLogFailAndExit("Failed to allocate memory for response", -1);
        }

        fread(response_data, 1, file_size, response);
        fclose(response);

        send(client_fd, response_data, file_size, 0);
        free(response_data);

        syslog(LOG_INFO, "Closed connection from %s", client_ip);
    }

    if (client_fd != -1)
        close(client_fd);
    if (server_fd != -1)
        close(server_fd);
    // free before closing
    if (servinfo)
        freeaddrinfo(servinfo);
    remove(FILE_WRITE_PATH);
    closelog();

    return EXIT_SUCCESS;
}

void *pvGetClientAddr(struct sockaddr *client_addr)
{
    if (client_addr->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)client_addr)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)client_addr)->sin6_addr);
}

void vDaemonize()
{
    pid_t pid, sid;
    pid = fork();
    if (pid < 0)
    {
        vLogFailAndExit("Failed to daemonize", -1);
    }
    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }
    sid = setsid();

    if (sid < 0)
    {
        vLogFailAndExit("Failed to daemonize", -1);
    }
    if ((chdir("/")) < 0)
    {
        vLogFailAndExit("Failed to daemonize", -1);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    if (-1 == open("/dev/null", O_RDWR, 0666))
    {
        vLogFailAndExit("failed to redirect to /dev/null", -1);
    }
    dup(0);
    dup(0);
}
