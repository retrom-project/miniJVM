#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "miniz.h"
#include "miniz_wrapper.h"

static void verify(const char *archive, const char *path, const char *expected, int size, int index) {
    char output[64] = {0};
    ByteBuf *buffer = bytebuf_create(1);
    assert(zip_get_file_index(archive, path) == index);
    assert(zip_get_file_unzip_size(archive, path) == size);
    assert(zip_loadfile_to_mem(archive, path, output, sizeof(output)) == 0);
    assert(memcmp(output, expected, size) == 0);
    assert(zip_loadfile(archive, path, buffer) == 0);
    assert(buffer->wp == (unsigned) size);
    assert(memcmp(buffer->buf, expected, size) == 0);
    bytebuf_destroy(buffer);
}

int main(int argc, char **argv) {
    assert(argc == 2);
    mz_zip_archive archive = {0};
    assert(mz_zip_writer_init_file(&archive, argv[1], 0));
    assert(mz_zip_writer_add_mem(&archive, "pkg/K.class", "UPPER", 5, 0));
    assert(mz_zip_writer_add_mem(&archive, "pkg/k.class", "low", 3, 0));
    assert(mz_zip_writer_add_mem(&archive, "资源/数据.bin", "utf8", 4, 0));
    assert(mz_zip_writer_add_mem(&archive, "empty", "", 0, 0));
    assert(mz_zip_writer_finalize_archive(&archive));
    assert(mz_zip_writer_end(&archive));
    verify(argv[1], "pkg/K.class", "UPPER", 5, 0);
    verify(argv[1], "pkg/k.class", "low", 3, 1);
    verify(argv[1], "/pkg/k.class", "low", 3, 1);
    verify(argv[1], "资源/数据.bin", "utf8", 4, 2);
    verify(argv[1], "empty", "", 0, 3);
    assert(zip_get_file_index(argv[1], "PKG/k.class") == -1);
    assert(zip_get_file_unzip_size(argv[1], "PKG/k.class") == -1);
    unlink(argv[1]);
    puts("ZIP exact names, size/index/read agreement, cached reads and UTF-8 verified.");
    return 0;
}
