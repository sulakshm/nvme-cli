prefix ?= /usr
includedir ?= $(prefix)/include
libdir ?= $(prefix)/lib

CFLAGS ?= -g -fomit-frame-pointer -O2
override CFLAGS += -Wall -fPIC -Iinclude/
SO_CFLAGS=-shared $(CFLAGS)
L_CFLAGS=$(CFLAGS)
LINK_FLAGS=
LINK_FLAGS+=$(LDFLAGS)
ENABLE_SHARED ?= 1

soname=libnvme.so.1
minor=0
micro=2
libname=$(soname).$(minor).$(micro)
all_targets += libnvme.a

ifeq ($(ENABLE_SHARED),1)
all_targets += $(libname)
endif

ifneq ($(findstring $(MAKEFLAGS),s),s)
ifndef V
	QUIET_CC	= @echo '    '   CC $@;
	QUIET_LINK	= @echo '  '   LINK $@;
	QUIET_AR	= @echo '    '   AR $@;
	QUIET_RANLIB	= @echo '' RANLIB $@;
endif
endif

all: $(all_targets)

libnvme_srcs := nvme/nvme-ioctl.c

libnvme_objs := $(patsubst %.c,%.ol,$(libnvme_srcs))
libnvme_sobjs := $(patsubst %.c,%.os,$(libnvme_srcs))

$(libnvme_objs) $(libnvme_sobjs): libnvme.h nvme/nvme-struct.h nvme/nvme-ioctl.h

%.os: %.c
	$(QUIET_CC)$(CC) $(SO_CFLAGS) -c -o $@ $<

%.ol: %.c
	$(QUIET_CC)$(CC) $(L_CFLAGS) -c -o $@ $<

AR ?= ar
RANLIB ?= ranlib
libnvme.a: $(libnvme_objs)
	@rm -f libnvme.a
	$(QUIET_AR)$(AR) r libnvme.a $^
	$(QUIET_RANLIB)$(RANLIB) libnvme.a

$(libname): $(libnvme_sobjs) libnvme.map
	$(QUIET_CC)$(CC) $(SO_CFLAGS) -Wl,--version-script=libnvme.map -Wl,-soname=$(soname) -o $@ $(libnvme_sobjs) $(LINK_FLAGS)

install: $(all_targets)
	install -D -m 644 nvme/nvme-struct.h $(includedir)/nvme/nvme-struct.h
	install -D -m 644 nvme/nvme-ioctl.h $(includedir)/nvme/nvme-ioctl.h
	install -D -m 644 libnvme.h $(includedir)/libnvme.h
	install -D -m 644 libnvme.a $(libdir)/libnvme.a
ifeq ($(ENABLE_SHARED),1)
	install -D -m 755 $(libname) $(libdir)/$(libname)
	ln -sf $(libname) $(libdir)/$(soname)
	ln -sf $(libname) $(libdir)/libnvme.so
endif

$(libnvme_objs): libnvme.h

clean:
	rm -f $(all_targets) $(libnvme_objs) $(libnvme_sobjs) $(soname).new
	rm -f *.so* *.a *.o

