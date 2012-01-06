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

	Counter(const Counter& other)
		: basepath(other.basepath),
		expr(other.expr),
		counters(other.counters),
		directly_included(other.directly_included)
	{}

	Counter(Counter&& other)
		: basepath(std::move(other.basepath)),
		expr(std::move(other.expr)),
		counters(std::move(other.counters)),
		directly_included(std::move(other.directly_included))
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

		auto& headerNames = counters.by<header_name>();

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
						const auto filename = p.filename().string();
						// If this is a directly included file (whitespace == 1), or it is a header from the project basepath:
						if (whitespace.length() == 1 || compare_parent_path(p))
						{
							directly_included.insert(filename);
						}
						auto _where = headerNames.insert(std::make_pair(std::move(filename), 0)).first;
						headerNames.replace_data(_where, _where->get<hits>() + 1);
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
			if (name != exclude && directly_included.find(name) != directly_included.end())
			{
				std::cout << it->get<hits>() << "\t" << name << "\n";

				if (++i >= num)
					break;
			}
		}
	}

	const bfs::path basepath;
	const std::regex expr;
	CounterMap_t counters;
	std::unordered_set<std::string> directly_included;
};

int main(int argc, const char** argv)
{
	if (argc <= 1)
		return 1;

	const std::string basepath(argv[1]);
	size_t outputNum;
	std::string exclude;
	enum OutputMode : unsigned int { AllInOne = 1, GroupSubdirs = 2 };
	unsigned int outputMode = AllInOne;

	if (argc >= 3)
		outputNum = (size_t)std::max(std::atol(argv[2]), 0l);
	else
		outputNum = 100;

	if (argc >= 4)
		exclude = argv[3];

	if (argc >= 5)
	{
		const std::string modeArg(argv[4]);
		if (modeArg == "aio")
			outputMode = AllInOne;
		else if (modeArg == "subdirs")
			outputMode = GroupSubdirs;
		else if (modeArg == "e")
			outputMode = AllInOne | GroupSubdirs;
	}

	bfs::path absBasePath(basepath);
	if (!absBasePath.is_absolute())
	{
		bfs::path workingDir(argv[0]);
		if (workingDir.is_absolute())
			workingDir.remove_filename();
		else
			workingDir = bfs::current_path();
		const auto relativePath = absBasePath;
		absBasePath = workingDir;
		for (auto it = relativePath.begin(), end = relativePath.end(); it != end; ++it)
		{
			if (it->string() == "..")
			{
				absBasePath = absBasePath.parent_path();
			}
			else if (it->string() != ".")
				absBasePath /= *it;
		}
	}

	Counter mainCounter(absBasePath);
	std::map<std::string, Counter> subdirCounters;

	std::deque<bfs::path> pathStack;
	pathStack.push_back(absBasePath);
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
					if (outputMode & AllInOne)
						mainCounter.check(path);
					if (outputMode & GroupSubdirs)
					{
						Counter& subdirCounter = subdirCounters.insert(std::make_pair(currentPath.string(), Counter(absBasePath))).first->second;
						subdirCounter.check(path);
					}
				}
			}
		}
	} while (!pathStack.empty());

	if (outputMode & AllInOne)
		mainCounter.output(outputNum, exclude);
	if (outputMode & GroupSubdirs)
	{
		for (auto it = subdirCounters.begin(), end = subdirCounters.end(); it != end; ++it)
		{
			std::cout << "---------------------\nProject build in: " << it->first << "\n";
			it->second.output(outputNum, exclude);
			std::cout << std::endl;
		}
	}

	return 0;
}

