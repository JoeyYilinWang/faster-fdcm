#include <cstdlib>
#include <climits>
#include <cstring>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

int main()
{
    vector<string> paths;
    string templatesPgmFolder = "/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/tempImages/上海近崇明岛/LINEs";
	for (const auto & entry : fs::directory_iterator(templatesPgmFolder))
	{
		paths.push_back(entry.path());
	}

    int countForTemplates = paths.size();
    for (int i = 0; i < countForTemplates; i++)
    {
        stringstream ss;
        ss << "/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/tempImages/template_lists/template_list_" << i << ".txt";
        ofstream in;
        in.open(ss.str(), ios::trunc);
        in << "1" << endl;
        in << "/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/tempImages/上海近崇明岛/TXTs/" << i << "Lines.txt";
        in.close();
    }

    return 0;
}