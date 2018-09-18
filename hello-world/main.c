#include <stdio.h>
#include <uv.h>

int main(int argc, char**argv) {
  uv_loop_t *loop = uv_default_loop();
  uv_loop_init(loop);
  printf("Hello, world!\n");
  uv_run(loop, UV_RUN_DEFAULT);
  uv_loop_close(loop);
  return 0;
}
