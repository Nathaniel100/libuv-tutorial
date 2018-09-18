
#include <stdlib.h>
#include <uv.h>

#define DEFAULT_SERVER "127.0.0.1"
#define DEFAULT_PORT 12345

void on_write(uv_write_t *req, int status);

void echo(uv_stream_t *tcp) {
  char buffer[100];
  uv_buf_t buf = uv_buf_init(buffer, sizeof(buffer));
  // TODO free write_req
  uv_write_t *write_req = (uv_write_t *)malloc(sizeof(uv_write_t));

  memset(buffer, 0, sizeof(buffer));
  fgets(buffer, sizeof(buffer), stdin);
  buf.len = strlen(buffer);
  uv_write(write_req, tcp, &buf, 1, on_write);
}

void on_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = (char *)malloc(suggested_size);
  buf->len = suggested_size;
}

void on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
  if (nread < 0) {
    fprintf(stderr, "read error");
    return;
  }
  printf("result: %s\n", buf->base);
  if (buf->base) {
    free(buf->base);
  }
  echo(client);
}

void on_write(uv_write_t *req, int status) {
  if (status != 0) {
    fprintf(stderr, "write error %s\n", uv_strerror(status));
    return;
  }
  uv_read_start(req->handle, on_alloc, on_read);
}


void on_connection(uv_connect_t *connection, int status) {
  if (status != 0) {
    fprintf(stderr, "connect error %s\n", uv_strerror(status));
    return;
  }
  echo((uv_stream_t *)connection->handle);
}

int main() {
  uv_loop_t *loop = uv_default_loop();
  uv_tcp_t socket;
  struct sockaddr_in addr;
  // TODO free
  uv_connect_t *connection = (uv_connect_t *)malloc(sizeof(uv_connect_t));

  uv_tcp_init(loop, &socket);
  uv_ip4_addr(DEFAULT_SERVER, DEFAULT_PORT, &addr);

  uv_tcp_connect(connection, &socket, (const struct sockaddr *)&addr,
                 on_connection);
  return uv_run(loop, UV_RUN_DEFAULT);
}