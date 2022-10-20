#include "CoordinatesTranform.h"

using namespace std;

/**
 * @brief draw points on grey image
 * @param binImage 
 */
void Draw(cv::Mat binImage, int w, int h, vector<cv::Point2d> points_P)
{
    binImage = Mat::zeros(cvSize(w,h),CV_8UC1);
    cv::drawContours(binImage, points_P, -1, cv::Scalar::all(255), CV_FILLED);
    cv::namedWindow("binImage", WINDOW_NORMAL);
    cv::imshow("binImage", binImage);
    cv::waitKey();
}


/**
 * @brief draw line according two points
 * @param img the image on which draw polygon
 * @param startPoint the start point of line
 * @param endPoint the end point of line
 */
void DrawLine(Mat img, cv::Point startPoint,cv::Point endPoint)
{
    cv::line(img, startPoint, endPoint, cv::Scalar(0,0,0), 1, 8);
}

/**
 * @brief draw polygon 
 * @param img the image on which draw polygon
 * @param polygonPoints the points of polygon
 * @param bIsFill bool label, decide if color fill or not
 * @param bIsClosed boo label, decide if polygon is closed or not
 */
void DrawPolygon(Mat img, cv::Point, vector<Point> polygonPoints, bool bIsFill, bool bIsClosed = true)
{
    vector<vector<Point>> contours;
    contours.push_back(polygonPoints);

    if (bIsFill)
        cv::fillPoly(img, contours, cv::Scalar(255,255,255), 8); // 255,255,255 is white color 
    else
        polylines(img, polygonPoints, bIsClosed, Scalar(255,255,255), 1, 8);
}

