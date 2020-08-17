CFILES   = $(wildcard src/*.c)
OBJFILES = $(CFILES:.c=.o)
CSVFILES = $(wildcard structures/*.csv)

RELEASE: tim.wasm generated-structs.js

tim.wasm: $(OBJFILES)
	mkdir -p build
	emcc -O2 $^ -s WASM=1 --no-entry -o $@

%.o: %.c
	emcc -O2 $^ -s WASM=1 -c -o $@

src/generated/structs.h: $(CSVFILES)
	python3 scripts/generate-structs.py > $@

generated-structs.js: $(CSVFILES)
	python3 scripts/generate-structs.py --js > $@

clean:
	rm -f src/*.o
	rm -f tim.wasm
