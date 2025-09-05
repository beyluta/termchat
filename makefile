.PHONY: clean build update init

COMPILER = clang
OUTDIR = build
OUT = $(OUTDIR)/out
LIBS = -lcurl
SOURCES = src/main.c \
					src/window.c \
					src/config.c \
					src/globdef.c \
					src/completions.c \
					minimal-c-json-parser/src/json.c
INCLUDES = -Iminimal-c-json-parser/include
ARGS = -Wall \
			 -Werror \
			 -Wextra \
			 -std=c23 \
			 -O2

init:
	git submodule update --init --recursive
update:
	git submodule update --recursive --remote
clean:
	rm -f $(OUT)
build: clean
	mkdir -p $(OUTDIR)
	$(COMPILER) $(INCLUDES) $(SOURCES) -o $(OUT) $(LIBS) $(ARGS)
