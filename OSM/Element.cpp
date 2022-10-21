#include "Element.h"
using namespace std;

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

void nodes::World2Img(const cv::Mat &intrinsicMat, const cv::Mat distCoeffs, const cv::Mat &rVec, const cv::Mat &tVec)
{
    vector<cv::Point3d> points_W;
    for (int i = 0; i < nodes.size(); i++)
    {
        cv::Point3d point{nodes[i].E_coord, nodes[i].N_coord, nodes[i].U_coord};
        points_W.push_back(point);
    }
    vector<cv::Point2d> points_P;
    cv::projectPoints(points_W, rVec, tVec, intrinsicMat, distCoeffs, points_P);
    for (int i = 0; i < points_P.size(); i++)
    {
        nodes[i].point_P = points_P[i];
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

void findLinkFromNodeCoord(link &Link, nodes &Nodes, pair<double, double> &coord)
{
    node Node = Nodes.nodes[Link.from_node_id];
    double x_coord = Node.x_coord;
    double y_coord = Node.y_coord;
    coord = make_pair(x_coord, y_coord);
}

void findLinkToNodeCoord(link &Link, nodes &Nodes, pair<double, double> &coord)
{
    node Node = Nodes.nodes[Link.to_node_id];
    double x_coord = Node.x_coord;
    double y_coord = Node.y_coord;
    coord = make_pair(x_coord, y_coord);
}

void AppendZcoord(nodes &Nodes, double &z)
{
    int size = Nodes.nodes.size();
    for (int i = 0; i < size; i++)
    {
        Nodes.nodes[i].z_coord = z;
    }
}
