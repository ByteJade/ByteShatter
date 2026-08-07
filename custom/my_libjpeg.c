#include "wrapper.h"

WRAP_FUNC(jpeg_read_header)
WRAP_FUNC(jpeg_start_decompress)
WRAP_FUNC_VOID(jpeg_CreateDecompress)
WRAP_FUNC_VOID(jpeg_destroy_decompress)
WRAP_FUNC(jpeg_std_error)
WRAP_FUNC_VOID(jpeg_read_scanlines)
WRAP_FUNC(jpeg_finish_decompress)