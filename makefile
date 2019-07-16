DEBUG?=1
CC?=clang

SRCS  = $(wildcard src/*.c) $(wildcard t/*.c) $(wildcard app/*.c) $(wildcard deps/*/*.c)
TESTS = $(filter t/%,$(SRCS))
OBJS  = $(SRCS:%.c=%.o)

CFLAGS=-Isrc -Ideps -std=c99 -Wall -Werror
LDFLAGS=

ifeq (${DEBUG},1)
	CFLAGS+= -g
endif

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

pdf: $(filter-out t/%,$(OBJS))
	$(CC) $(CFLAGS) -o $@ $^

all: pdf

define make-test
.PHONY: $(1)
$(1):
	$$(CC) $$(CFLAGS) \
		-o $$@ t/$(1).o \
		$$(filter-out t/% app/%,$$(OBJS))
	./$(1)
endef

$(foreach T,$(TESTS:t/%.c=%),$(eval $(call make-test,$(T))))

tests: $(OBJS) $(TESTS:t/%.c=%)

clean:
	rm -rf $(OBJS) pdf
