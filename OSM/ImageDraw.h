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

#ifndef _IMAGEDRAW_H
#define _IMAGEDRAW_H

using namespace std;

/**
 * @brief draw contours based on points on grey image
 * @param binImage 
 * @param w the width of 
 */
void DrawContours(cv::Mat binImage, int w, int h, vector<cv::Point2d> points_P)
{
    binImage = cv::Mat::zeros(cvSize(w,h),CV_8UC1);
    cv::drawContours(binImage, points_P, -1, cv::Scalar::all(255), CV_FILLED);
    cv::namedWindow("binImage", cv::WINDOW_NORMAL);
    cv::imshow("binImage", binImage);
    cv::waitKey();
}


/**
 * @brief draw nodes on image
 * @param binImage grey image
 * @param w the width of image 
 * @param h the height of image
 * @param Nodes the nodes in OSM map
 */
void DrawNodes(cv::Mat &binImage, int w, int h, nodes &Nodes)
{
    // config the width and height of image
    binImage = cv::Mat::zeros(cvSize(w, h), CV_8UC1);
    for (int i = 0; i < Nodes.nodes.size(); i++)
    {
        cv::circle(binImage, Nodes.nodes[i].point_P, 1, cv::Scalar::all(255), CV_FILLED);
    }
}


/**
 * @brief draw line according two points
 * @param binImage the image on which draw polygon
 * @param Node1 the start point of line
 * @param Node2 the end point of line
 */
void DrawLineOfTwoNodes(cv::Mat &binImage, node &Node1, node &Node2)
{
    cv::line(binImage, Node1.point_P, Node2.point_P, cv::Scalar::all(255), 1, 8);
}


/**
 * @brief draw line based on link
 * @param binImage the image where line will be drown
 * @param Link the link between two nodes
 * @param Nodes the nodes of OSM
 */
void DrawLineOfLink(cv::Mat &binImage, link &Link, nodes &Nodes)
{
    node Node1, Node2;
    int Node1_id, Node2_id;
    Node1_id = Link.from_node_id;
    Node2_id = Link.to_node_id;

    if ((Node1_id < Nodes.nodes.size()) && (Node2_id < Nodes.nodes.size()))
    {
        Node1 = Nodes.nodes[Node1_id];
        Node2 = Nodes.nodes[Node2_id];
        DrawLineOfTwoNodes(binImage, Node1, Node2);
    }
}


/**
 * @brief draw lines based on links
 * @param binImage the image where lines will be drown
 * @param Links links which contains every link with two nodes connected
 * @param Nodes the nodes of OSM   
 */
void DrawLinesOfLinks(cv::Mat &binImage, links &Links, nodes &Nodes)
{
    int links_num = Links.links.size();
    for (int i = 0; i < links_num; i++)
    {
        DrawLineOfLink(binImage, Links.links[i], Nodes);
    }
}

void SaveImage(cv::Mat &binImage, const char* filename)
{
    cv::imwrite(filename, binImage);
}



#endif