
#include <uv.h>

int64_t counter = 0;
void on_idle(uv_idle_t *idle) {
  counter++;
  if (counter > 1000000) {
    uv_idle_stop(idle);
  }
}

int main() {
  uv_idle_t idle;
  uv_idle_init(uv_default_loop(), &idle);
  uv_idle_start(&idle, on_idle);

  uv_run(uv_default_loop(), UV_RUN_DEFAULT);

  uv_loop_close(uv_default_loop());
  return 0;
}