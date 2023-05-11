
#include <Eigen/Core>

namespace cg {

class Factor {
 public:
  Factor() = default; // 使用系统提供的默认构造函数

  Factor(const Factor &) = delete; // 防止该函数被再次调用，注意这个“再”字。此函数是个复制构造函数，加delete目的是防止被复制

  virtual Eigen::MatrixXd measurement_residual(const Eigen::MatrixXd &mat_x, const Eigen::MatrixXd &mat_z) = 0;

  virtual Eigen::MatrixXd measurement_jacobian(const Eigen::MatrixXd &mat_x, const Eigen::MatrixXd &mat_z) = 0;

  virtual void check_jacobian(const Eigen::MatrixXd &mat_x, const Eigen::MatrixXd &mat_z) = 0;

  virtual ~Factor() {}
};
using FactorPtr = std::shared_ptr<Factor>;

}  // namespace cg
