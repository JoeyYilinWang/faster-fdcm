#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include <GeographicLib/LocalCartesian.hpp>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>


using namespace cv;

typedef std::vector<std::vector<double>> dataMat_NxN;

// Read Matrix element from .csv file
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

