// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_FOREACH_HPP_
#define SFZ_FOREACH_HPP_

#define foreach(VAR, CONTAINER) \
    for (bool __loop = true; __loop; ) \
    for (const typeof(CONTAINER)& __container = (CONTAINER); __loop; __loop = false) \
    for (typeof(__container.begin()) VAR = __container.begin(), __end = __container.end(); \
            VAR != __end; ++VAR)

#endif  // SFZ_FOREACH_HPP_
