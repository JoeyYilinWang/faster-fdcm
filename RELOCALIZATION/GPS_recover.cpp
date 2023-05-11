#include "FdcmMatch.h"
#include "Render.h"

int main(int argc, char* argv[])
{
    string singleTemplate;
    string queryImageLines;
    string queryImagePPM;

    if (argc != 4)
    {
        cout << "[Syntax] Matcher-UNIT template.pgm queryImageEdges.txt queryImagePPM.ppm";
        cout << "switch to default parameters";
        singleTemplate = "/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/templateImages/上海近崇明岛/PGMs/111.pgm";
        queryImageLines = "/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/queryImages/ShanghaiNetLines.txt";
        queryImagePPM = "/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/queryImages/ShanghaiNet.ppm";
    }
    else
    {
        singleTemplate = argv[1];
        queryImageLines = argv[2];
        queryImagePPM = argv[3];
    }
    vector<int> center_point;

    // 单个模板匹配，得到最佳匹配框，并输出匹配框的像素坐标
    MatchForSingleTemp(singleTemplate.c_str(), queryImageLines.c_str(), queryImagePPM.c_str(), center_point);
    cv::Point2d centerPoint;
    centerPoint.x = center_point[0];
    centerPoint.y = center_point[1];
    vector<double> origin_NED_GPS;
    origin_NED_GPS[0] = render::Nodes.nodes[0].x_coord;
    origin_NED_GPS[1] = render::Nodes.nodes[1].y_coord;
    origin_NED_GPS[2] = render::Nodes.nodes[2].z_coord;

    vector<double> point_GPS;
    pixel2GPS(centerPoint, render::render_height, render::intrisicMat, origin_NED_GPS, point_GPS);
    
/**
 * 下面这些应该是static全局变量。渲染阶段把这些变量暴露到外面，让main函数看到。
 * render_height 渲染高度
 * cam_K 相机内参
 * origin_NED_GPS NED坐标系原点的GPS坐标
 */

    
    
}