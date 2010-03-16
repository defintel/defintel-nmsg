MSG_LIBTOOL_FLAGS = -module -avoid-version
SUBDIRS = example

SUFFIXES = .proto .pb-c.c .pb-c.h

.proto.pb-c.c:
	protoc-c --c_out="`dirname $@`" $<

.proto.pb-c.h:
	protoc-c --c_out="`dirname $@`" $<

moduledir = $(libdir)/nmsg

EXTRA_DIST = \
	sink.c \
	sink.pb-c.c \
	sink.pb-c.h \
	sink.proto


# NOTE: if the nmsg msgmod version (nmsg_msgX) changes, it must be
# updated both here and in check_libnmsg.m4.

module_LTLIBRARIES = nmsg_msg6_defintel.la

nmsg_msg6_defintel_la_CFLAGS = \
	@libnmsg_cflags@
nmsg_msg6_defintel_la_LDFLAGS = \
	$(MSG_LIBTOOL_FLAGS) \
	@libnmsg_ldflags@
nmsg_msg6_defintel_la_SOURCES = nmsg_msg_defintel.c

dep: sink.pb-c.c

.PHONY: protobuf
