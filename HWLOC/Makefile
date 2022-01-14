CFLAGS += $(shell pkg-config --cflags hwloc)
LDLIBS += $(shell pkg-config --libs hwloc)

hwloc-hello: hwloc-hello.c
        $(CC) hwloc-hello.c $(CFLAGS) -o hwloc-hello $(LDLIBS)
