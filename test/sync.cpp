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

TEST(TestSYNC, CreateFile)
{
	directory dir(TEST_DIR1);

	services::path_monitor pm(io_context, "Path Monitor");
	std::system_error se;
	pm.add_path(TEST_DIR1, se);

	EXPECT_EQ(se.code(), std::error_code());

	dir.create_file(TEST_FILE1);

	services::path_monitor_event ev = pm.monitor(se);

	EXPECT_EQ(se.code(), std::error_code());
	EXPECT_EQ(ev.parent_path, TEST_DIR1);
	EXPECT_EQ(ev.path, TEST_FILE1);
	EXPECT_EQ(static_cast<int>(ev.event), static_cast<int>(services::path_monitor_event::type::added));
}

TEST(TestSYNC, RenameFile)
{
	directory dir(TEST_DIR1);
	dir.create_file(TEST_FILE1);

	services::path_monitor pm(io_context, "Path Monitor");
	std::system_error se;
	pm.add_path(TEST_DIR1, se);

	EXPECT_EQ(se.code(), std::error_code());

	dir.rename_file(TEST_FILE1, TEST_FILE2);

	services::path_monitor_event ev = pm.monitor(se);

	EXPECT_EQ(se.code(), std::error_code());
	EXPECT_EQ(ev.parent_path, TEST_DIR1);
	EXPECT_EQ(ev.path, TEST_FILE1);
	EXPECT_EQ(static_cast<int>(ev.event), static_cast<int>(services::path_monitor_event::type::renamed_old_name));

	ev = pm.monitor(se);

	EXPECT_EQ(se.code(), std::error_code());
	EXPECT_EQ(ev.parent_path, TEST_DIR1);
	EXPECT_EQ(ev.path, TEST_FILE2);
	EXPECT_EQ(static_cast<int>(ev.event), static_cast<int>(services::path_monitor_event::type::renamed_new_name));

	#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
	ev = pm.monitor(se);
	EXPECT_EQ(ev.parent_path, TEST_DIR1);
	EXPECT_EQ(ev.path, TEST_FILE2);
	EXPECT_EQ(static_cast<int>(ev.event), static_cast<int>(services::path_monitor_event::type::modified));
	#endif
}

TEST(TestSYNC, RemoveFile)
{
	directory dir(TEST_DIR1);
	dir.create_file(TEST_FILE1);

	services::path_monitor pm(io_context, "Path Monitor");
	std::system_error se;
	pm.add_path(TEST_DIR1, se);

	EXPECT_EQ(se.code(), std::error_code());

	dir.remove_file(TEST_FILE1);

	services::path_monitor_event ev = pm.monitor(se);

	EXPECT_EQ(se.code(), std::error_code());
	EXPECT_EQ(ev.parent_path, TEST_DIR1);
	EXPECT_EQ(ev.path, TEST_FILE1);
	EXPECT_EQ(static_cast<int>(ev.event), static_cast<int>(services::path_monitor_event::type::removed));
}

TEST(TestSYNC, MultipleEvents)
{
	directory dir(TEST_DIR1);

	services::path_monitor pm(io_context, "Path Monitor");
	std::system_error se;
	pm.add_path(TEST_DIR1, se);

	EXPECT_EQ(se.code(), std::error_code());

	dir.create_file(TEST_FILE1);
	dir.rename_file(TEST_FILE1, TEST_FILE2);
	dir.remove_file(TEST_FILE2);

	services::path_monitor_event ev = pm.monitor(se);
	EXPECT_EQ(se.code(), std::error_code());
	EXPECT_EQ(ev.parent_path, TEST_DIR1);
	EXPECT_EQ(ev.path, TEST_FILE1);
	EXPECT_EQ(static_cast<int>(ev.event), static_cast<int>(services::path_monitor_event::type::added));

	ev = pm.monitor(se);
	EXPECT_EQ(se.code(), std::error_code());
	EXPECT_EQ(ev.parent_path, TEST_DIR1);
	EXPECT_EQ(ev.path, TEST_FILE1);
	EXPECT_EQ(static_cast<int>(ev.event), static_cast<int>(services::path_monitor_event::type::renamed_old_name));

	ev = pm.monitor(se);
	EXPECT_EQ(ev.parent_path, TEST_DIR1);
	EXPECT_EQ(ev.path, TEST_FILE2);
	EXPECT_EQ(static_cast<int>(ev.event), static_cast<int>(services::path_monitor_event::type::renamed_new_name));

	#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
	ev = pm.monitor(se);
	EXPECT_EQ(se.code(), std::error_code());
	EXPECT_EQ(ev.parent_path, TEST_DIR1);
	EXPECT_EQ(ev.path, TEST_FILE2);
	EXPECT_EQ(static_cast<int>(ev.event), static_cast<int>(services::path_monitor_event::type::modified));
	#endif

	ev = pm.monitor(se);
	EXPECT_EQ(se.code(), std::error_code());
	EXPECT_EQ(ev.parent_path, TEST_DIR1);
	EXPECT_EQ(ev.path, TEST_FILE2);
	EXPECT_EQ(static_cast<int>(ev.event), static_cast<int>(services::path_monitor_event::type::removed));
}

TEST(TestSYNC, DirMonitorDestruction)
{
	directory dir(TEST_DIR1);

	services::path_monitor pm(io_context, "Path Monitor");
	std::system_error se;
	pm.add_path(TEST_DIR1, se);

	EXPECT_EQ(se.code(), std::error_code());

	dir.create_file(TEST_FILE1);
}
