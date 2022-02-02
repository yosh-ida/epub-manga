#include "epub.h"
#include "resource.h"
#include <windows.h>
#include <stdio.h>
#include <map>

using namespace std;

static const map<string, string> MIMES = { { ".jpeg", "image/jpeg" }, {".jpg", "image/jpeg"}, {".bmp", "image/bmp"}, {".svg", "image/svg+xml"}, {".gif", "image/gif"} };

struct membuf : streambuf
{
	membuf(char* begin, char* end)
	{
		this->setg(begin, begin, end);
	}
};

streambuf* res(int IDR_RODATA)
{
	// リソースデータのハンドル
	HRSRC hbin = FindResource(NULL, MAKEINTRESOURCE(IDR_RODATA), TEXT("RODATA"));
	// それをロード
	char* bindata = (char*)LoadResource(0, hbin);
	// それのサイズ
	DWORD binsize = SizeofResource(0, hbin);

	// FreeResource(hbin);
	membuf* buf = new membuf(bindata, bindata + binsize);
	return buf;
}

epub::epub(std::ostream& stream, uint8_t c, const epub::BookInfo info): z(stream, c), info(info)
{
	//  kobospan kobo.1.1

	//  必須要素を追加する
	streambuf* mimetype = res(IDR_RODATA_MIMETYPE);
	streambuf* container = res(IDR_RODATA_CONTAINER_XML);
	streambuf* style = res(IDR_RODATA_STYLE);
	z.add("mimetype", istream(mimetype));
	z.add("META-INF/container.xml", istream(container));
	z.add("item/style.css", istream(style));
	free(mimetype);
	free(container);
	free(style);
	pages = 0;
}

void epub::nav()
{
	string content = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
		"<!DOCTYPE html>\n"
		"<html xmlns=\"http://www.w3.org/1999/xhtml\" xmlns:epub=\"http://www.idpf.org/2007/ops\" xml:lang=\"ja\">\n"
		"<head>\n"
		"<meta charset=\"utf-8\" />\n"
		"<title>" + info.title + "</title>\n"
		"<style type=\"text/css\">\n"
		"ol{ list-style: none; }\n"
		"img.gaiji{ width: 1em; height: 1em; display: inline; }\n"
		"</style>\n"
		"</head>\n"
		"<body>\n"

		"<nav epub:type=\"toc\" id=\"toc\">\n"
		u8"<h1>目次</h1>\n"
		"<ol>\n"
		u8"<li><a href=\"xhtml/" + to_string(info.cover) + u8".xhtml\">表紙</a></li>\n"
		"</ol>\n"
		"</nav>\n"
		"</body>\n"
		"</html>\n";

	char* str = const_cast<char*>(content.c_str());
	streambuf* buf = new membuf(str, str + content.length());
	z.add("item/navigation-documents.xhtml", istream(buf));
	free(buf);
}

void epub::opf()
{
	string content = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
		"<package xmlns=\"http://www.idpf.org/2007/opf\" unique-identifier=\"unique-id\" version=\"3.0\" xml:lang=\"ja\" prefix=\"ebpaj: http://www.ebpaj.jp/\" >\n"
		"\t<metadata xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n"
		"\t\t<dc:title id=\"title01\">" + info.title + "</dc:title>\n"
		"\t\t<dc:creator id=\"creator01\">" + info.author[0] + "</dc:creator>\n"
		"\t\t<meta refines=\"#creator01\" property=\"role\" scheme=\"marc:relators\">aut</meta>\n"
		"\t\t<meta refines=\"#creator01\" property=\"display-seq\">1</meta>\n"
		"\t\t<dc:publisher id=\"publisher\">" + info.publisher + "</dc:publisher>\n"
		"\t\t<dc:language>ja</dc:language>\n"
		"\t\t<dc:identifier id=\"unique-id\">urn:uuid:498f69a9-649c-44f5-a4e3-7f26bd99ece4</dc:identifier>\n"
		"\t\t<meta name=\"book-type\" content=\"comic\" />\n"
		"\t\t<meta property=\"dcterms:modified\">2019-11-06T00:00:00Z</meta>\n"
		"\t\t<meta name=\"cover\" content=\"pic" + to_string(info.cover) + "\" />\n"
		"\t\t<dc:type>comic</dc:type>\n"
		"\t</metadata>\n"
		"\t<manifest>\n"

		"\t\t<item href=\"navigation-documents.xhtml\" id=\"nav\" media-type=\"application/xhtml+xml\" properties=\"nav\" />\n"
		//"\t\t<item id=\"ncx\" href=\"toc.ncx\" media-type=\"application/x-dtbncx+xml\" />\n"
		"\t\t<item media-type=\"text/css\" id=\"style-layout-jp\" href=\"style.css\" />\n";

	{
		int i = 0;
		for (auto path : pics)
		{
			string ext = path.substr(path.find_last_of('.'));
			auto type = MIMES.find(ext);
			if (type == MIMES.end())
				type = MIMES.begin();
			content += "\t\t<item href=\"" + path + "\" id=\"pic" + to_string(i) + "\" media-type=\"" + type->second + "\" />\n"
				"\t\t<item href=\"xhtml/" + to_string(i) + ".xhtml\" id=\"xhtml" + to_string(i) + "\"  properties=\"svg\" media-type=\"application/xhtml+xml\" />\n";
			i++;
		}
	}

	content += "\t</manifest>\n"
		"\t<spine page-progression-direction=\"rtl\">\n";
	for (int i = 0; i < pics.size(); i++)
		content += "\t\t<itemref idref=\"xhtml" + to_string(i) + "\" />\n";
	content += "\t</spine>\n"
		"\t<guide>\n"
		"\t\t<reference type=\"cover\" title=\"Cover\" href=\"xhtml/" + to_string(info.cover) + ".xhtml\" />\n"
		"\t</guide>\n"
		"</package>";

	char* str = const_cast<char*>(content.c_str());
	streambuf* buf = new membuf(str, str + content.length());
	z.add("item/standard.opf", istream(buf));
	free(buf);
}

epub::~epub()
{
}

void epub::addPage(const istream& in, const char* ext)
{
	string path = "image/" + to_string(pages) + ext;
	pics.push_back(path);
	path = "item/" + path;
	z.add(path.c_str(), in);

	string xhtml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<!DOCTYPE html>\n"
		"<html xmlns=\"http://www.w3.org/1999/xhtml\" xmlns:epub=\"http://www.idpf.org/2007/ops\" xml:lang=\"ja\">\n"
		"<head>\n"
		"<meta charset=\"UTF-8\"/>\n"
		"<title>name</title>\n"
		"<link rel=\"stylesheet\" type=\"text/css\" href=\"../style.css\"/>\n"
		"<meta name=\"viewport\" content=\"width=764, height=1200\"/>\n"
		"</head>\n"
		"<body>\n"
		"<div id=\"top\">\n"
		"<svg class=\"rightpage\" xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\"\n xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n width=\"100%\" height=\"100%\" viewBox=\"0 0 764 1200\">\n"
		"<image width=\"764\" height=\"1200\" xlink:href=\"../image/"
		+ to_string(pages) + ext + "\"/>\n"
		"</svg>\n"
		"</div>\n"
		"</body>\n"
		"</html>\n";
	path = "item/xhtml/" + to_string(pages++) + ".xhtml";
	char* str = const_cast<char*>(xhtml.c_str());
	streambuf* buf = new membuf(str, str + xhtml.length());
	z.add(path.c_str(), istream(buf));
	free(buf);
}

void epub::write()
{
	nav();
	opf();
	z.write();
}
