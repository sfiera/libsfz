# Copyright (c) 2019 The libsfz Authors
# This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
# under the terms of the MIT License.

LIBSFZ_CPPFLAGS := -I $(LIBSFZ_ROOT)/include/$(TARGET_OS)

LIBSFZ_A := $(OUT)/libsfz.a
LIBSFZ_SRCS := \
	$(LIBSFZ_ROOT)/src/all/sfz/args.cpp \
	$(LIBSFZ_ROOT)/src/all/sfz/digest.cpp \
	$(LIBSFZ_ROOT)/src/all/sfz/encoding.cpp \
	$(LIBSFZ_ROOT)/src/all/sfz/format.cpp \
	$(LIBSFZ_ROOT)/src/all/sfz/string-utils.cpp

ifeq ($(TARGET_OS),win)
LIBSFZ_SRCS += \
	$(LIBSFZ_ROOT)/src/win/sfz/error.cpp \
	$(LIBSFZ_ROOT)/src/win/sfz/file.cpp \
	$(LIBSFZ_ROOT)/src/win/sfz/os.cpp
else
LIBSFZ_SRCS += \
	$(LIBSFZ_ROOT)/src/posix/sfz/error.cpp \
	$(LIBSFZ_ROOT)/src/posix/sfz/file.cpp \
	$(LIBSFZ_ROOT)/src/posix/sfz/os.cpp
endif

LIBSFZ_OBJS := $(LIBSFZ_SRCS:%=$(OUT)/%.o)

$(LIBSFZ_A): $(LIBSFZ_OBJS)
	$(AR) rcs $@ $+

$(LIBSFZ_OBJS): $(OUT)/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CXXFLAGS) $(LIBSFZ_CPPFLAGS) $(LIBPROCYON_CPPFLAGS) -c $< -o $@

-include $(LIBSFZ_OBJS:.o=.d)
