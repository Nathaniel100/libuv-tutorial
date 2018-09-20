
#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#define DEFAULT_PORT 12346

void on_close(uv_handle_t *handle) {
  free(handle);
}

void on_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = (char *)malloc(suggested_size);
  buf->len = suggested_size;
}

void on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
  if (nread < 0) {
    if (nread != UV_EOF) {
      fprintf(stderr, "read failed: %s", uv_strerror(nread));
    }
  } else if (nread > 0) {
    printf("%.*s\n", (int)nread, buf->base);
  }

  free(buf->base);
  uv_close((uv_handle_t *)client, on_close);
}

void on_connection(uv_connect_t *connection, int status) {
  if (status != 0) {
    fprintf(stderr, "connect failed: %s\n", uv_strerror(status));
    return;
  }
  uv_read_start(connection->handle, on_alloc, on_read);
}

int main() {
  uv_loop_t *loop = (uv_loop_t *)malloc(sizeof(uv_loop_t));
  uv_connect_t *connection = (uv_connect_t *)malloc(sizeof(uv_connect_t));
  uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
  struct sockaddr_in addr;

  uv_loop_init(loop);
  uv_tcp_init(loop, client);
  uv_ip4_addr("127.0.0.1", DEFAULT_PORT, &addr);

  uv_tcp_connect(connection, client, (const struct sockaddr *)&addr,
                 on_connection);

  uv_run(loop, UV_RUN_DEFAULT);

  free(connection);
  uv_loop_close(loop);
  free(loop);
  return 0;
}