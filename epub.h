#pragma once

#include <list>
#include "zip.h"

class epub
{
	zip z;
	uint16_t pages;

	const std::string title;
	const std::string author;
	const std::string publisher;
	uint16_t cover;

	std::list<std::string> pics;

	void opf();
	void nav();
public:
	epub(std::ostream& stream);
	~epub();
	void addPage(const std::istream& in, const char* ext);
	void write();
};