/*
Copyright 2022, Yi-Lin Wang
All Rights Reserved 
Permission to use, copy, modify, and distribute this software and 
its documentation for any non-commercial purpose is hereby granted 
without fee, provided that the above copyright notice appear in 
all copies and that both that copyright notice and this permission 
notice appear in supporting documentation, and that the name of 
the author not be used in advertising or publicity pertaining to 
distribution of the software without specific, written prior 
permission. 
THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
ANY PARTICULAR PURPOSE. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR 
ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN 
AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING 
OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
*/

#include "Element.h"
using namespace std;

/**
 * @brief 从CSV文件读取元素存储到data vector中
 * @param str_csv_file CSV文件，保存着OSM相应元素信息
 * @param data {output, vector<vector<string>> &}保存着从CSV文件中解析的数据
 */
void nodes::ReadFromCSV(string str_csv_file, vector<vector<string>> & data)
{
    ifstream in_file(str_csv_file, std::ios::in);
    string std_line;
    while (getline(in_file, std_line))
    {
        std::stringstream ss(std_line);
        string data_each;
        vector<string> data_array;
        while (getline(ss, data_each, ','))
        {
            data_array.push_back(data_each.c_str());
        }
        data.push_back(data_array);
    }
}

/**
 * @brief 将解析后的数据赋值填充给node对象，注意data中的第一个节点可能不是子图区域的左下角
 * @param data {vector<vector<string>> &} 保存着从CSV文件中解析的数据
 */
void nodes::Extract(vector<vector<string>> & data)
{
    nodes.clear();
    // iterate from i=1, because row0 corresponds to Labels
    for (int i = 1; i < data.size(); i++)
    {
        node Node;
        Node.node_id = atof(data[i][1].c_str());
        Node.osm_node_id = atof(data[i][2].c_str());
        Node.x_coord = atof(data[i][9].c_str());
        Node.y_coord = atof(data[i][10].c_str());
        nodes.push_back(Node);
    }
}

/**
 * @brief 经纬度坐标转化为ENU坐标
 * @param Origin_ENU ENU坐标系原点的绝对GPS坐标
 */
void nodes::LongLatHeight2ENU(vector<double> &Origin_ENU)
{
    // geo origin init
    GeographicLib::LocalCartesian geo_converter;
    // set ENU origin
    geo_converter.Reset(Origin_ENU[1], Origin_ENU[0], Origin_ENU[2]);
    if (nodes.size() > 0)
    {
        for (int i = 0; i < nodes.size(); i++)
        {
            double local_E, local_N, local_U;
            // convert[lat, lon, hgt] to ENU
            geo_converter.Forward(nodes[i].y_coord, nodes[i].x_coord, nodes[i].z_coord, local_E, local_N, local_U);
            nodes[i].E_coord = local_E;
            nodes[i].N_coord = local_N;
            nodes[i].U_coord = local_U;
        }
    }    
}

/**
 * @brief 经纬度坐标转化为ENU坐标，为与之同名函数的重载版本，将nodes中第一个节点的GPS坐标作为ENU坐标系原点的全局GPS坐标，
 * 不过要注意的是nodes中第一个节点不一定是子图区域的左下角
 */
void nodes::LongLatHeight2ENU()
{
    // geo origin init
    GeographicLib::LocalCartesian geo_converter;
    if (nodes.size() > 0)
    {
        // 设置ENU坐标系原点，以第一个节点作为ENU坐标系的原点
        geo_converter.Reset(nodes[0].y_coord, nodes[0].x_coord, nodes[0].z_coord);
        for (int i = 0; i < nodes.size(); i++)
        {
            double local_E, local_N, local_U;
            // convert[lat, lon, hgt] to ENU
            geo_converter.Forward(nodes[i].y_coord, nodes[i].x_coord, nodes[i].z_coord, local_E, local_N, local_U);
            nodes[i].E_coord = local_E;
            nodes[i].N_coord = local_N;
            nodes[i].U_coord = local_U;
        }
    }    
}

/**
 * @brief ENU坐标转化为Body坐标
 * @param Rotate {cv::Matx33f &} ENU坐标到Body坐标系的旋转矩阵
 */
void nodes::ENU2Body(cv::Matx33f &Rotate)
{
    for (int i = 0; i < nodes.size(); i++)
    {
        cv::Vec3d pointENU = {nodes[i].E_coord, nodes[i].N_coord, nodes[i].U_coord};
        cv::Vec3d pointBody = Rotate * pointENU;
        nodes[i].x_body = pointBody[0];
        nodes[i].y_body = pointBody[1];
        nodes[i].z_body = pointBody[2];
    }
}

/**
 * @brief ENU坐标转化为NED坐标，作为ENU坐标系到Body坐标系的转换的中间结果
 */
void nodes::ENU2NED()
{   
    // ENU坐标系到NED坐标系的转换只有旋转变换并没有平移
    cv::Matx33f R(0, 1, 0, 1, 0, 0, 0, 0, -1);
    for (int i = 0; i < nodes.size(); i++)
    {
        cv::Vec3d pointENU = {nodes[i].E_coord, nodes[i].N_coord, nodes[i].U_coord};
        cv::Vec3d pointNED = R * pointENU;
        nodes[i].point_NED = pointNED;
    }
}

/**
 * @brief Body坐标转化为相机坐标系下坐标
 * @param T {cv::Matx44f &} Body坐标系到相机坐标系的变换矩阵
 * @attention UN-USED
 */
void nodes::Body2Cam(cv::Matx44f &T)
{
    for (int i = 0; i < nodes.size(); i++)
    {
        cv::Vec4d pointBody_HOMO = {nodes[i].x_body, nodes[i].y_body, nodes[i].z_body, 1};
        cv::Vec4d pointCam = T *  pointBody_HOMO;
        nodes[i].x_cam = pointCam[0];
        nodes[i].y_cam = pointCam[1];
        nodes[i].z_cam = pointCam[2];
    }
}

/**
 * @brief 世界坐标系坐标到像素坐标系坐标的转化
 * @param intrinsicMat {const cv::Mat &}相机内参
 * @param disCoeffs {const cv::Mat &}相机畸变参数
 * @param rVec {const cv::Mat &}世界坐标系到相机坐标系的旋转向量
 * @param tVec {const cv::Mat &}世界坐标系到相机坐标系的平移向量
 * @attention 这里将NED坐标系作为世界坐标系
 */
void nodes::World2Img(const cv::Mat &intrinsicMat, const cv::Mat &distCoeffs, const cv::Mat &rVec, const cv::Mat &tVec)
{
    vector<cv::Point3d> points_W;
    for (int i = 0; i < nodes.size(); i++)
    {
        // regard NED coordinates as World coordinates.
        cv::Point3d point{nodes[i].point_NED[0], nodes[i].point_NED[1], nodes[i].point_NED[2]};
        points_W.push_back(point);
    }
    vector<cv::Point2d> points_P;
    cv::projectPoints(points_W, rVec, tVec, intrinsicMat, distCoeffs, points_P);
    for (int i = 0; i < points_P.size(); i++)
    {
        nodes[i].point_P = points_P[i];
    }
}

/**
 * @brief 给nodes坐标加上z轴坐标，使其具有高度信息
 * @param z {double &} z轴坐标
 * @attention UN-USED
 */ 
void nodes::AppendZcoord(double &z)
{
    int size = nodes.size();
    for (int i = 0; i < size; i++)
    {
        nodes[i].z_coord = z;
    }
}


nodes::~nodes()
{
    nodes.clear();
}

node::~node()
{

}

link::~link()
{

}

/**
 * @brief 从CSV文件中读取数据
 * @param str_csv_file {std::string}CSV文件名
 * @param data {std::vector<vector<string>> &}保存从CSV文件中提取的数据
 */
void links::ReadFromCSV(std::string str_csv_file, std::vector<vector<string>> &data)
{
    ifstream in_file(str_csv_file, std::ios::in);    
    string std_line;
    while (getline(in_file, std_line))
    {
        std::stringstream ss(std_line);
        std::string data_each;
        std::vector<std::string> data_array;
        // read each element of line through splited signal ',', but How about continuous comma:
        while (getline(ss, data_each, ',')) 
        {
            // data_array must contains null elements
            data_array.push_back(data_each.c_str());
        }
        data.push_back(data_array);
    }
}

/**
 * @brief 将解析后的数据赋值填充给link对象
 * @param data {vector<vector<string>> &} 保存着从CSV文件中解析的数据
 */
void links::Extract(vector<vector<string>> &data)
{   
    links.clear();
    // iterate every link, throw away first row
    for (int i = 1; i < data.size(); i++)
    {   
        link Link;
        Link.link_id = atof(data[i][1].c_str());
        Link.osm_way_id = atof(data[i][2].c_str());
        Link.from_node_id = atof(data[i][3].c_str());
        Link.to_node_id = atof(data[i][4].c_str());
        Link.length = atof(data[i][6].c_str()); 
        links.push_back(Link);     
    }
}

links::~links()
{
    links.clear();
}

/**
 * @brief 根据link信息获得node起点信息
 * @param Link {link &}link对象
 * @param Nodes {nodes &}保存所有节点的对象
 * @param coord {output, pair<double, double> &}保存node起点的坐标信息
 */
void findLinkFromNodeCoord(link &Link, nodes &Nodes, pair<double, double> &coord)
{
    node Node = Nodes.nodes[Link.from_node_id];
    double x_coord = Node.x_coord;
    double y_coord = Node.y_coord;
    coord = make_pair(x_coord, y_coord);
}

/**
 * @brief 根据link信息获得node终点信息
 * @param Link {link &}link对象
 * @param Nodes {nodes &}保存所有节点的对象
 * @param coord {output, pair<double, double> &}保存node终点的坐标信息
 */
void findLinkToNodeCoord(link &Link, nodes &Nodes, pair<double, double> &coord)
{
    node Node = Nodes.nodes[Link.to_node_id];
    double x_coord = Node.x_coord;
    double y_coord = Node.y_coord;
    coord = make_pair(x_coord, y_coord);
}

/**
 * @brief NED坐标转化为WGS-84坐标 
 * @param point_NED ENU坐标系下坐标
 * @param origin_NED_GPS NED坐标原点的GPS坐标 
 * @param point_GPS point的GPS坐标
 */
void NED2LongLatHeight(cv::Point3d point_NED, std::vector<double> origin_NED_GPS, std::vector<double>& point_GPS) 
{
    GeographicLib::LocalCartesian geo_converter;
    cv::Matx33d R(0, 1, 0, 1, 0, 0, 0, 0, -1);
    cv::Point3d point_ENU = R.inv() * point_NED;
    geo_converter.Reset(origin_NED_GPS[0], origin_NED_GPS[1], origin_NED_GPS[2]); // ENU坐标原点与NED原点GPS坐标相同
    geo_converter.Reverse(point_ENU.x, point_ENU.y, point_ENU.z, point_GPS[0], point_GPS[1], point_GPS[2]);
}


/**
 * 航拍中心点的反投影变换：
 * 1. 在经过FDCM算法匹配后，匹配成功候选以匹配矩阵框来表示 
 * 2. 匹配框的尺寸其实就是长宽等比例缩放的航拍图像的尺寸（如果没有缩放，则匹配框的尺寸就是航拍图像的尺寸)。无论有没有缩放，匹配框的中心一定与航拍中心点一致。
 * 3. 根据匹配框提供的左上角像素坐标和尺寸，计算出匹配框中心点的像素坐标
 * 4. 根据匹配框中心点的像素坐标以及此时飞行高度，根据反投影公式计算出该点在NED上的三维坐标
 * 5. 根据该点的NED三维坐标计算出ENU三维坐标
 * 航拍中心店的GPS坐标恢复：
 * 1. 根据该点的ENU三维坐标，以及ENU坐标系原点的GPS坐标，即可计算出GPS坐标系下坐标
 */

/**
 * @brief 由匹配框像素中心点恢复NED坐标，该函数是个粗略不可靠版本，原因有两个： 
 * 1. NED坐标系原点可能由于分辨率限制的原因不被投影在像素平面上。
 * 2. 在成像过程中，像素的生成过程伴随着小数的取整，因此会损失距离信息。在飞行高度较高的条件下，这个可能是致命的。
 * @attention 不精确，且不可靠
 * @param center_pixel 匹配框中心点像素坐标
 * @param flight_height 飞行高度
 * @param scale 渲染时所对应的飞行高度倍数
 * @param cam_f 相机焦距
 * @param point_NED 中心点在NED坐标系下的坐标
 * @param origin_NED_pixel NED坐标系原点在像素平面的坐标（但有个问题要注意，如果像分辨率较小，那么NED坐标系原点可能无法被投影到该像平面中，超出临界了）
 */
void pixel2NED(cv::Point2d center_pixel, float flight_height, int scale, double cam_f, cv::Point3d& point_NED, cv::Point2d origin_NED_pixel)
{
    // 计算像素比
    double distance_u = abs(center_pixel.x - origin_NED_pixel.x);
    double distance_v = abs(center_pixel.y - origin_NED_pixel.y); 
    double ratio = cam_f / (flight_height * scale);
    double distance_E_NED = distance_u / ratio;
    double distance_N_NED = distance_v / ratio;
    point_NED.x = distance_E_NED;
    point_NED.y = distance_N_NED;
    point_NED.z = 0; 
}

/**
 * @brief 根据像素中心点恢复NED坐标
 * @param center_pixel 匹配框中心点像素坐标
 * @param render_height 渲染高度
 * @param cam_K 相机内参
 * @param point_NED 中心点在NED坐标系下的坐标
 */
void pixel2NED(cv::Point2d center_pixel, float render_height, cv::Matx33d cam_K, cv::Point3d& point_NED)
{
    cv::Point3d pixel_HOMO;
    pixel_HOMO.x = center_pixel.x;
    pixel_HOMO.y = center_pixel.y;
    pixel_HOMO.z = 1;

    cv::Point3d point_CAM = cam_K.inv() * (render_height * pixel_HOMO);
    
    // 因为匹配结果显示在渲染图像的，而渲染图像则以NED坐标系为世界坐标系渲染，NED坐标系与用于渲染图像的相机位姿只差一个垂直高度上的平移
    point_NED.x = point_CAM.x;
    point_NED.y = point_CAM.y;
    point_NED.z = point_CAM.z - render_height;
}


/**
 * @brief 根据匹配像素中心恢复GPS坐标
 * @param center_pixel 匹配框中心点像素坐标
 * @param render_height 渲染高度
 * @param cam_K 相机内参矩阵
 * @param origin_NED_GPS NED坐标系原点的GPS坐标
 * @param point_GPS 匹配中心的GPS坐标
 */
void pixel2GPS(cv::Point2d center_pixel, float render_height, cv::Matx33d cam_K, std::vector<double> origin_NED_GPS, std::vector<double>& point_GPS)
{
    cv::Point3d point_NED;
    pixel2NED(center_pixel, render_height, cam_K, point_NED);
    NED2LongLatHeight(point_NED, origin_NED_GPS, point_GPS);
}
