//
// example.cpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2018 Edward Kigwana (ekigwana at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include "path_monitor/path_monitor.hpp"

std::shared_ptr<services::path_monitor> m_path_monitor = nullptr;

void handler(const std::system_error &se, const services::path_monitor_event &t)
{
	std::cout << ">>> handler system error: " << se.what() << " ";

	switch (t.event) {
		case services::path_monitor_event::type::null:
			std::cout << "null";
			break;

		case services::path_monitor_event::type::added:
			std::cout << "added";
			break;

		case services::path_monitor_event::type::removed:
			std::cout << "removed";
			break;

		case services::path_monitor_event::type::modified:
			std::cout << "modified";
			break;

		case services::path_monitor_event::type::renamed_old_name:
			std::cout << "renamed_old_name";
			break;

		case services::path_monitor_event::type::renamed_new_name:
			std::cout << "renamed_new_name";
			break;
	}

	std::cout << " parent path: " << t.parent_path << " path: " << t.path << std::endl;

	std::error_code ec;

	std::cout << "    absolute path: " << std::filesystem::absolute(t.path, ec);
	std::cout << " error code: " << ec.message() << std::endl;

	if (!ec and m_path_monitor)
		m_path_monitor->async_monitor(std::bind(&handler, std::placeholders::_1, std::placeholders::_2));
}

int main(int, char **)
{
	boost::asio::io_context io_context;

	m_path_monitor = std::make_shared<services::path_monitor>(io_context, "Path Monitor");

	std::system_error se;

	std::cout << ">>> Identifier: " << m_path_monitor->identifier() << std::endl;

	// TODO: create path

	m_path_monitor->add_path("x", se);

	std::cout << ">>> " << se.what() << std::endl;

	m_path_monitor->async_monitor(std::bind(&handler, std::placeholders::_1, std::placeholders::_2));

	// TODO: create file in path

	// TODO: delete path

	return 0;
}
