#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

#define BACKLOG 3
volatile sig_atomic_t do_work = 1;

void sigint_handler(int sig)
{
        do_work = 0;
}

int sethandler(void (*f)(int), int sigNo)
{
        struct sigaction act;
        memset(&act, 0, sizeof(struct sigaction));
        act.sa_handler = f;
        if (-1 == sigaction(sigNo, &act, NULL))
                return -1;
        return 0;
}

int make_socket(int domain, int type)
{
        int sock;
        sock = socket(domain, type, 0);
        if (sock < 0)
                ERR("socket");
        return sock;
}

int bind_tcp_socket(uint16_t port)
{
        struct sockaddr_in addr;
        int socketfd, t = 1;
        socketfd = make_socket(PF_INET, SOCK_STREAM);
        memset(&addr, 0, sizeof(struct sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(t)))
                ERR("setsockopt");
        if (bind(socketfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
                ERR("bind");
        if (listen(socketfd, BACKLOG) < 0)
                ERR("listen");
        return socketfd;
}

int add_new_client(int sfd)
{
        int nfd;
        if ((nfd = TEMP_FAILURE_RETRY(accept(sfd, NULL, NULL))) < 0) {
                if (EAGAIN == errno || EWOULDBLOCK == errno)
                        return -1;
                ERR("accept");
        }
        return nfd;
}

void usage(char *name)
{
        fprintf(stderr, "USAGE: %s socket port\n", name);
}

ssize_t bulk_read(int fd, char *buf, size_t count)
{
        int c;
        size_t len = 0;
        do {
                c = TEMP_FAILURE_RETRY(read(fd, buf, count));
                if (c < 0)
                        return c;
                if (0 == c)
                        return len;
                buf += c;
                len += c;
                count -= c;
        } while (count > 0);
        return len;
}

// ssize_t my_read(int fd, char *addr_send, char *addr_rec, char *buf)
// {
//      int c;
//      size_t len = 0;

//     // char addr_send, addr_rec, tmp;
//     char tmp;

//     c = TEMP_FAILURE_RETRY(read(fd, addr_send, 1));
//     if (c < 0)
//         return c;
//     if (0 == c)
//         return len;

//     c = TEMP_FAILURE_RETRY(read(fd, addr_rec, 1));
//     if (c < 0)
//         return c;
//     if (0 == c)
//         return len;

//      do {
//              c = TEMP_FAILURE_RETRY(read(fd, &tmp, 1));
//              if (c < 0)
//                      return c;
//              if (0 == c)
//                      return len;
//              buf++;
//              len++;
//      } while (tmp != '$');
//      return len;
// }

ssize_t bulk_write(int fd, char *buf, size_t count)
{
        int c;
        size_t len = 0;
        do {
                c = TEMP_FAILURE_RETRY(write(fd, buf, count));
                if (c < 0)
                        return c;
                buf += c;
                len += c;
                count -= c;
        } while (count > 0);
        return len;
}

// void communicate(int cfd)
// {
//      ssize_t size;
//      int32_t data[5];
//      if ((size = bulk_read(cfd, (char *)data, sizeof(int32_t[5]))) < 0)
//              ERR("read:");
//      if (size == (int)sizeof(int32_t[5])) {
//              calculate(data);
//              if (bulk_write(cfd, (char *)data, sizeof(int32_t[5])) < 0 && errno != EPIPE)
//                      ERR("write:");
//      }
//      if (TEMP_FAILURE_RETRY(close(cfd)) < 0)
//              ERR("close");
// }

void get_host_no(int cur_cfd, int cfd[10])
{
        ssize_t size;
        char data;

        if ((size = bulk_read(cur_cfd, &data, sizeof(char))) < 0)
                ERR("read:");
    fprintf(stderr, "%s", "get_host_no\n");
        if (size == (int)sizeof(char)) {
        fprintf(stderr, "%c", data);
        fprintf(stderr, "%s", "\n");
                // calculate(data);

        if((int)data - '0' < 1 || (int)data - '0' > 8)
        {
            char *tmp_buf = "NOT A VALID ADDRESS!!!";
            if (bulk_write(cur_cfd, tmp_buf, 22*sizeof(char)) < 0 && errno != EPIPE)
                            ERR("write:");
            fprintf(stderr, "NOT A VALID ADDRESS!!!\n");
            return;
        }

        if( cfd[(int)data - '0'] == -1) {
            cfd[(int)data - '0'] = cur_cfd;
            fprintf(stderr, "address %c given successfully\n", data);
            if (bulk_write(cur_cfd, &data, sizeof(char)) < 0 && errno != EPIPE)
                            ERR("write:");
        }
        else {
            fprintf(stderr, "address %c already in use!!!\n", data);
            data = 'w';
            if (bulk_write(cur_cfd, &data, sizeof(char)) < 0 && errno != EPIPE)
                            ERR("write:");
        }

        }
        // if (TEMP_FAILURE_RETRY(close(cur_cfd)) < 0)
        //      ERR("close");
}

void doServer(int fdT)
{
        int cfd[10], cur_cfd;
    for(int i = 0; i < 10; i++) cfd[i] = -1;

    // char buf[576];
        fd_set base_rfds, rfds;
        sigset_t mask, oldmask;
        FD_ZERO(&base_rfds);
        FD_SET(fdT, &base_rfds);
        sigemptyset(&mask);
        sigaddset(&mask, SIGINT);
        sigprocmask(SIG_BLOCK, &mask, &oldmask);
        while (do_work) {
                rfds = base_rfds;
                if (pselect(fdT + 1, &rfds, NULL, NULL, NULL, &oldmask) > 0) {
            // cfd[0] = add_new_client(fdT);
            fprintf(stderr, "%s", "hello\n");
            cur_cfd = add_new_client(fdT);
                        if (cur_cfd >= 0)
                                get_host_no(cur_cfd, cfd);
                } else {
                        if (EINTR == errno)
                                continue;
                        ERR("pselect");
                }
        }
        sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

int main(int argc, char **argv)
{
        int fdT;
        int new_flags;
        if (argc != 3) {
                usage(argv[0]);
                return EXIT_FAILURE;
        }
        if (sethandler(SIG_IGN, SIGPIPE))
                ERR("Seting SIGPIPE:");
        if (sethandler(sigint_handler, SIGINT))
                ERR("Seting SIGINT:");
        fdT = bind_tcp_socket(atoi(argv[2]));
        new_flags = fcntl(fdT, F_GETFL) | O_NONBLOCK;
        fcntl(fdT, F_SETFL, new_flags);
        doServer(fdT);
        if (unlink(argv[1]) < 0)
                ERR("unlink");
        if (TEMP_FAILURE_RETRY(close(fdT)) < 0)
                ERR("close");
        fprintf(stderr, "Server has terminated.\n");
        return EXIT_SUCCESS;
}