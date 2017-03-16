NINJA=build/lib/bin/ninja -C out/cur

all:
	@$(NINJA)

test: all
	out/cur/algorithm-test
	out/cur/args-test
	out/cur/bytes-test
	out/cur/digest-test
	out/cur/encoding-test
	out/cur/optional-test
	out/cur/os-test
	out/cur/print-test
	out/cur/read-test
	out/cur/string-map-test
	out/cur/string-test
	out/cur/string-utils-test
	out/cur/write-test

clean:
	@$(NINJA) -t clean

distclean:
	rm -Rf out/

.PHONY: all test clean dist distclean
