MAIN = main.c
COMMON_FLAGS = -I . -O2
TABLE_FLAGS = -DTABLE
SIMD_FLAGS = -DSIMD -msse2 -mssse3

build:
	mkdir -p build
	gcc $(MAIN) $(COMMON_FLAGS) -o build/sm4-test
	gcc $(MAIN) $(COMMON_FLAGS) $(TABLE_FLAGS) -o build/sm4-test-table
	gcc $(MAIN) $(COMMON_FLAGS) $(SIMD_FLAGS) -o build/sm4-test-simd

test:
	./build/sm4-test
	./build/sm4-test-table
	./build/sm4-test-simd

clean:
	rm -rf build