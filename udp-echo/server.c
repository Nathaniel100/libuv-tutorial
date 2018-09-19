#include <stdlib.h>
#include <uv.h>

#define DEFAULT_PORT 12347

typedef struct echo_server {
  uv_loop_t *loop;
  uv_udp_t *udp;
} echo_server_t;

static echo_server_t *echo_server_create();
static void echo_server_start(echo_server_t *server, const char *ip, int port);
static void echo_server_destroy(echo_server_t *server);

typedef struct send_req {
  uv_udp_send_t req;
  uv_buf_t buf;
} send_req_t;

static void free_send_req(send_req_t *req) {
  free(req->buf.base);
  free(req);
}

static void on_send(uv_udp_send_t *req, int status) {
  if (status != 0) {
    fprintf(stderr, "send failed: %s\n", uv_strerror(status));
  }
  free_send_req((send_req_t *)req);
}

static void on_alloc(uv_handle_t *handle, size_t suggested_size,
                     uv_buf_t *buf) {
  buf->base = (char *)malloc(suggested_size);
  buf->len = suggested_size;
}

static void on_recv(uv_udp_t *udp, ssize_t nread, const uv_buf_t *buf,
                    const struct sockaddr *addr, unsigned flags) {
  if (nread > 0) {
    send_req_t *sr = (send_req_t *)malloc(sizeof(send_req_t));
    sr->buf = uv_buf_init(buf->base, nread);
    uv_udp_send((uv_udp_send_t *)sr, udp, &sr->buf, 1, addr, on_send);
    return;
  } else if (nread < 0) {
    if (nread != UV_EOF) {
      fprintf(stderr, "recv failed: %s\n", uv_strerror(nread));
    }
    uv_close((uv_handle_t *)udp, NULL);
  }

  free(buf->base);
}

static echo_server_t *echo_server_create() {
  echo_server_t *server = (echo_server_t *)malloc(sizeof(echo_server_t));
  server->loop = (uv_loop_t *)malloc(sizeof(uv_loop_t));
  uv_loop_init(server->loop);
  server->udp = (uv_udp_t *)malloc(sizeof(uv_udp_t));
  uv_udp_init(server->loop, server->udp);
  return server;
}

static void echo_server_start(echo_server_t *server, const char *ip, int port) {
  struct sockaddr_in addr;
  uv_ip4_addr(ip, port, &addr);

  uv_udp_bind(server->udp, (struct sockaddr *)&addr, UV_UDP_REUSEADDR);
  uv_udp_recv_start(server->udp, on_alloc, on_recv);

  uv_run(server->loop, UV_RUN_DEFAULT);
}

static void echo_server_destroy(echo_server_t *server) {
  uv_loop_close(server->loop);
  free(server->udp);
  free(server->loop);
  free(server);
}

int main() {
  echo_server_t *server = echo_server_create();
  echo_server_start(server, "0.0.0.0", DEFAULT_PORT);
  echo_server_destroy(server);
  return 0;
}