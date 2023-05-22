#include <arch/scheduler.h>
#include <arch/terminal.h>
#include <sys/misc.h>
#include <sys/unix_socket.h>
#include <tests/test.h>

#define UNIX_BUF_LEN                        128

static volatile bool server_init = false;

void unix_socket_server(void *param) {
    (void) param;

    struct sockaddr_un addr;
    socklen_t addrlen = sizeof(addr);

    socket_t *sock;
    int err = unix_socket_create(&sock, SOCK_STREAM, AF_UNIX);
    ASSERT(err == SKT_OK, err, "usocket_server: init failure");

    const char sun_path[] = "/tmp/socket_gui";
    addr.sun_family = AF_UNIX;
    __memcpy(addr.sun_path, sun_path, __strlen(sun_path));
    err = sock->socket_bind(sock, (const struct sockaddr *)&addr, sizeof(addr));
    ASSERT(err == SKT_OK, err, "usocket_server: bind failure");

    err = sock->socket_listen(sock, 10);
    ASSERT(err == SKT_OK, err, "usocket_server: listen failure");

    server_init = true;

    socket_t *receive;
    err = sock->socket_accept(sock, &receive, (struct sockaddr *)&addr, &addrlen, 0);
    ASSERT(err == SKT_OK, err, "usocket_server: connection failure");

    char buff[UNIX_BUF_LEN] = {0};
    int bytes_received = 0;
    err = sock->socket_recv(receive, buff, UNIX_BUF_LEN, 0, 0, &bytes_received);
    ASSERT(err == SKT_OK, err, "usocket_server: receive failure");

    kprintf(INFO "usocket_server: %d bytes: %s\n", bytes_received, buff);

    const char msg1[] = "syn+ack";
    int bytes_sent;
    err = sock->socket_send(receive, msg1, __strlen(msg1), 0, 0, &bytes_sent);
    ASSERT(err == SKT_OK, err, "usocket_server: send failure");
}

void unix_socket_client(void *param) {
    (void)param;
    struct sockaddr_un addr;

    while (!server_init) {}

    socket_t *sock;
    int err = unix_socket_create(&sock, SOCK_STREAM, AF_UNIX);
    ASSERT(err == SKT_OK, err, "usocket_client: init failure");

    const char sun_path[] = "/tmp/socket_gui";
    addr.sun_family = AF_UNIX;
    __memcpy(addr.sun_path, sun_path, __strlen(sun_path));
    err = sock->socket_connect(sock, (const struct sockaddr *)&addr, sizeof(addr));
    ASSERT(err == SKT_OK, err, "usocket_client: connect failure");

    const char msg1[] = "syn";
    int bytes_sent;
    err = sock->socket_send(sock, msg1, __strlen(msg1), 0, 0, &bytes_sent);
    ASSERT(err == SKT_OK, err, "usocket_client: send failure");

    char buff[UNIX_BUF_LEN] = {0};
    int bytes_received;
    err = sock->socket_recv(sock, buff, UNIX_BUF_LEN, 0, 0, &bytes_received);
    ASSERT(err == SKT_OK, err, "usocket_client: receive failure");

    kprintf(INFO "usocket_client: %d bytes: %s\n", bytes_received, buff);
}

void unix_socket_test(void) {
    void *server_entry = (void *)((uintptr_t)unix_socket_server);
    void *client_entry = (void *)((uintptr_t)unix_socket_client);

    thread_t *server = create_kernel_thread(server_entry, NULL);
    thread_t *client = create_kernel_thread(client_entry, NULL);

    schedule_add_thread(server);
    schedule_add_thread(client);
}