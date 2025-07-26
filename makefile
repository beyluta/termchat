.PHONY: clean build update

COMPILER = clang
OUTDIR = build
OUT = $(OUTDIR)/out
LIBS = -lcurl
SOURCES = src/main.c \
					src/window.c \
					src/config.c \
					src/globdef.c \
					src/completions.c
ARGS = -Wall \
			 -Werror \
			 -Wextra \
			 -std=c23 \
			 -O2

update:
	git submodule update --init --recursive
clean:
	rm -f $(OUT)
build: update clean
	mkdir -p $(OUTDIR)
	$(COMPILER) $(SOURCES) -o $(OUT) $(LIBS) $(ARGS)
