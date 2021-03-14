ifeq ($(OS),Windows_NT)
	uname_S := Windows
else
	uname_S := $(shell uname -s)
endif

ifeq ($(uname_S), Windows)
	target = mycurl_test.exe
endif
ifeq ($(uname_S), Linux)
	target = mycurl_test
endif
ifeq ($(uname_S), Darwin)
	target = mycurl_test
endif

%.o: %.c
	gcc  -c $< -I ../lib  -o $@

$(target): mycurl_test.o
	gcc  $^  -L../lib  -lmycurl -lpthread -o $@

clean:
	$(RM) *.o mycurl_test mycurl_test.exe
