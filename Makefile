# Copyright (c) 2019 The libsfz Authors
# This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
# under the terms of the MIT License.

NINJA=build/lib/bin/ninja -C out/cur

all:
	@$(NINJA)

test: all
	out/cur/args-test
	out/cur/digest-test
	out/cur/encoding-test
	out/cur/optional-test
	out/cur/os-test
	out/cur/string-map-test
	out/cur/string-utils-test

clean:
	@$(NINJA) -t clean

distclean:
	rm -Rf out/

.PHONY: all test clean dist distclean
