//
// Copyright (c) 2018 Edward Kigwana (ekigwana at gmail dot com)
// Copyright (c) 2008, 2009 Boris Schaeling <boris@highscore.de>
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef directory_A004F6FF_8109_4a0e_970D_3FA0ECE1F2FF
#define directory_A004F6FF_8109_4a0e_970D_3FA0ECE1F2FF

#include <filesystem>
#include <fstream>
#include <thread>
#include <gtest/gtest.h>

#define TEST_DIR1 "A95A7AE9-D5F5-459a-AB8D-28649FB1F3F4"
#define TEST_DIR2 "EA63DF88-7BFF-4038-B317-F37434DF4ED1"
#define TEST_FILE1 "test1.txt"
#define TEST_FILE2 "test2.txt"

class directory
{
public:
	directory(std::string name)
	: m_name(std::filesystem::absolute(name))
	{
		std::filesystem::create_directory(m_name);
		EXPECT_TRUE(std::filesystem::is_directory(m_name));
	}

	~directory()
	{
		std::filesystem::remove_all(m_name);
	}

	void create_file(std::string file)
	{
		auto current_path = std::filesystem::current_path();

		std::filesystem::current_path(m_name);
		ASSERT_TRUE(std::filesystem::equivalent(m_name, std::filesystem::current_path()));
		std::ofstream ofs(file);
		ASSERT_TRUE(std::filesystem::exists(file));
		std::filesystem::current_path(current_path);
	}

	void rename_file(std::string from, std::string to)
	{
		auto current_path = std::filesystem::current_path();

		std::filesystem::current_path(m_name);
		ASSERT_TRUE(std::filesystem::equivalent(m_name, std::filesystem::current_path()));
		ASSERT_TRUE(std::filesystem::exists(from));
		std::filesystem::rename(from, to);
		ASSERT_TRUE(std::filesystem::exists(to));
		std::filesystem::current_path(current_path);
	}

	void remove_file(std::string file)
	{
		auto current_path = std::filesystem::current_path();

		std::filesystem::current_path(m_name);
		ASSERT_TRUE(std::filesystem::equivalent(m_name, std::filesystem::current_path()));
		ASSERT_TRUE(std::filesystem::exists(file));
		std::filesystem::remove(file);
		std::filesystem::current_path(current_path);
	}

private:
	std::filesystem::path m_name;
};

#endif
