#pragma once

#include <vector>
#include <list>
#include "zip.h"

class epub
{
public:
	struct BookInfo
	{
		std::string& title;
		uint16_t cover;
		std::vector<std::string>& author;
		std::vector<std::string>& supporter;
		std::string& publisher;
		std::vector<std::pair<uint16_t, std::string>>& section;
	};

private:
	zip z;
	uint16_t pages;

	const BookInfo info;

	//const std::string title;
	//const std::string author;
	//const std::string publisher;
	//uint16_t cover;

	std::list<std::string> pics;

	void opf();
	void nav();
public:
	epub(std::ostream& stream, uint8_t c, const BookInfo info);
	//epub(std::ostream& stream, const uint8_t c, const char* title, const char* author, const char* publisher, const uint16_t cover);
	~epub();
	void addPage(const std::istream& in, const char* ext);
	void write();
};