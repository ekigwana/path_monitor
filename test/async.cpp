//
// Copyright (c) 2018 Edward Kigwana (ekigwana at gmail dot com)
// Copyright (c) 2008, 2009 Boris Schaeling <boris@highscore.de>
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "path_monitor/path_monitor.hpp"
#include "directory.hpp"

boost::asio::io_context io_context;

void create_file_handler(const std::system_error &se, const services::path_monitor_event &ev)
{
	EXPECT_EQ(se.code().value(), std::error_code().value());
	EXPECT_EQ(ev.parent_path, TEST_DIR1);
	EXPECT_EQ(ev.path, TEST_FILE1);
	EXPECT_EQ(static_cast<int>(ev.event), static_cast<int>(services::path_monitor_event::type::added));
}

// directory dir(TEST_DIR1);

TEST(TestASYNC, CreateFile)
{
	directory dir(TEST_DIR1);

	services::path_monitor pm(io_context, "Path Monitor");
	std::system_error se;
	pm.add_path(TEST_DIR1, se);

	EXPECT_EQ(se.code(), std::error_code());

	dir.create_file(TEST_FILE1);

	pm.async_monitor(create_file_handler);
	io_context.run();
	io_context.reset();
	std::this_thread::sleep_for(std::chrono::microseconds(1000));
}

void rename_file_handler_old(const std::system_error &se, const services::path_monitor_event &ev)
{
	EXPECT_EQ(se.code(), std::error_code());
	EXPECT_EQ(ev.parent_path, TEST_DIR1);
	EXPECT_EQ(ev.path, TEST_FILE1);
	EXPECT_EQ(static_cast<int>(ev.event), static_cast<int>(services::path_monitor_event::type::renamed_old_name));
}

void rename_file_handler_new(const std::system_error &se, const services::path_monitor_event &ev)
{
	EXPECT_EQ(se.code(), std::error_code());
	EXPECT_EQ(ev.parent_path, TEST_DIR1);
	EXPECT_EQ(ev.path, TEST_FILE2);
	EXPECT_EQ(static_cast<int>(ev.event), static_cast<int>(services::path_monitor_event::type::renamed_new_name));
}

void modify_file_handler(const std::system_error &se, const services::path_monitor_event &ev)
{
	EXPECT_EQ(se.code(), std::error_code());
	EXPECT_EQ(ev.parent_path, TEST_DIR1);
	EXPECT_EQ(ev.path, TEST_FILE2);
	EXPECT_EQ(static_cast<int>(ev.event), static_cast<int>(services::path_monitor_event::type::modified));
}

TEST(TestASYNC, RenameFile)
{
	directory dir(TEST_DIR1);
	dir.create_file(TEST_FILE1);

	services::path_monitor pm(io_context, "Path Monitor");
	std::system_error se;
	pm.add_path(TEST_DIR1, se);

	EXPECT_EQ(se.code(), std::error_code());

	dir.rename_file(TEST_FILE1, TEST_FILE2);

	pm.async_monitor(rename_file_handler_old);
	io_context.run();
	io_context.reset();

	pm.async_monitor(rename_file_handler_new);
	io_context.run();
	io_context.reset();

	#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
	pm.async_monitor(modify_file_handler);
	io_context.run();
	io_context.reset();
	#endif

	std::this_thread::sleep_for(std::chrono::microseconds(1000));
}

void remove_file_handler(const std::system_error &se, const services::path_monitor_event &ev)
{
	EXPECT_EQ(se.code(), std::error_code());
	EXPECT_EQ(ev.parent_path, TEST_DIR1);
	EXPECT_EQ(ev.path, TEST_FILE1);
	EXPECT_EQ(static_cast<int>(ev.event), static_cast<int>(services::path_monitor_event::type::removed));
}

TEST(TestASYNC, RemoveFile)
{
	directory dir(TEST_DIR1);
	dir.create_file(TEST_FILE1);

	services::path_monitor pm(io_context, "Path Monitor");
	std::system_error se;
	pm.add_path(TEST_DIR1, se);

	EXPECT_EQ(se.code(), std::error_code());

	dir.remove_file(TEST_FILE1);

	pm.async_monitor(remove_file_handler);
	io_context.run();
	io_context.reset();

	std::this_thread::sleep_for(std::chrono::microseconds(1000));
}

TEST(TestASYNC, MultipleEvents)
{
	directory dir(TEST_DIR1);

	services::path_monitor pm(io_context, "Path Monitor");
	std::system_error se;
	pm.add_path(TEST_DIR1, se);

	EXPECT_EQ(se.code(), std::error_code());

	dir.create_file(TEST_FILE1);
	dir.rename_file(TEST_FILE1, TEST_FILE2);

	pm.async_monitor(create_file_handler);
	io_context.run();
	io_context.reset();

	pm.async_monitor(rename_file_handler_old);
	io_context.run();
	io_context.reset();

	pm.async_monitor(rename_file_handler_new);
	io_context.run();
	io_context.reset();

	#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
	pm.async_monitor(modify_file_handler);
	io_context.run();
	io_context.reset();
	#endif

	std::this_thread::sleep_for(std::chrono::microseconds(1000));
}

void aborted_async_call_handler(const std::system_error &se, const services::path_monitor_event &)
{
	EXPECT_EQ(se.code().value(), static_cast<int>(std::errc::operation_canceled));
}

TEST(TestASYNC, AbortedAsyncCall)
{
	directory dir(TEST_DIR1);

	{
		services::path_monitor pm(io_context, "Path Monitor");
		std::system_error se;
		pm.add_path(TEST_DIR1, se);

		EXPECT_EQ(se.code(), std::error_code());

		pm.async_monitor(aborted_async_call_handler);
	}

	io_context.run();
	io_context.reset();
}

void blocked_async_call_handler_with_local_ioservice(const std::system_error &se, const services::path_monitor_event &)
{
	EXPECT_EQ(se.code().value(), static_cast<int>(std::errc::operation_canceled));
}

TEST(TestASYNC, BlockedAsyncCall)
{
	directory dir(TEST_DIR1);
	std::thread t;

	{
		boost::asio::io_context io_context;

		services::path_monitor pm(io_context, "Path Monitor");
		std::system_error se;
		pm.add_path(TEST_DIR1, se);

		EXPECT_EQ(se.code(), std::error_code());

		pm.async_monitor(blocked_async_call_handler_with_local_ioservice);

		// run() is invoked on another thread to make async_monitor() call a blocking function.
		// When pm and io_context go out of scope they should be destroyed properly without
		// a thread being blocked.
		t = std::thread(boost::bind(&boost::asio::io_context::run, boost::ref(io_context)));
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	t.join();
	io_context.reset();
}

void unregister_directory_handler(const std::system_error &se, const services::path_monitor_event &e)
{
	EXPECT_EQ(se.code().value(), static_cast<int>(std::errc::operation_canceled));
}

TEST(TestASYNC, UnregisterDirectory)
{
	directory dir(TEST_DIR1);
	std::thread t;

	{
		services::path_monitor pm(io_context, "Path Monitor");
		std::system_error se;
		pm.add_path(TEST_DIR1, se);

		EXPECT_EQ(se.code(), std::error_code());

		pm.remove_path(TEST_DIR1, se);

		EXPECT_EQ(se.code(), std::error_code());

		dir.create_file(TEST_FILE1);

		pm.async_monitor(unregister_directory_handler);

		// run() is invoked on another thread to make this test case return. Without using
		// another thread run() would block as the file was created after remove_path()
		// had been called.
		t = std::thread(boost::bind(&boost::asio::io_context::run, boost::ref(io_context)));
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	t.join();
	io_context.reset();
}

void two_dir_monitors_handler(const std::system_error &se, const services::path_monitor_event &)
{
	EXPECT_EQ(se.code().value(), static_cast<int>(std::errc::operation_canceled));
}

TEST(TestASYNC, TwoDirMonitors)
{
	directory dir1(TEST_DIR1);
	directory dir2(TEST_DIR2);
	std::thread t;

	{
		services::path_monitor pm1(io_context, "Path Monitor 1");
		std::system_error se;
		pm1.add_path(TEST_DIR1, se);

		EXPECT_EQ(se.code(), std::error_code());

		services::path_monitor pm2(io_context, "Path Monitor 2");
		pm2.add_path(TEST_DIR2, se);

		EXPECT_EQ(se.code(), std::error_code());

		dir2.create_file(TEST_FILE1);

		pm1.async_monitor(two_dir_monitors_handler);

		// run() is invoked on another thread to make this test case return. Without using
		// another thread run() would block as the directory the file was created in is
		// monitored by pm2 while async_monitor() was called for pm1.
		t = std::thread(boost::bind(&boost::asio::io_context::run, boost::ref(io_context)));
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	t.join();
	io_context.reset();
}
