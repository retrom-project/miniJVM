#include <assert.h>
#include "jvm.h"
#include "jvm_util.h"

extern s32 org_mini_fs_InnerFile_openFile(Runtime *, JClass *);
extern s32 org_mini_zip_ZipFile_getEntryIndex0(Runtime *, JClass *);
static Runtime runtime;
static JavaThreadInfo thread;
static int calls;
/* Intercept the actual OS boundary, just as Emscripten proxies it to the
 * browser thread. The collector must be able to recognize this wait. */
FILE *__wrap_fopen(const char *name, const char *mode) {
    (void) name; (void) mode;
    assert(thread.is_blocking && "native file proxy is invisible to VM safepoints");
    calls++;
    return NULL;
}
int main(void) {
    LocalVarItem variables[4] = {0};
    StackEntry operands[16] = {0};
    RuntimeStack stack = {0};
    MiniJVM vm = {0};
    Instance path = {0}, mode = {0};
    path.arr_body = "/controlled-file"; path.arr_length = 17;
    mode.arr_body = "rb"; mode.arr_length = 3;
    runtime.jvm = &vm; runtime.localvar = variables; runtime.thrd_info = &thread;
    runtime.stack = &stack; stack.sp = operands;
    localvar_setRefer(variables, 0, &path); localvar_setRefer(variables, 1, &mode);
    for (int nested = 0; nested < 2; nested++) {
        thread.is_blocking = nested;
        org_mini_fs_InnerFile_openFile(&runtime, NULL);
        assert(thread.is_blocking == nested && "I/O cleared an inherited blocking state");
        org_mini_zip_ZipFile_getEntryIndex0(&runtime, NULL);
        assert(thread.is_blocking == nested && "ZIP cleared an inherited blocking state");
    }
    assert(calls >= 4);
    puts("NATIVE_IO_SAFEPOINT_VERIFIED");
    return 0;
}
