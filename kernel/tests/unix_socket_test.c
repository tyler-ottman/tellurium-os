#include <arch/scheduler.h>
#include <arch/terminal.h>
#include <sys/misc.h>
#include <sys/unix_socket.h>
#include <tests/test.h>

static volatile bool server_init = false;

void unix_socket_server(void *param) {
    (void) param;

    struct sockaddr_un addr;
    socklen_t addrlen = sizeof(addr);

    socket_t *sock;
    int err = unix_socket_create(&sock, SOCK_STREAM, AF_UNIX);
    ASSERT(err == SKT_OK, err, "unix_socket_server: init failure");

    const char sun_path[] = "/tmp/socket_gui";
    addr.sun_family = AF_UNIX;
    __memcpy(addr.sun_path, sun_path, __strlen(sun_path));
    err = sock->socket_bind(sock, (const struct sockaddr *)&addr, sizeof(addr));
    ASSERT(err == SKT_OK, err, "unix_socket_server: bind failure");

    err = sock->socket_listen(sock, 10);
    ASSERT(err == SKT_OK, err, "unix_socket_server: listen failure");

    server_init = true;

    socket_t *client;
    err = sock->socket_accept(sock, &client, (struct sockaddr *)&addr, &addrlen, 0);
    ASSERT(err == SKT_OK, err, "socket connection failure");
}

void unix_socket_client(void *param) {
    (void)param;
    struct sockaddr_un addr;

    while (!server_init) {}

    socket_t *sock;
    int err = unix_socket_create(&sock, SOCK_STREAM, AF_UNIX);
    ASSERT(err == SKT_OK, err, "unix_socket_client: init failure");

    const char sun_path[] = "/tmp/socket_gui";
    addr.sun_family = AF_UNIX;
    __memcpy(addr.sun_path, sun_path, __strlen(sun_path));
    err = sock->socket_connect(sock, (const struct sockaddr *)&addr, sizeof(addr));
    ASSERT(err == SKT_OK, err, "unix_socket_client: connect failure");
}

void unix_socket_test(void) {
    void *server_entry = (void *)((uintptr_t)unix_socket_server);
    void *client_entry = (void *)((uintptr_t)unix_socket_client);

    thread_t *server = create_kernel_thread(server_entry, NULL);
    thread_t *client = create_kernel_thread(client_entry, NULL);

    schedule_add_thread(server);
    schedule_add_thread(client);
}