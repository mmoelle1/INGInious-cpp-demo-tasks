# Include test specific makefile
include config/Makefile.inc

# Name of the C++ compiler command
CXX     ?= g++

# Name of the executable
EXEC     = tests

# List of source files
SRC      = $ tests.c student_code.cxx bridge.cxx CTester/wrap_mutex.c CTester/wrap_malloc.c CTester/wrap_file.c CTester/wrap_sleep.c CTester/CTester.c CTester/trap.c

# List of object files
OBJ      = $(SRC:.c=.o)

# Flags for the C/C++ compiler command 
CFLAGS   = -Wall -Werror -DC99 -std=gnu99 -ICTester
CXXFLAGS = $(CXXFLAGS_EXTRA) -Wall -Werror -DC99 -ICTester 

# Flags for the linker command
LDFLAGS  = $(LDFLAGS_EXTRA) -lcunit -lm -lpthread -ldl -rdynamic 

# Additional compiler flags
WRAP=-Wl,-wrap=pthread_mutex_lock -Wl,-wrap=pthread_mutex_unlock -Wl,-wrap=pthread_mutex_trylock -Wl,-wrap=pthread_mutex_init -Wl,-wrap=pthread_mutex_destroy -Wl,-wrap=malloc -Wl,-wrap=free -Wl,-wrap=realloc -Wl,-wrap=calloc -Wl,-wrap=open -Wl,-wrap=creat -Wl,-wrap=close -Wl,-wrap=read -Wl,-wrap=write -Wl,-wrap=stat -Wl,-wrap=fstat -Wl,-wrap=lseek -Wl,-wrap=exit -Wl,-wrap=sleep

all: $(EXEC)

%.o: %.c %.cxx
	$(CXX) $(CXXFLAGS) -c -o $@ $< 

$(EXEC): $(OBJ)
	$(CXX) $(WRAP) -o $@ $(OBJ) $(LDFLAGS)

create-po:
	mkdir -p po/fr/
	xgettext --keyword=_ --language=C --add-comments --sort-output --from-code=UTF-8 -o po/tests.pot $(SRC)
	msginit --input=po/tests.pot --locale=fr_BE.utf8 --output=po/fr/tests.po

update-po:
	xgettext --keyword=_ --language=C --add-comments --sort-output --from-code=UTF-8 -o po/tests.pot $(SRC)
	msgmerge --update po/fr/tests.po po/tests.pot

compile-mo:
	msgfmt --no-hash --output-file=po/fr/tests.mo po/fr/tests.po
	mkdir -p fr/LC_MESSAGES/
	cp po/fr/tests.mo fr/LC_MESSAGES/tests.mo

clean:
	rm -f $(EXEC) $(OBJ)

.PHONY: tests

