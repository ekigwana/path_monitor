//
// path_monitor_event.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SERVICES_PATH_MONITOR_HPP
#define SERVICES_PATH_MONITOR_HPP

#include "basic_path_monitor.hpp"

#if defined(linux) || defined(__linux) || defined(__linux__) || defined(__GNU__) || defined(__GLIBC__)
#	include "inotify/path_monitor_service.hpp"
#else
#	error "Platform not supported."
#endif

namespace services {

/// Typedef for typical path monitor usage.
typedef basic_path_monitor< path_monitor_service<> > path_monitor;

} // namespace services

#endif // SERVICES_PATH_MONITOR_HPP
