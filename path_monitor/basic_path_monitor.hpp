//
// basic_path_monitor.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2018 Edward Kigwana (ekigwana at gmail dot com)
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SERVICES_BASIC_PATH_MONITOR_HPP
#define SERVICES_BASIC_PATH_MONITOR_HPP

#include <filesystem>

#define BOOST_ERROR_CODE_HEADER_ONLY
#include <boost/asio/io_context.hpp>

namespace services {

struct path_monitor_event
{
	enum class type
	{
		null = 0,
		added = 1,
		removed = 2,
		modified = 3,
		renamed_old_name = 4,
		renamed_new_name = 5
	};

	path_monitor_event() {}

	path_monitor_event(const std::filesystem::path&pp, const std::filesystem::path &p, path_monitor_event::type t)
		: parent_path(pp), path(p), event(t) { }

	std::filesystem::path parent_path;	// Facilitates filtering events for a particular directory in handler.
	std::filesystem::path path;		// Path.
	type event = type::null;
};

/// Class to provide simple logging functionality. Use the services::logger
/// typedef.
template <typename Service>
class basic_path_monitor
{
public:
	/// The type of the service that will be used to provide timer operations.
	typedef Service service_type;

	/// The native implementation type of the timer.
	typedef typename service_type::impl_type impl_type;

	/// Constructor.
	/**
	* This constructor creates a logger.
	*
	* @param io_context The io_context object used to locate the logger service.
	*
	* @param identifier An identifier for this logger.
	*/
	explicit basic_path_monitor(boost::asio::io_context &io_context, const std::string &identifier)
		: m_service(boost::asio::use_service<Service>(io_context))
	{
		m_service.create(m_impl, identifier);
	}

	basic_path_monitor(basic_path_monitor &&) noexcept;		// Movable.
	basic_path_monitor& operator=(basic_path_monitor &&) noexcept;	// Noncopyable.

	~basic_path_monitor()
	{
		stop();
	}

	void stop()
	{
		if (!m_impl)
			return;

		m_service.destroy(m_impl);
	}

	const std::string identifier() {
		auto str = m_service.identifier(m_impl);

		return m_service.identifier(m_impl);
	}

	/// Get the io_context associated with the object.
	boost::asio::io_context &get_io_context()
	{
		return m_service.get_io_context();
	}

	/// Add path to monitor.
	void add_path(const std::filesystem::path &path, std::system_error &se)
	{
		m_service.add_path(m_impl, path, se);
	}

	/// Remove path from monitor.
	void remove_path(const std::filesystem::path &path, std::system_error &se)
	{
		m_service.remove_path(m_impl, path, se);
	}

	/// Monitor path events synchronously.
	path_monitor_event monitor(std::system_error &se)
	{
		return m_service.monitor(m_impl, se);
	}

	template <typename Handler>
	void async_monitor(Handler handler)
	{
		m_service.async_monitor(m_impl, handler);
	}

private:
	/// The backend service implementation.
	service_type &m_service;

	/// The underlying native implementation.
	impl_type m_impl = nullptr;
};

} // namespace services

#endif // SERVICES_BASIC_PATH_MONITOR_HPP
