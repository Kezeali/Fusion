// CountIncludes.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


namespace bfs = boost::filesystem;

struct header_name {};
struct hits {};

typedef boost::bimaps::bimap<
	boost::bimaps::unordered_set_of< boost::bimaps::tagged<std::string, header_name> >,
	boost::bimaps::multiset_of< boost::bimaps::tagged<size_t, hits>, std::greater<size_t> >
> CounterMap_t;


class Counter
{
public:
	Counter(const bfs::path& basepath_)
		: basepath(basepath_),
		expr("^\\s*Note:\\sincluding\\sfile:(\\s+)(.*)$", std::regex_constants::ECMAScript | std::regex_constants::optimize)
	{}

	bool compare_parent_path(const bfs::path& path)
	{
		for (auto bit = basepath.begin(), bend = basepath.end(), it = path.begin(), end = path.end(); bit != bend; ++bit, ++it)
		{
			if (it != end && *it != *bit)
				return false;
		}
		return true;
	}

	void check(const bfs::path& path)
	{
		std::array<char, 4096> buffer;

		std::ifstream inStream(path.generic_string());
		while (inStream && !inStream.eof())
		{
			inStream.getline(buffer.data(), buffer.size());
			if (inStream.gcount() > 0)
			{
				std::match_results<char*> results;

				if (std::regex_match(buffer.data(), buffer.data() + inStream.gcount(), results, expr))
				{
					if (results.length() >= 2)
					{
						const std::string whitespace = results.str(1);
						const bfs::path p(results.str(2));
						if (whitespace.length() == 1 || compare_parent_path(p))
						{
							auto r = counters.by<header_name>().insert(std::make_pair(p.filename().string(), 0));
							counters.by<header_name>().replace_data(r.first, r.first->get<hits>() + 1);
						}
					}
				}
			}
		}
	}

	void output(size_t num, const std::string& exclude)
	{
		size_t i = 0;
		const auto& results = counters.by<hits>();
		for (auto it = results.begin(), end = results.end(); it != end; ++it)
		{
			std::string name = it->get<header_name>();
			if (name != exclude)
			{
				std::cout << it->get<hits>() << "\t" << name << std::endl;

				if (++i >= num)
					break;
			}
		}
	}

	const bfs::path basepath;
	const std::regex expr;
	CounterMap_t counters;
};

int main(int argc, const char** argv)
{
	if (argc <= 1)
		return 1;

	std::string basepath(argv[1]);
	size_t outputNum;
	std::string exclude;

	if (argc >= 3)
		outputNum = (size_t)std::max(std::atol(argv[2]), 0l);
	else
		outputNum = 100;

	if (argc >= 4)
		exclude = argv[3];

	Counter counter(basepath);

	std::deque<bfs::path> pathStack;
	pathStack.push_back(basepath);
	do
	{
		const auto currentPath = pathStack.back();
		pathStack.pop_back();
		for (bfs::directory_iterator it(currentPath); it != bfs::directory_iterator(); it++)
		{
			if (bfs::is_directory(it->status()))
				pathStack.push_back(it->path());
			if (bfs::is_regular_file(it->status()))
			{
				const auto path = it->path();
				if (path.extension() == ".log")
				{
					counter.check(path);
				}
			}
		}
	} while (!pathStack.empty());

	counter.output(outputNum, exclude);

	return 0;
}

