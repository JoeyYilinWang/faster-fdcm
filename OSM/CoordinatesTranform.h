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
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include <GeographicLib/LocalCartesian.hpp>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>

#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/calib3d.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/imgproc/imgproc_c.h>

using namespace cv;

// 用于存储经纬坐标的矩阵，用vector来表示
typedef std::vector<std::vector<double>> dataMat_NxN;

// Read Matrix element from .csv file
// 要注意的是CSV文件中第一个节点不一定是OSM子图左上角的节点
void ReadCSV(std::string str_csv_file, dataMat_NxN &data_NxN)
{   
    std::ifstream in_file(str_csv_file, std::ios::in);
    std::string std_line;
    // iterate each line of .csv file through \n
    while (getline(in_file, std_line))
    {
        std::stringstream ss(std_line);
        std::string data_each;
        std::vector<double> data_array;
        // read each element of line through splited signal ','
        while (getline(ss, data_each, ',')) 
        {
            // function atof convert string to double  
            data_array.push_back(atof(data_each.c_str()));
        }
        data_NxN.push_back(data_array);
    }
}

// Write Matrix element to a .csv file
void WriteCSV(std::string str_csv_file, dataMat_NxN &data_NxN)
{
    std::ofstream out_file(str_csv_file, std::ios::out);
    for (int i = 0; i < data_NxN.size(); ++i)
    {
        for (int j = 0; j < data_NxN[i].size(); ++j)
        {
            out_file << data_NxN[i][j]<<',';
        }
        out_file << "\n";
    }
}

/**
 * @brief convert longtitude, latitude and height to ENU coordinate
 * @param csv_file_read the coordinates of points in WGS-84 frame saved in .csv file
 * @param csv_file_write the coordinates of points in ENU frame saved in .csv file
 * @param Origin_ENU the coodinates in WGS-84 frame of ENU frame origin  
 */
void LongLatHeight2ENU(std::string csv_file_read, std::string csv_file_write, std::vector<double> Origin_ENU)
{
    // geo origin init
    GeographicLib::LocalCartesian geo_converter;
    // data structure define
    dataMat_NxN geo_data;
    dataMat_NxN ENU_data;
    // read data from .csv file
    std::string str_geo_data = csv_file_read;
    ReadCSV(str_geo_data, geo_data);
    if (geo_data.size() > 0)
    {
        if (geo_data[0].size() == 3)
        {
            // set ENU origin
            geo_converter.Reset(Origin_ENU[0], Origin_ENU[1], Origin_ENU[2]);
            for (int i = 0; i < geo_data.size(); i++)
            {
                double local_E, local_N, local_U;
                // convert[lat, lon, hgt] to ENU
                geo_converter.Forward(geo_data[i][0], geo_data[i][1], geo_data[i][2], local_E, local_N, local_U);
                std::vector<double> data_each;
                data_each.emplace_back(local_E);
                data_each.emplace_back(local_N);
                data_each.emplace_back(local_U);
                ENU_data.emplace_back(data_each);
            }
        }
    }
    WriteCSV(csv_file_write, ENU_data);
}


/**
 * @brief transform coordinates of point in ENU frame to the coordinates in body frame
 * @param M_Rotate the rotation matrix between ENU frame and body frame, bacause the relation between ENU and body frame 
 *                 only related to rotation without translation
 * @param point_in_ENU the coordinates of point in ENU frame
 * @param point_in_body the coordinates of point in body frame
 */
void ENU2Body(Eigen::Matrix<float,3,3> M_Rotate, Eigen::Vector3d & point_in_ENU, Eigen:Vector3d & point_in_body)
{
    point_in_body = Rotate * point_in_ENU;
}


/**
 * @brief transform coordinates of point in body frame to the coordinates in camera frame
 * @param M_transform the transformation matrix between body frame and camera frame
 * @param point_in_body the coordinates of point in body frame
 * @param point_in_cam the coordinates of point in camera frame
 */
void Body2Cam(Eigen::Matrix<float,4,4> M_transform, Eigen::Vector4d & point_in_body Eigen::Vector4d & point_in_cam)
{
    point_in_cam = M_transform * point_in_body;
} 

/**
 * @brief project point in camera frame to pixel frame, directly using function in OpenCV
 * @param points_W the coordinates of points in world frame
 * @param points_P the coordinates of points in image frame
 * @param intrisicMat the matrix of intrisic parameter of camera
 * @param distCoeffs the distortion parameters of lens in camera
 * @param rVec rotation vector 
 * @param tVec translation vector
 */
void World2Pixel(const vector<cv::Point3d> & points_W, vector<cv::Point2d> &points_P, const cv::Mat & intrisicMat, const cv::Mat & distCoeffs, const cv::Mat & rVec, 
                const cv::Mat & tVec)
{
    cv::projectPoints(points_W, rVec, tVec, intrisicMat, distCoeffs, points_P);
}



