#include <assert.h>
#include <stdatomic.h>
#include "jvm.h"
#include "jvm_util.h"

static MiniJVM vm;
static atomic_int entered;
static int notifier(void *unused) {
    (void) unused;
    struct timespec delay = {0, 30000000};
    thrd_sleep(&delay, NULL);
    vm_share_lock(&vm);
    atomic_store(&entered, 1);
    vm_share_notifyall(&vm);
    vm_share_unlock(&vm);
    return 0;
}
int main(void) {
    mtx_init(&vm.threadlock.mutex_lock, mtx_recursive);
    cnd_init(&vm.threadlock.thread_cond);
    for (int timed = 0; timed < 2; timed++) {
        atomic_store(&entered, 0);
        vm_share_lock(&vm); vm_share_lock(&vm); vm_share_lock(&vm);
        thrd_t thread;
        assert(thrd_create(&thread, notifier, NULL) == thrd_success);
        while (!atomic_load(&entered)) {
            if (timed) vm_share_timedwait(&vm, 10);
            else vm_share_wait(&vm);
        }
        assert(vm.threadlock.count == 3);
        assert(vm.threadlock.owner_thread == (JavaThreadInfo *)(intptr_t) thrd_current());
        vm_share_unlock(&vm); vm_share_unlock(&vm); vm_share_unlock(&vm);
        assert(vm.threadlock.count == 0 && vm.threadlock.owner_thread == NULL);
        thrd_join(thread, NULL);
    }
    cnd_destroy(&vm.threadlock.thread_cond);
    mtx_destroy(&vm.threadlock.mutex_lock);
    puts("RECURSIVE_METADATA_WAIT_VERIFIED");
    return 0;
}
