#ifndef COMMON_H
#define COMMON_H
#ifdef DEBUG
#define DEBUG_PRINTF(...)                                                      \
  do {                                                                         \
    fprintf(stderr, "%s:%d at %s: ", __FILE__, __LINE__, __func__);            \
    fprintf(stderr, __VA_ARGS__);                                              \
  } while (0)
#else
#define DEBUG_PRINTF(...) void(0)
#endif // !DEBUG
#endif // !#ifndef COMMON_H
