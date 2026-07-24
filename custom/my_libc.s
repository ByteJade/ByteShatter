.include "./wrapper.inc"

.global my_stdout
.extern stdout
.global my_stderr
.extern stderr

.section .data
my_stdout:
    .quad stdout
my_stderr:
    .quad stderr

.section .text

// just call main and return
.global my___libc_start_main
my___libc_start_main:
    stp x29, x30, [sp, #-16]!
    mov x7, x0
    mov x0, x1
    mov x1, x2
    blr x7
    ldp x29, x30, [sp], #16
    ret

.global my___isoc23_strtol
.global my_strtol
.extern strtol
my___isoc23_strtol:
my_strtol:
	b strtol
.global my___isoc23_sscanf
.global my_sscanf
.extern sscanf
my___isoc23_sscanf:
my_sscanf:
	b sscanf

WRAP_BIG_FUNC printf
WRAP_BIG_FUNC vsnprintf
WRAP_BIG_FUNC fprintf
WRAP_BIG_FUNC sprintf
WRAP_FUNC_VOID exit
WRAP_FUNC puts
WRAP_FUNC malloc
WRAP_FUNC memcpy
WRAP_FUNC memset
WRAP_FUNC memcmp
WRAP_FUNC realloc
WRAP_FUNC calloc
WRAP_FUNC_VOID free
WRAP_FUNC strlen
WRAP_FUNC strstr
WRAP_FUNC strcmp
WRAP_FUNC strncmp
WRAP_FUNC strncpy
WRAP_FUNC strchr
WRAP_FUNC strcat
WRAP_FUNC strcspn
WRAP_FUNC strerror
WRAP_FUNC strrchr
WRAP_FUNC fflush
WRAP_FUNC strtod
WRAP_FUNC nanosleep
WRAP_FUNC gettimeofday
WRAP_FUNC localtime
WRAP_FUNC strftime
WRAP_FUNC gmtime
WRAP_FUNC mktime
WRAP_FUNC utime
WRAP_FUNC time
WRAP_FUNC setlocale
WRAP_FUNC ftell
WRAP_FUNC fputs
WRAP_FUNC dcgettext
WRAP_FUNC qsort
WRAP_FUNC rand
WRAP_FUNC_VOID srand

WRAP_FUNC feof
WRAP_FUNC fseek
WRAP_FUNC fseeko
WRAP_FUNC fseeko64
WRAP_FUNC ftello64
WRAP_FUNC remove
WRAP_FUNC rename
WRAP_FUNC opendir
WRAP_FUNC closedir
WRAP_FUNC readdir
WRAP_FUNC mkdir
WRAP_FUNC fopen
WRAP_FUNC fopen64
WRAP_FUNC fwrite
WRAP_FUNC fread
WRAP_FUNC fclose
WRAP_FUNC access

WRAP_FUNC setenv
WRAP_FUNC getenv
WRAP_FUNC unsetenv
