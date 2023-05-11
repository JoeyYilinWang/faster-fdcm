#include <Eigen/Core>
#include <Eigen/Geometry>
#include <GeographicLib/LocalCartesian.hpp>
#include <memory>
#include "../fusion/observer.hpp"
#include "../common/state.hpp"

namespace cg {

constexpr int kMeasDim = 3;

// OpenStreetMap
struct OsmData
{
    double timestamp; // 时间戳
    Eigen::Vector3d lla;  // Latitude in degree, longitude in degree, and altitude in meter
    Eigen::Matrix3d cov;  // 协方差，但基于OpenStreetMap的重定位系统并没有该概念，为了保持一致性，可以使其为0矩阵
};
using OsmDataPtr = std::shared_ptr<OsmData>;
using OsmDataConstPtr = std::shared_ptr<const OsmData>;
class OSM : public Observer 
{
    public:
    OSM() = default;

    virtual ~OSM() {}

    
    void set_params(OsmDataConstPtr osm_data_ptr, const Eigen::Vector3d &I_p_Gps = Eigen::Vector3d::Zero()) 
    {
        // 提供的GPS初始定位信息
        init_lla_ = osm_data_ptr->lla;
        // 预测GPS定位信息
        I_p_Gps_ = I_p_Gps;
    }

/**
  * @brief compute measurement estimate based on transition matrix of state prediciton
  * @param mat_x transition matrix of state predicion 
  * @return state 
*/
virtual Eigen::MatrixXd measurement_function(const Eigen::MatrixXd &mat_x) 
{
    Eigen::Isometry3d Twb; // 变换矩阵
    Twb.matrix() = mat_x;
    return Twb * I_p_Gps_; 
}

/**
  * @brief compute measurement error between real measurement and measurement estimate
  * @param mat_x transition matrix of state prediction 
  * @param mat_z real measurement 
  * @return measurement residual 
*/
virtual Eigen::MatrixXd measurement_residual(const Eigen::MatrixXd &mat_x, const Eigen::MatrixXd &mat_z) 
{
    return mat_z - measurement_function(mat_x);
}

virtual Eigen::MatrixXd measurement_jacobian(const Eigen::MatrixXd &mat_x, const Eigen::MatrixXd &mat_z) 
{
    Eigen::Isometry3d Twb;
    Twb.matrix() = mat_x;

    Eigen::Matrix<double, kMeasDim, kStateDim> H; // H维度为[kMeasDim, kStateDim]，行数：kMeasDim, 列数：kStateDim，这里实际为3*15
    H.setZero();
    H.block<3, 3>(0, 0) = Eigen::Matrix3d::Identity();
    H.block<3, 3>(0, 6) = -Twb.linear() * Utils::skew_matrix(I_p_Gps_);

    return H;
}

virtual void check_jacobian(const Eigen::MatrixXd &mat_x, const Eigen::MatrixXd &mat_z) {}

/**
  * @brief global to local coordinate, convert WGS84 to ENU frame
  *
  * @param osm_data_ptr
  * @return Eigen::Vector3d
*/
Eigen::Vector3d g2l(OsmDataConstPtr osm_data_ptr) 
{
    Eigen::Vector3d p_G_Gps;
    OSM::lla2enu(init_lla_, osm_data_ptr->lla, &p_G_Gps);
    return p_G_Gps;
};

/**
  * @brief local to glocal coordinate, convert ENU to WGS84 lla
  *
  * @param p_wb
  * @return Eigen::Vector3d
*/
Eigen::Vector3d l2g(const Eigen::Vector3d &p_wb) 
{
    Eigen::Vector3d lla;
    OSM::enu2lla(init_lla_, p_wb, &lla);
    return lla;
}

static inline void lla2enu(const Eigen::Vector3d &init_lla,
                             const Eigen::Vector3d &point_lla,
                             Eigen::Vector3d *point_enu) 
{
    static GeographicLib::LocalCartesian local_cartesian;
    local_cartesian.Reset(init_lla(0), init_lla(1), init_lla(2)); // 以初始的坐标为原点
    local_cartesian.Forward(
        point_lla(0), point_lla(1), point_lla(2), point_enu->data()[0], point_enu->data()[1], point_enu->data()[2]);
}
  
static inline void enu2lla(const Eigen::Vector3d &init_lla,
                             const Eigen::Vector3d &point_enu,
                             Eigen::Vector3d *point_lla) 
{
    static GeographicLib::LocalCartesian local_cartesian;
    local_cartesian.Reset(init_lla(0), init_lla(1), init_lla(2));
    local_cartesian.Reverse(
        point_enu(0), point_enu(1), point_enu(2), point_lla->data()[0], point_lla->data()[1], point_lla->data()[2]);
}

   private:
    Eigen::Vector3d init_lla_;  // initial location in WGS64
    Eigen::Vector3d I_p_Gps_ = Eigen::Vector3d::Zero(); 
};
using OSMPtr = std::shared_ptr<OSM>;

}  // namespace cg
