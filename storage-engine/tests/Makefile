CC     = gcc
CFLAGS = -Wall -Wextra -g
SRC    = ..

# Todos los fuentes del storage engine
BM_SRCS = $(SRC)/buffer_manager.c $(SRC)/buffer_frame.c \
          $(SRC)/page_table.c $(SRC)/eviction_policy.c $(SRC)/trace.c

BASE_SRCS = $(SRC)/disk.c $(SRC)/page.c $(SRC)/heap.c $(SRC)/schema.c

ALL_SRCS = $(BASE_SRCS) $(BM_SRCS)

TESTS = test_disk \
        test_page \
        test_slot_entry \
        test_heap \
        test_schema \
        test_integration \
        test_buffer_frame \
        test_page_table \
        test_eviction_policy \
        test_buffer_manager \
        test_btree

all: $(TESTS)

test_disk: test_disk.c $(SRC)/disk.c
	$(CC) $(CFLAGS) -o $@ $^

test_page: test_page.c $(SRC)/page.c $(SRC)/disk.c
	$(CC) $(CFLAGS) -o $@ $^

test_slot_entry: test_slot_entry.c $(SRC)/page.c $(SRC)/disk.c
	$(CC) $(CFLAGS) -o $@ $^

test_heap: test_heap.c $(ALL_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

test_schema: test_schema.c $(ALL_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

test_integration: test_integration.c $(ALL_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

test_buffer_frame: test_buffer_frame.c $(SRC)/buffer_frame.c $(SRC)/disk.c
	$(CC) $(CFLAGS) -o $@ $^

test_page_table: test_page_table.c $(SRC)/page_table.c \
                 $(SRC)/buffer_frame.c $(SRC)/disk.c
	$(CC) $(CFLAGS) -o $@ $^

test_eviction_policy: test_eviction_policy.c $(SRC)/eviction_policy.c \
                      $(SRC)/buffer_frame.c $(SRC)/disk.c
	$(CC) $(CFLAGS) -o $@ $^

test_buffer_manager: test_buffer_manager.c $(ALL_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

test_btree: test_btree.c $(SRC)/btree.c $(SRC)/disk.c
	$(CC) $(CFLAGS) -o $@ $^

run: all
	@echo "\n=== test_disk ===" && ./test_disk
	@echo "\n=== test_page ===" && ./test_page
	@echo "\n=== test_slot_entry ===" && ./test_slot_entry
	@echo "\n=== test_heap ===" && ./test_heap
	@echo "\n=== test_schema ===" && ./test_schema
	@echo "\n=== test_integration ===" && ./test_integration
	@echo "\n=== test_buffer_frame ===" && ./test_buffer_frame
	@echo "\n=== test_page_table ===" && ./test_page_table
	@echo "\n=== test_eviction_policy ===" && ./test_eviction_policy
	@echo "\n=== test_buffer_manager ===" && ./test_buffer_manager
	@echo "\n=== test_btree ===" && ./test_btree

clean:
	rm -f $(TESTS)

.PHONY: all run clean
