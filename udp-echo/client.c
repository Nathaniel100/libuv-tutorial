
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#define DEFAULT_PORT 12347

typedef struct echo_client {
  uv_loop_t *loop;
  uv_udp_t *udp;
} echo_client_t;

typedef struct send_req {
  uv_udp_send_t req;
  uv_buf_t buf;
} send_req_t;

static void free_send_req(send_req_t *req) {
  free(req->buf.base);
  free(req);
}

void echo(uv_udp_t *udp, const struct sockaddr *dest);

static void on_alloc(uv_handle_t *handle, size_t suggested_size,
                     uv_buf_t *buf) {
  buf->base = (char *)malloc(suggested_size);
  buf->len = suggested_size;
}

static void on_recv(uv_udp_t *udp, ssize_t nread, const uv_buf_t *buf,
                    const struct sockaddr *addr, unsigned flags) {
  if (nread > 0) {                     
    printf("%s\n", buf->base);
    echo(udp, addr);
  } else if (nread < 0) {
    if (nread != UV_EOF) {
      fprintf(stderr, "recv failed: %s\n", uv_strerror(nread));
    }
    uv_close((uv_handle_t *)udp, NULL);
  }
  free(buf->base);
}

static void on_send(uv_udp_send_t *req, int status) {
  if (status != 0) {
    fprintf(stderr, "send failed: %s\n", uv_strerror(status));
  }
  free_send_req((send_req_t *)req);
}

void echo(uv_udp_t *udp, const struct sockaddr *dest) {
  char *buffer = (char *)malloc(1024);
  memset(buffer, 0, 1024);
  fgets(buffer, 1024, stdin);
  send_req_t *sr = (send_req_t *)malloc(sizeof(send_req_t));
  sr->buf = uv_buf_init(buffer, strlen(buffer));
  uv_udp_send((uv_udp_send_t *)sr, udp, &sr->buf, 1, dest, on_send);
  uv_udp_recv_start(udp, on_alloc, on_recv);
}

echo_client_t *echo_client_create() {
  echo_client_t *client = (echo_client_t *)malloc(sizeof(echo_client_t));
  client->loop = (uv_loop_t *)malloc(sizeof(uv_loop_t));
  client->udp = (uv_udp_t *)malloc(sizeof(uv_udp_t));
  uv_loop_init(client->loop);
  uv_udp_init(client->loop, client->udp);
  return client;
}

void echo_client_start(echo_client_t *client, const char *dest_ip,
                       int dest_port) {
  struct sockaddr_in dest;
  uv_ip4_addr(dest_ip, dest_port, &dest);

  echo(client->udp, (struct sockaddr *)&dest);

  uv_run(client->loop, UV_RUN_DEFAULT);
}

void echo_client_destroy(echo_client_t *client) {
  uv_loop_close(client->loop);
  free(client->udp);
  free(client->loop);
  free(client);
}

int main() {
  echo_client_t *client = echo_client_create();
  echo_client_start(client, "127.0.0.1", DEFAULT_PORT);
  echo_client_destroy(client);
  return 0;
}