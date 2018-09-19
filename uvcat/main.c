
#include <uv.h>
#include <assert.h>
uv_fs_t open_req, read_req, write_req;
uv_buf_t iov;
char buffer[1024] = {0};

void on_open(uv_fs_t *req);
void on_read(uv_fs_t *req);
void on_write(uv_fs_t *req);

void on_write(uv_fs_t *req) {
  if (req->result < 0) {
    fprintf(stderr, "write failed: %s\n", uv_strerror(req->result));
  } else {
    uv_fs_read(uv_default_loop(), &read_req, open_req.result, &iov, 1, -1, on_read);
  }
}

void on_read(uv_fs_t *req) {
  assert(req == &read_req);
  if (req->result < 0) {
    fprintf(stderr, "read failed: %s\n", uv_strerror(req->result));
  } else if (req->result == 0) {
    uv_fs_t close_req;
    uv_fs_close(uv_default_loop(), &close_req, open_req.result, NULL);
  } else {
    iov.len = req->result;
    uv_fs_write(uv_default_loop(), &write_req, 1, &iov, 1, -1, on_write);
  }
}

void on_open(uv_fs_t *req) {
  assert(req == &open_req);
  if (req->result >= 0) {
    iov = uv_buf_init(buffer, sizeof(buffer));
    uv_fs_read(uv_default_loop(), &read_req, req->result, &iov, 1, -1, on_read);
  } else {
    fprintf(stderr, "open failed: %s\n", uv_strerror(req->result));
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: uvcat <FILE>\n");
    return 0;
  }
  uv_fs_open(uv_default_loop(), &open_req, argv[1], O_RDONLY, S_IRUSR | S_IRGRP, on_open);
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);

  uv_fs_req_cleanup(&open_req);
  uv_fs_req_cleanup(&read_req);
  uv_fs_req_cleanup(&write_req);
  return 0;
}