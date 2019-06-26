#include "student_code.hpp"

// C++20 eventually supports the following syntax.
//
// #define EXPAND_ARGV(...) __VA_ARGS__ __VA_OPT__(,) nullptr
//
// Until then, the following macros handle up to 64 arguments explicitly
#define _IS_EMPTY( _1,  _2,  _3,  _4,  _5,  _6,  _7,  _8,  _9, _10,     \
                  _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,     \
                  _21, _22, _23, _24, _25, _26, _27, _28, _29, _30,     \
                  _31, _32, _33, _34, _35, _36, _37, _38, _39, _40,     \
                  _41, _42, _43, _44, _45, _46, _47, _48, _49, _50,     \
                  _51, _52, _53, _54, _55, _56, _57, _58, _59, _60,     \
                  _61, _62, _63, _64,   N, ...) N
#define IS_EMPTY(...) _IS_EMPTY(dummy, ##__VA_ARGS__,                   \
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  		\
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  		\
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  		\
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  		\
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  		\
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  		\
                                0, 0, 0, 1 )
#define _CAT(a,b) a ## b
#define  CAT(a,b) _CAT(a,b)
#define _EXPAND_ARGV1(...) "program_name", nullptr
#define _EXPAND_ARGV0(...) "program_name", __VA_ARGS__, nullptr
#define  EXPAND_ARGV(...)  CAT(_EXPAND_ARGV, IS_EMPTY(__VA_ARGS__))(__VA_ARGS__)

#ifdef __cplusplus
    // Prevent name mangling
    extern "C" {
#endif

void test_student_code() {

  const char *argv[] = {
    EXPAND_ARGV(@@student_argv@@)
  };
  
  __main__(sizeof argv/sizeof argv[0]-1, const_cast<char**>(argv));
}
      
#ifdef __cplusplus
    }
#endif
