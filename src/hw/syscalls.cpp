// Newlib syscall stubs. There is no OS on the Seed, so these do nothing. Without
// them, libnosys links its own stubs, each with a "not implemented" linker warning.
#include <sys/stat.h>
#include <unistd.h>

extern "C" {

int _close(int) { return -1; }

int _fstat(int, struct stat* st) {
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int) { return 1; }

_off_t _lseek(int, _off_t, int) { return 0; }

_READ_WRITE_RETURN_TYPE _read(int, void*, size_t) { return 0; }

_READ_WRITE_RETURN_TYPE _write(int, const void*, size_t len) { return len; }

}
