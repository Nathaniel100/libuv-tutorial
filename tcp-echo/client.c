
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#define DEFAULT_SERVER "127.0.0.1"
#define DEFAULT_PORT 12345
#define DEFAULT_BUFFER_SIZE 1024

typedef struct write_req {
  uv_write_t req;
  uv_buf_t buf;
} write_req_t;

void free_write_req(uv_write_t *req) {
  write_req_t *wr = (write_req_t *)req;
  free(wr->buf.base);
  free(wr);
}

void on_write(uv_write_t *req, int status);
void on_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
void on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);

void echo(uv_stream_t *tcp) {
  char *buffer = (char *)malloc(DEFAULT_BUFFER_SIZE);
  memset(buffer, 0, DEFAULT_BUFFER_SIZE);
  write_req_t *req = (write_req_t *)malloc(sizeof(write_req_t));
  req->buf = uv_buf_init(buffer, DEFAULT_BUFFER_SIZE);

  fgets(buffer, DEFAULT_BUFFER_SIZE, stdin);
  req->buf.len = strlen(buffer);
  uv_write((uv_write_t *)req, tcp, &req->buf, 1, on_write);
  uv_read_start((uv_stream_t *)tcp, on_alloc, on_read);
}

void on_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = (char *)malloc(suggested_size);
  buf->len = suggested_size;
}

void on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
  if (nread > 0) {
    printf("result: %s\n", buf->base);
    echo(client);
  }
  if (nread < 0) {
    if (nread != UV_EOF) {
      fprintf(stderr, "read failed: %s\n", uv_strerror(nread));
    }
    uv_close((uv_handle_t *)client, NULL);
  }
  free(buf->base);
}

void on_write(uv_write_t *req, int status) {
  if (status != 0) {
    fprintf(stderr, "write error %s\n", uv_strerror(status));
    return;
  }
  free_write_req(req);
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
  uv_connect_t connection;

  uv_tcp_init(loop, &socket);
  uv_ip4_addr(DEFAULT_SERVER, DEFAULT_PORT, &addr);

  uv_tcp_connect(&connection, &socket, (const struct sockaddr *)&addr,
                 on_connection);
  return uv_run(loop, UV_RUN_DEFAULT);
}