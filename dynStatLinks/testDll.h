#ifndef INCLUDE_TEST_DLL_H
#define INCLUDE_TEST_DLL_H 1

#ifdef __cplusplus
#   define EXTERN extern "C"
#else /* ifdef __cplusplus */
#   define EXTERN extern
#endif /* ifdef __cplusplus */

EXTERN const char *hello();

#endif /* ifndef INCLUDE_TEST_DLL_H */
