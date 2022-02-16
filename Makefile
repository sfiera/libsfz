# Copyright (c) 2019 The libsfz Authors
# This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
# under the terms of the MIT License.

NINJA := out/cur/ninja -C out/cur

all:
	@$(NINJA)

test: all
	out/cur/args-test
	out/cur/digest-test
	out/cur/encoding-test
	out/cur/optional-test
	out/cur/os-test
	out/cur/string-utils-test

test-wine: all
	wine out/cur/args-test.exe
	wine out/cur/digest-test.exe
	wine out/cur/encoding-test.exe
	wine out/cur/optional-test.exe
	wine out/cur/os-test.exe
	wine out/cur/string-utils-test.exe

clean:
	@$(NINJA) -t clean

distclean:
	rm -Rf out/

.PHONY: all test clean dist distclean
