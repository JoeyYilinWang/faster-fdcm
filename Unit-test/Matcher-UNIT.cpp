#include "../Image/Image.h"
#include "../Image/ImageIO.h"
#include <filesystem>
#include "../LineFit/LineFitter.h"
#include "../Fdcm/LMLineMatcher.h"

#include <iostream>
#include <string>
using namespace std;
namespace fs = std::filesystem;

int MatchForSingleTemp(const char* tempName, const char* queryEdgeName, const char* renderImageName);
int MatchForTempBatch(const char* templatesFolderName, const char* queryEdgeName, const char* renderImageName);

int main(int argc, char* argv[])
{
    string templateName;
    string queryEdgeMapName;
    string queryImageName;

    if (argc != 4)
    {
        cout << "[Syntax] Matcher-UNIT templateName.txt queryEdgeMapName.pgm queryImageName.ppm";
        cout << "switch to default parameters";
        templateName = "/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/tempImages/template_list.txt";
        queryEdgeMapName = "/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/queryImages/ShanghaiNetlines.pgm";
        queryImageName = "/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/queryImages/ShanghaiNet.ppm";
        
    }
    else
    {
        templateName = argv[1];
        queryEdgeMapName = argv[2];
        queryImageName = argv[3];
    }
    
    // MatchForSingleTemp(templateName.c_str(), queryEdgeMapName.c_str(), queryImageName.c_str());
    const char* queryLinesTXT = "/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/queryImages/ShanghaiNet.txt";

    const char* templatesFolderName = "/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/tempImages/template_lists";
    MatchForTempBatch(templatesFolderName, queryLinesTXT, queryImageName.c_str());

}


int MatchForSingleTemp(const char* tempName, const char* queryEdgeTXTName, const char* renderImageName)
{
    string templateName, queryEdgeMapTXTName, queryImageName;
    templateName = tempName;
    queryEdgeMapTXTName = queryEdgeTXTName;
    queryImageName = renderImageName;

    LFLineFitter lf;

    LMLineMatcher lm;

    lf.Configure("/home/joey/Projects/faster-fdcm/Unit-test/Config/LFlineFitterConfig.txt");
    lm.Configure("/home/joey/Projects/faster-fdcm/Unit-test/Config/LMlineMatcherConfig.txt");

    Image<uchar> *inputImage = ImageIO::LoadPGM(queryEdgeMapTXTName.c_str());
    lf.Init();
    // lf.FitLine(inputImage);

    lf.LoadEdgeMap(queryEdgeMapTXTName.c_str());
    lm.Init(templateName.c_str());

    vector<LMDetWind> detWind;
	lm.Match(lf,detWind);
    cout << "The Num of Matched successfully windows is: " << detWind.size() << endl;

    if (queryImageName.c_str())
    {
        Image<RGBMap> *debugImage = ImageIO::LoadPPM(queryImageName.c_str());
        LMDisplay::DrawDetWind(debugImage,detWind[0].x_,detWind[0].y_,detWind[0].width_,detWind[0].height_,RGBMap(255,0,0),4);
        char outputname[256];
        sprintf(outputname,"%s.output.ppm",queryImageName.c_str());
        ImageIO::SavePPM(debugImage,outputname);
        delete debugImage;
    }
    
    delete inputImage;

    return 0;
}



int MatchForTempBatch(const char* templatesFolderName, const char* queryEdgeTXTName, const char* renderImageName)
{
    vector<string> paths;
	for (const auto & entry : fs::directory_iterator(templatesFolderName))
	{
		paths.push_back(entry.path());
	}
    for (auto i:paths)
    {
        MatchForSingleTemp(i.c_str(), queryEdgeTXTName, renderImageName);
    }

    return 0;
}