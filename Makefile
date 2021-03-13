CFLAGS := -fPIC -O3 -g -Wall -Werror
CC := gcc
MAJOR := 0
MINOR := 1
NAME := mycurl
VERSION := $(MAJOR).$(MINOR)

lib: lib$(NAME).so.$(VERSION)

all: test

test: $(NAME)_test
	#LD_LIBRARY_PATH=. ./$(NAME)_test

$(NAME)_test: lib$(NAME).so
	$(CC) $(NAME)_test.c -o $@ -L. -l$(NAME) -lcurl -lpthread

lib$(NAME).so: lib$(NAME).so.$(VERSION)
	ldconfig -v -n .
	ln -s lib$(NAME).so.$(MAJOR) lib$(NAME).so

lib$(NAME).so.$(VERSION): $(NAME).o
	$(CC) $(CFLAGS) -shared -Wl,-soname,lib$(NAME).so.$(MAJOR) $^ -o $@

lib$(NAME).a: $(NAME).o
	ar rcs lib$(NAME).a $(NAME).o

clean:
	$(RM) $(NAME)_test *.o *.so*

install:
	cp lib$(NAME).so.$(MAJOR) /usr/lib64
