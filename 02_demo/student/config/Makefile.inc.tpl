# C++11 (student_cxx_standard multiple choice: 0)
CXXFLAGS_EXTRA_0 = -std=gnu++11
LDFLAGS_EXTRA_0  = -std=gnu++11

# C++14 (student_cxx_standard multiple choice: 1)
CXXFLAGS_EXTRA_1 = -std=gnu++14
LDFLAGS_EXTRA_1  = -std=gnu++14

# C++17 (student_cxx_standard multiple choice: 2)
CXXFLAGS_EXTRA_2 = -std=gnu++17
LDFLAGS_EXTRA_2  = -std=gnu++17

# C++20 (student_cxx_standard multiple choice: 3)
CXXFLAGS_EXTRA_3 = -std=gnu++2a
LDFLAGS_EXTRA_3  = -std=gnu++2a

CXXFLAGS_EXTRA   = -O3 -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-unused-variable -Wno-variadic-macros -Wvla -Wall -Wextra -Werror ${CXXFLAGS_EXTRA_@@student_cxx_standard@@}
LDFLAGS_EXTRA    = -O3 -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-unused-variable -Wno-variadic-macros -Wvla -Wall -Wextra -Werror ${LDFLAGS_EXTRA_@@student_cxx_standard@@}
