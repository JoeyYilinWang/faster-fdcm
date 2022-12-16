#include "../Image/VideoCapture.h"

int main(int argc, char* argv[])
{
    // const char* filename = argv[1];
    // GetVideoInfo(filename);
    // GetFramesFromVideo(filename);
    // GetFramefromVideo();

    // const char* outPMG = "/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/tempImages/上海近崇明岛/PGMs/1.pgm";
    // const char* inPNG = "/home/joey/Pictures/Segmentation/1.png";
    // cvConvertoPNG2PGM(outPMG, inPNG);
    // PrintFilesNameOfDirectory("/home/joey/Pictures/Segmentation");


    const char* inFolder = "/home/joey/Pictures/Segmentation/";
    const char* outFolder = "/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/tempImages/上海近崇明岛/PGMs/";

    cvConvertoPNG2PGM_Batch(outFolder, inFolder);
    return 0;
}