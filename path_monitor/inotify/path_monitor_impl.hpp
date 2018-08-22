//
// path_monitor_impl.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2018 Edward Kigwana (ekigwana at gmail dot com)
// Copyright (c) 2008, 2009 Boris Schaeling <boris@highscore.de>
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SERVICES_PATH_MONITOR_IMPL_HPP
#define SERVICES_PATH_MONITOR_IMPL_HPP

#include <condition_variable>
#include <deque>
#include <filesystem>
#include <mutex>
#include <thread>
#include <system_error>

#define BOOST_ERROR_CODE_HEADER_ONLY
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/bimap.hpp>
#include <sys/inotify.h>
#include <errno.h>

namespace services {

class path_monitor_impl
	: public std::enable_shared_from_this<path_monitor_impl>
{
public:
	path_monitor_impl(const std::string &identifier)
		: m_identifier(identifier),
		m_fd(init_fd()),
		m_stream_descriptor(m_inotify_io_context, m_fd),
		m_inotify_work(boost::asio::make_work_guard(m_inotify_io_context)),
		m_inotify_work_thread(std::bind(static_cast<std::size_t (boost::asio::io_context::*)()>(
			&boost::asio::io_context::run), &m_inotify_io_context))
	{
	}

	/// Return service identifier.
	const std::string identifier()
	{
		return m_identifier;
	}

	/// Add path to monitor.
	void add_path(const std::filesystem::path &path, std::system_error &se)
	{
		int wd = inotify_add_watch(m_fd, path.c_str(), IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVE);

		if (wd == -1) {
			se = std::system_error(std::error_code(errno, std::system_category()),
					       "service::path_monitor_impl::add_path: inotify_add_watch for \"" +
					       path.string() + "\" path failed");

			return;
		}

		std::unique_lock<std::mutex> lk(m_watch_descriptors_mutex);

		m_watch_descriptors.insert(watch_descriptors_type::value_type(wd, path.string()));

		se = std::system_error(std::error_code());
	}

	/// Remove path from monitor.
	void remove_path(const std::filesystem::path &path, std::system_error &se)
	{
		std::unique_lock<std::mutex> lk(m_watch_descriptors_mutex);

		auto it = m_watch_descriptors.right.find(path.string());

		if (it != m_watch_descriptors.right.end()) {
			int i = inotify_rm_watch(m_fd, it->second);

			if (i == -1) {
				se = std::system_error(std::error_code(errno, std::system_category()),
						       "service::path_monitor_impl::remove_path: inotify_rm_watch for \"" +
						       path.string() + "\" path failed");

				return;
			}

			m_watch_descriptors.right.erase(it);
		}

		se = std::system_error(std::error_code());
	}

	/// Destroy a path monitor implementation.
	void destroy()
	{
		m_inotify_work.reset();
		m_inotify_io_context.stop();
		m_inotify_work_thread.join();

		std::unique_lock<std::mutex> lk(m_events_mutex);

		m_run = false;

		m_events_cond.notify_all();
	}

	/// Get earliest inotify event (FIFO).
	path_monitor_event popfront_event(std::system_error &se)
	{
		std::unique_lock<std::mutex> lk(m_events_mutex);

		while (m_run && m_events.empty())
			m_events_cond.wait(lk);

		path_monitor_event ev;

		if (!m_events.empty()) {
			ev = m_events.front();

			m_events.pop_front();

			se = std::system_error(std::error_code());
		} else {
			se = std::system_error(std::error_code(static_cast<int>(std::errc::operation_canceled), std::system_category()),
					       "service::path_monitor_impl::popfront_event: operation canceled");
		}

		return ev;
	}

	/// Insert inotify event into FIFO.
	void pushback_event(path_monitor_event ev)
	{
		std::unique_lock<std::mutex> lk(m_events_mutex);

		if (m_run) {
			m_events.push_back(ev);
			m_events_cond.notify_all();
		}
	}

private:
	int init_fd()
	{
		int fd = inotify_init1(IN_NONBLOCK);

		if (fd == -1) {
			throw std::system_error(std::error_code(errno, std::system_category()),
						"service::path_monitor_impl::init_fd: inotify_init1 failed");
		}

		return fd;
	}

public:
	void begin_read()
	{
		m_stream_descriptor.async_read_some(boost::asio::buffer(m_read_buffer),
						    std::bind(&path_monitor_impl::end_read, shared_from_this(),
							      std::placeholders::_1, std::placeholders::_2));
	}

private:
	void end_read(const std::error_code &ec, std::size_t bytes_transferred)
	{
		if (!ec) {
			m_pending_read_buffer += std::string(m_read_buffer.data(), bytes_transferred);

			while (m_pending_read_buffer.size() >= sizeof(inotify_event)) {
				const inotify_event *iev = reinterpret_cast<const inotify_event*>(m_pending_read_buffer.data());

				if (iev->mask & (IN_UNMOUNT | IN_Q_OVERFLOW | IN_IGNORED)) {
					m_pending_read_buffer.erase(0, sizeof(inotify_event) + iev->len);

					continue;
				}

				auto type = path_monitor_event::type::null;

				switch (iev->mask & 0xFFF) {
					case IN_MODIFY:
						type = path_monitor_event::type::modified;
						break;

					case IN_CREATE:
						type = path_monitor_event::type::added;
						break;

					case IN_DELETE:
						type = path_monitor_event::type::removed;
						break;

					case IN_MOVED_FROM:
						type = path_monitor_event::type::renamed_old_name;
						break;

					case IN_MOVED_TO:
						type = path_monitor_event::type::renamed_new_name;
						break;
				}

				pushback_event(path_monitor_event(get_dirname(iev->wd), iev->name, type));

				m_pending_read_buffer.erase(0, sizeof(inotify_event) + iev->len);
			}

			begin_read();
		} else if (ec != std::errc::operation_canceled) {
			throw std::system_error(std::error_code(ec.value(), ec.category()), ec.message());
		}
	}

	std::string get_dirname(int wd)
	{
		std::unique_lock<std::mutex> lk(m_watch_descriptors_mutex);

		auto it = m_watch_descriptors.left.find(wd);

		return it != m_watch_descriptors.left.end() ? it->second : "";
	}

	std::string m_identifier;
	int m_fd;
	boost::asio::io_context m_inotify_io_context;
	boost::asio::posix::stream_descriptor m_stream_descriptor;
	boost::asio::executor_work_guard<boost::asio::io_context::executor_type> m_inotify_work;
	std::thread m_inotify_work_thread;
	std::array<char, 4096> m_read_buffer;
	std::string m_pending_read_buffer;
	std::mutex m_watch_descriptors_mutex;
	typedef boost::bimap<int, std::string> watch_descriptors_type;
	watch_descriptors_type m_watch_descriptors;
	std::mutex m_events_mutex;
	std::condition_variable m_events_cond;
	bool m_run = true;
	std::deque<path_monitor_event> m_events;
};

} // namespace services

#endif // SERVICES_PATH_MONITOR_IMPL_HPP
