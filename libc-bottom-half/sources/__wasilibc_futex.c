#include <wasi/api.h>
#include <sys/types.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>

int __wasilibc_futex_wait_wasix(volatile void *addr, int op, int expected, int64_t max_wait_ns) {
  if ((((intptr_t)addr) & 3) != 0) {
    return -EINVAL;
  }

  __wasi_bool_t woken = __WASI_BOOL_FALSE;
  
  __wasi_option_timestamp_t timeout;
  if (max_wait_ns > 0) {
    timeout.tag = __WASI_OPTION_SOME;
    timeout.u.none = max_wait_ns;
  } else {
    timeout.tag = __WASI_OPTION_NONE;
    timeout.u.none = 0;
  }

  // int ret = __builtin_wasm_memory_atomic_wait32((int *)addr, val, max_wait_ns);
  // memory.atomic.wait32 returns:
  //   0 => "ok", woken by another agent.
  //   1 => "not-equal", loaded value != expected value
  //   2 => "timed-out", the timeout expired
  volatile int *paddr = (volatile int *)addr;
  if (*paddr != expected) {
    return -EWOULDBLOCK;
  }

  int ret = __wasi_futex_wait((uint32_t*)addr, expected, &timeout, &woken);
  if (ret != 0) {
    return -ret;
  }

  if (woken == __WASI_BOOL_FALSE && *paddr == expected) {
    return -ETIMEDOUT;
  }
  return 0;
}
