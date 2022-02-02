#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <map>
#include <list>
//#include <stringapiset.h>
#include <Windows.h>
#include "epub.h"

using namespace std;
namespace fs = std::filesystem;

template<typename K, typename V>
auto map_find(map<K, list<V>>& m, K key)
{
	auto itr = m.find(key);
	if (itr == m.end())
	{
		m.emplace(key, list<V>());
		itr = m.find(key);
	}
	return itr;
}

void wstring2string(wchar_t* src, string &dist)
{
	uint16_t k = WideCharToMultiByte(CP_UTF8, 0, src, -1, NULL, 0, NULL, NULL);
	dist.resize(k);
	WideCharToMultiByte(CP_UTF8, 0, src, -1, const_cast<char*>(dist.c_str()), k, NULL, NULL);
	if (*dist.rbegin() == '\0')
		dist.resize(k - 1);
}

int wmain(int argc, wchar_t* argv[])
{
	setlocale(LC_ALL, "");

	map<string, list<string>> args;
	for (int i = 1; i < argc; )
	{
		string str;
		wstring2string(argv[i], str);
		auto itr = map_find<string, string>(args, str[0] == '-' ? str.substr(1) : "-");
		if (str[0] != '-')
			itr->second.push_back(str);
		for (i++; i < argc && argv[i][0] != L'-'; i++)
		{
			str = "";
			wstring2string(argv[i], str);
			itr->second.push_back(str);
		}
	}

	string& input = *args["-"].begin();
	string& output = *(++args["-"].begin());
	string& title = *args["t"].begin();
	vector<string> author = vector<string>(args["a"].begin(), args["a"].end());
	string& publisher = *args["p"].begin();
	vector<string> supporter = vector<string>();
	uint16_t cover = stoi(args["c"].begin()->c_str());
	uint16_t c = stoi(args["o"].begin()->c_str());

	vector<string> sv = vector<string>(args["sec"].begin(), args["sec"].end());
	vector<pair<uint16_t, string>> section;
	for (int i = 0; i < sv.size(); i += 2)
		section.push_back(make_pair(stoi(sv[i]), sv[i + 1]));

	ofstream ofs(output, ios_base::binary);
	epub::BookInfo info{ title, cover, author, supporter, publisher, section };
	epub e(ofs, c, info);

	fs::directory_iterator it(input);
	const fs::directory_iterator end;

	for (; it != end; it++)
		if (!it->is_directory())
		{
			//cout << it->path() << endl;
			ifstream ifs(it->path(), ios::binary);
			e.addPage(ifs, it->path().extension().string().c_str());
		}
	e.write();
	ofs.close();
}