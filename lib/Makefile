ifeq ($(OS),Windows_NT)
	uname_S := Windows
else
	uname_S := $(shell uname -s)
endif

ifeq ($(uname_S), Windows)
	target = mycurl.dll
endif
ifeq ($(uname_S), Linux)
	target = libmycurl.so
endif

%.o: %.c
	gcc  -c $<  -fPIC  -o $@

$(target): mycurl.o
	gcc  $^  -shared -lcurl -o $@

clean:
	$(RM) *.o *.so* *.dll
