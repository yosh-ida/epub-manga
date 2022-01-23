#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include "epub.h"

using namespace std;
namespace fs = std::filesystem;

int main(int atgc, char* argv[])
{
	ofstream ofs("out.zip", ios_base::binary);
	epub e(ofs);

	fs::directory_iterator it("C:\\Users\\Haru\\Pictures\\book\\BokuNoHeroAcademia262");
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