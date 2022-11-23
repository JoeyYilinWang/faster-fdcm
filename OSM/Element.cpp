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
 * @brief 将解析后的数据赋值填充给node对象
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
 * @param {vector<double> &} Origin_ENU
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
 * @brief 经纬度坐标转化为ENU坐标，为与之同名函数的重载版本
 */
void nodes::LongLatHeight2ENU()
{
    // geo origin init
    GeographicLib::LocalCartesian geo_converter;
    if (nodes.size() > 0)
    {
        // set ENU origin 
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
 * @brief ENU坐标转化为NED坐标
 */
void nodes::ENU2NED()
{
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

