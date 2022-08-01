#include "nkarch.h"
#include "nkuart.h"
#include "nksched.h"
#include "nkcli.h"


#ifdef TEST
int test_tid;

void test_task(void *data)
{
    nk_puts("Hi\r\n");
    nk_sched(test_tid, (void (*)(void *))test_task, NULL, 1000, "Test");
}
#endif

int main(void)
{
    nk_init_uart();
    nk_puts("Hello, world!\r\n");
    nk_init_sched();
    nk_init_cli();
#ifdef TEST
    test_tid = nk_alloc_tid();
    nk_sched(test_tid, (void (*)(void *))test_task, NULL, 1000, "Test");
#endif
    nk_sched_loop();
}
