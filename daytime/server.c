
#include <stdlib.h>
#include <time.h>
#include <uv.h>

#define DEFAULT_PORT 12346
#define DEFAULT_BACKLOG 128

typedef struct daytime_server {
  uv_loop_t *loop;
  uv_tcp_t *tcp;
} daytime_server_t;

typedef struct write_req {
  uv_write_t req;
  uv_buf_t buf;
} write_req_t;

char *make_daytime_string() {
  time_t t = time(NULL);
  struct tm local_tm;
  char *buffer = (char *)malloc(32);
  memset(buffer, 0, 32);
  return asctime_r(localtime_r(&t, &local_tm), buffer);
}

void free_write_req(write_req_t *wr) {
  free(wr->buf.base);
  free(wr);
}

void on_client_close(uv_handle_t *handle) {
  free(handle);
}

void on_write(uv_write_t *req, int status) {
  if (status != 0) {
    fprintf(stderr, "write failed: %s\n", uv_strerror(status));
  }
  uv_close((uv_handle_t *)req->handle, on_client_close);
  free_write_req((write_req_t *)req);
}

void on_new_connection(uv_stream_t *server_tcp, int status) {
  if (status != 0) {
    fprintf(stderr, "on_new_connection failed: %s\n", uv_strerror(status));
    return;
  }
  daytime_server_t *server = (daytime_server_t *) server_tcp->data;
  uv_tcp_t *client_tcp = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(server_tcp->loop, client_tcp);
  if (uv_accept(server_tcp, (uv_stream_t *)client_tcp) == 0) {
    char *message = make_daytime_string();
    write_req_t *wr = (write_req_t *)malloc(sizeof(write_req_t));
    wr->buf = uv_buf_init(message, strlen(message));
    uv_write((uv_write_t *)wr, (uv_stream_t *)client_tcp, &wr->buf, 1, on_write);
  } else {
    uv_close((uv_handle_t *)client_tcp, on_client_close);
  }
}

daytime_server_t *daytime_server_create() {
  daytime_server_t *server = (daytime_server_t *)malloc(sizeof(daytime_server_t));
  server->loop = (uv_loop_t *)malloc(sizeof(uv_loop_t));
  uv_loop_init(server->loop);
  return server;
}

void daytime_server_destroy(daytime_server_t *server) {
  uv_loop_close(server->loop);
  free(server->loop);
  if (server->tcp) {
    server->tcp->data = NULL;
    free(server->tcp);
  }
  free(server);
}

void daytime_server_bind(daytime_server_t *server, const char *ip, int port) {
  server->tcp = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(server->loop, server->tcp);
  server->tcp->data = server;

  struct sockaddr_in addr;
  uv_ip4_addr(ip, port, &addr);
  uv_tcp_bind(server->tcp, (struct sockaddr *)&addr, 0);
}

void daytime_server_start(daytime_server_t *server) {
  int r = uv_listen((uv_stream_t *)server->tcp, DEFAULT_BACKLOG,
                    on_new_connection);
  if (r != 0) {
    fprintf(stderr, "uv_listen failed: %s\n", uv_strerror(r));
    return;
  }
  uv_run(server->loop, UV_RUN_DEFAULT);
}

int main() {
  daytime_server_t *server = daytime_server_create();
  daytime_server_bind(server, "0.0.0.0", DEFAULT_PORT);
  daytime_server_start(server);
  daytime_server_destroy(server);
  return 0;
}