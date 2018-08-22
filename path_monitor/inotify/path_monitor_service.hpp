//
// path_monitor_service.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2018 Edward Kigwana (ekigwana at gmail dot com)
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SERVICES_PATH_MONITOR_SERVICE_HPP
#define SERVICES_PATH_MONITOR_SERVICE_HPP

#include "path_monitor_impl.hpp"

namespace services {

/// Service implementation for the path monitor.
template <typename FileMonitorImplementation = path_monitor_impl>
class path_monitor_service
	: public boost::asio::io_context::service
{
public:
	/// The unique service identifier.
	static boost::asio::io_context::id id;

	/// The type for an implementation of the path monitor.
	typedef std::shared_ptr<path_monitor_impl> impl_type;

	/// Constructor creates a thread to run a private io_context.
	path_monitor_service(boost::asio::io_context &io_context)
		: boost::asio::io_context::service(io_context),
		m_work_io_context(),
		m_work(boost::asio::make_work_guard(m_work_io_context)),
		m_work_thread(std::make_shared<std::thread>(
			std::bind(static_cast<std::size_t (boost::asio::io_context::*)()>(
				&boost::asio::io_context::run), &m_work_io_context)))
	{
	}

	path_monitor_service(path_monitor_service &&) noexcept;			// Movable.
	path_monitor_service& operator=(path_monitor_service &&) noexcept;	// Noncopyable.

	/// Destructor shuts down the private io_context.
	~path_monitor_service()
	{
		/// Indicate that we have finished with the private io_context. Its
		/// io_context::run() function will exit once all other work has completed.
		m_work.reset();

		if (m_work_thread)
			m_work_thread->join();
	}

	/// Destroy all user-defined handler objects owned by the service.
	void shutdown_service() override
	{
	}

	/// Create a new path monitor implementation.
	void create(impl_type &impl, const std::string &identifier)
	{
		impl = std::make_shared<path_monitor_impl>(identifier);

		// begin_read() can't be called within the constructor but must be called
		// explicitly as it calls shared_from_this().
		impl->begin_read();
	}

	/// Destroy a path monitor implementation.
	void destroy(impl_type &impl)
	{
		// If an asynchronous call is currently waiting for an event
		// we must interrupt the blocked call to make sure it returns.
		impl->destroy();
		impl.reset();
	}

	/// Return service identifier.
	const std::string identifier(impl_type &impl)
	{
		return impl->identifier();
	}

	/// Add path to monitor.
	void add_path(impl_type &impl, const std::filesystem::path &path, std::system_error &se)
	{
		impl->add_path(path, se);
	}

	/// Remove path from monitor.
	void remove_path(impl_type &impl, const std::filesystem::path &path, std::system_error &se)
	{
		impl->remove_path(path, se);
	}

	/// Monitor path events synchronously.
	path_monitor_event monitor(impl_type &impl, std::system_error &se)
	{
		return impl->popfront_event(se);
	}

	/// Class to facilitate monitoring operations asynchronously.
	template <typename Handler>
	class monitor_operation
	{
	public:
		monitor_operation(impl_type impl, boost::asio::io_context &io_context, Handler handler)
			: m_impl(impl),
			m_io_context(io_context),
			m_work(boost::asio::make_work_guard(io_context)),
			m_handler(handler)
		{
		}

		~monitor_operation()
		{
			m_work.reset();
		}

		void operator()() const
		{
			auto impl = m_impl.lock();

			if (impl) {
				std::system_error se;

				auto ev = impl->popfront_event(se);

				this->m_io_context.post(boost::asio::detail::bind_handler(m_handler, se, ev));
			} else {
				this->m_io_context.post(boost::asio::detail::bind_handler(
					m_handler,
					std::system_error(std::error_code(static_cast<int>(std::errc::operation_canceled), std::system_category()),
							  "service::path_monitor_service::monitor_operation: operation canceled"),
					path_monitor_event()));
			}
		}

	private:
		std::weak_ptr<FileMonitorImplementation> m_impl;
		boost::asio::io_context &m_io_context;

		/// Work for the private io_context to perform. If we do not give the
		/// io_context some work to do then the io_context::run() function will exit
		/// immediately.
		boost::asio::executor_work_guard<boost::asio::io_context::executor_type> m_work;

		Handler m_handler;
	};

	/// Monitor operations asynchronously.
	template <typename Handler>
	void async_monitor(impl_type &impl, Handler handler)
	{
		m_work_io_context.post(monitor_operation<Handler>(impl, m_work_io_context, handler));
	}

private:
	/// Private io_context used for performing logging operations.
	boost::asio::io_context m_work_io_context;

	/// Work for the private io_context to perform. If we do not give the
	/// io_context some work to do then the io_context::run() function will exit
	/// immediately.
	boost::asio::executor_work_guard<boost::asio::io_context::executor_type> m_work;

	/// Thread used for running the work io_context's run loop.
	std::shared_ptr<std::thread> m_work_thread;
};

template <typename FileMonitorImplementation>
boost::asio::io_context::id path_monitor_service<FileMonitorImplementation>::id;

} // namespace services

#endif // SERVICES_PATH_MONITOR_SERVICE_HPP
