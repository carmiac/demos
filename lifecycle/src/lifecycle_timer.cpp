// Copyright 2023 Elroy Air, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <chrono>

#include "rclcpp/rclcpp.hpp"

#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "rclcpp_lifecycle/lifecycle_timer.hpp"

using namespace std::chrono_literals;

/// Lifecycle node that logs a message periodically, but only when active.
class LifecycleTimerNode : public rclcpp_lifecycle::LifecycleNode
{
public:
  LifecycleTimerNode(
    const std::string & node_name = "lifecycle_timer_node",
    bool intra_process_comms = false)
  : rclcpp_lifecycle::LifecycleNode(node_name,
      rclcpp::NodeOptions().use_intra_process_comms(intra_process_comms))
  {}

  void timer_callback()
  {
    RCLCPP_INFO(get_logger(), "Hello, Lifecycle Timer!");
  }


  void wall_timer_callback()
  {
    RCLCPP_INFO(get_logger(), "Hello, Lifecycle Wall Timer!");
  }

  rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
  on_configure(const rclcpp_lifecycle::State &)
  {
    timer_ = this->create_lifecycle_timer(
      1s,
      std::bind(&LifecycleTimerNode::timer_callback, this));
    wall_timer_ = this->create_lifecycle_wall_timer(
      1s,
      std::bind(&LifecycleTimerNode::wall_timer_callback, this));
    return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::SUCCESS;
  }

  rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
  on_cleanup(const rclcpp_lifecycle::State &)
  {
    timer_.reset();
    wall_timer_.reset();
    return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::SUCCESS;
  }

  rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
  on_shutdown(const rclcpp_lifecycle::State &)
  {
    timer_.reset();
    wall_timer_.reset();
    return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::SUCCESS;
  }

private:
  std::shared_ptr<rclcpp::TimerBase> wall_timer_;
  std::shared_ptr<rclcpp::TimerBase> timer_;
};


int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<LifecycleTimerNode>();
  rclcpp::spin(node->get_node_base_interface());
  rclcpp::shutdown();
  return 0;
}