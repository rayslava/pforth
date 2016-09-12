BINARY=pforth
JOBS=$(shell grep -c '^processor' /proc/cpuinfo)

all: stylecheck doxygen asan lsan msan clang release clang-release analyzed

doxygen:
	doxygen Doxyfile

build-dir = \
	rm -rf $1-build && mkdir $1-build && cd $1-build

debug:
	$(call build-dir, $@) && cmake .. -DCMAKE_BUILD_TYPE=Debug && $(MAKE) -j $(JOBS) && ctest -j $(JOBS)

release: debug
	$(call build-dir, $@) && cmake .. -DCMAKE_BUILD_TYPE=Release && $(MAKE) -j $(JOBS) && ctest -j $(JOBS)

static-release:
	$(call build-dir, $@) && cmake .. -DCMAKE_BUILD_TYPE=Release -DSTATIC=True && $(MAKE) -j $(JOBS) && ctest -j $(JOBS)

asan:
	$(call build-dir, $@) && cmake .. -DCMAKE_CXX_FLAGS="-fsanitize=address" -DCMAKE_BUILD_TYPE=RelWithDebInfo && $(MAKE) -j $(JOBS) && ctest -j $(JOBS)

lsan:
	$(call build-dir, $@) && CXX=clang++ CC=clang cmake .. -DCMAKE_CXX_FLAGS="-fsanitize=leak"  -DCMAKE_BUILD_TYPE=RelWithDebInfo && $(MAKE) -j $(JOBS) && ctest -j $(JOBS)

msan:
	$(call build-dir, $@) && CXX=clang++ CC=clang cmake .. -DCMAKE_CXX_FLAGS="-fsanitize=memory"  -DCMAKE_BUILD_TYPE=RelWithDebInfo && $(MAKE) -j $(JOBS) && ctest -j $(JOBS)

clang:
	$(call build-dir, $@) && CXX=clang++ CC=clang cmake .. && $(MAKE) $(BINARY) -j $(JOBS)

clang-release:
	$(call build-dir, $@) && CXX=clang++ CC=clang cmake .. -DCMAKE_BUILD_TYPE=Release && $(MAKE) $(BINARY) -j $(JOBS)

analyzed:
	$(call build-dir, $@) && scan-build cmake ..  && scan-build $(MAKE) $(BINARY) -j $(JOBS)

tidy:	debug
	$(call build-dir, $@) && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=True ..  && clang-tidy -p debug-build ../src/*.c && $(MAKE) -j $(JOBS)

clean:
	rm -rf *-build doc ; find -name '*~' -delete

memcheck-binary = \
	(valgrind --tool=memcheck --track-origins=yes --leak-check=full --trace-children=yes --show-reachable=yes ./$1 2>/tmp/unit-test-valg-$1.log)</dev/null && sed '/in use/!d;s/.*exit:.*\s\([[:digit:]]\+\)\sblocks.*/\1/' /tmp/unit-test-valg-$1.log | { read lines; test $$lines -eq 1 || cat /tmp/unit-test-valg-$1.log; }

memcheck: debug release
	$(call build-dir, $@) && cmake .. -DCMAKE_BUILD_TYPE=Debug && $(MAKE) -j $(JOBS)

memcheck-stack: memcheck
	cd memcheck-build && $(call memcheck-binary,stack_test)

memcheck-pforth: memcheck
	cd memcheck-build && $(call memcheck-binary,pforth)

memcheck-math: memcheck
	cd memcheck-build && $(call memcheck-binary,math_test)

memcheck-interp: memcheck
	cd memcheck-build && $(call memcheck-binary,interp_test)

memcheck-words: memcheck
	cd memcheck-build && $(call memcheck-binary,words_test)

valgrind: memcheck-stack memcheck-pforth memcheck-math memcheck-interp memcheck-words

dockerimage:
	cd dockerbuild && (docker images | grep $(BINARY)-deploy) || docker build -t $(BINARY)-deploy .

dockerbuild/$(BINARY).tar.gz:
	git archive --format=tar.gz -o dockerbuild/$(BINARY).tar.gz --prefix=$(BINARY)/ HEAD

stylecheck:
	uncrustify -c uncrustify.cfg src/*.c src/*.h test/*.c --check -l c

stylefix:
	uncrustify -c uncrustify.cfg src/*.c src/*.h test/*.c --replace -l c

coverage:
	$(call build-dir, $@) && cmake .. -DCOVERAGE=True && $(MAKE) -j $(JOBS) && $(MAKE) clean && $(MAKE) -j 1 coverage

coveralls:
	$(call build-dir, $@) && cmake .. -DCOVERALLS=True && $(MAKE) -j $(JOBS) && $(MAKE) clean && $(MAKE) -j 1 coveralls

deploy: debug release clang clang-release analyzed memcheck dockerbuild/$(BINARY).tar.gz
	cd dockerbuild && docker run -v "$(shell pwd)/dockerbuild:/mnt/host" $(BINARY)-deploy /bin/bash -c 'cd /root && tar xfz /mnt/host/$(BINARY).tar.gz && cd $(BINARY) && mkdir build &&p cd build && cmake -DSTATIC=True -DCMAKE_BUILD_TYPE=RelWithDebInfo .. && make $(BINARY) -j $(JOBS) && cp $(BINARY) /mnt/host'
