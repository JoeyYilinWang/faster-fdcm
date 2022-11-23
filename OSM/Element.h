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

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <GeographicLib/LocalCartesian.hpp>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/calib3d.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/imgproc/imgproc_c.h>


#ifndef _ELEMENT_H
#define _ELEMENT_H
using namespace std;


struct node
{
    public:
    unsigned int node_id;
    unsigned int osm_node_id;
    double x_coord; // longtitude in degree
    double y_coord; // latitude in degree
    double z_coord = 0; // height in m
    double E_coord;
    double N_coord;
    double U_coord;
    double x_body;
    double y_body;
    double z_body;
    double x_cam;
    double y_cam;
    double z_cam;
    cv::Point2d point_P;
    cv::Vec3d point_NED;
    ~node();
};


class nodes
{
    public:
    vector<node> nodes;
    void ReadFromCSV(string str_csv_file, vector<vector<string>> &data);
    void Extract(vector<vector<string>> &data);  
    void LongLatHeight2ENU(vector<double> &Origin_ENU);
    void LongLatHeight2ENU();
    void ENU2Body(cv::Matx33f &Rotate);
    void ENU2NED();
    void Body2Cam(cv::Matx44f &T);
    void World2Img(const cv::Mat &intrinsicMat, const cv::Mat distCoeffs, const cv::Mat &rVec, const cv::Mat &tVec);
    void AppendZcoord(double &z);

    ~nodes();
};

struct link
{
    public:
    unsigned int link_id;
    unsigned int osm_way_id;
    unsigned int from_node_id;
    unsigned int to_node_id;
    double length;
    ~link();
};

class links
{
    public:
    vector<link> links;
    void ReadFromCSV(string str_csv_file, vector<vector<string>> & data);
    void Extract(vector<vector<string>> &data); 
    ~links();
};

void findLinkFromNodeCoord(link &Link, nodes &Nodes, pair<double, double> &coord);
void findLinkToNodeCoord(link &Link, nodes &Nodes, pair<double, double> &coord);
void AppendZcoord(nodes &Nodes, double &z);

#endif