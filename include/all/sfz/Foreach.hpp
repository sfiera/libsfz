// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_FOREACH_HPP_
#define SFZ_FOREACH_HPP_

#   if (_MSC_VER >= 1600) && (_MSC_VER < 1700)
#       define SFZ_DECLTYPE(x) decltype(x)
#   elif __GNUC__ == 4
#       define SFZ_DECLTYPE(x) __typeof__(x)
#   else
#       define SFZ_DECLTYPE(x) int
#       error "Don't know how to get decltype-like functionality"
#   endif

#define foreach(VAR, CONTAINER) \
    for (bool __loop = true; __loop; ) \
    for (const SFZ_DECLTYPE(CONTAINER)& __container = (CONTAINER); __loop; __loop = false) \
    for (SFZ_DECLTYPE(__container.begin()) VAR = __container.begin(), __end = __container.end(); \
            VAR != __end; ++VAR)

#endif  // SFZ_FOREACH_HPP_
