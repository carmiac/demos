// Copyright 2016 Open Source Robotics Foundation, Inc.
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

#include <functional>
#include <memory>
#include <string>

#include "lifecycle_msgs/msg/transition_event.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "std_msgs/msg/string.hpp"

/// LifecycleListener class as a simple listener node
/**
 * We subscribe to two topics
 * - lifecycle_chatter: The data topic from the talker
 * - lc_talker__transition_event: The topic publishing
 *   notifications about state changes of the node
 *   lc_talker
 */
class LifecycleListener : public rclcpp_lifecycle::LifecycleNode
{
public:
  explicit LifecycleListener(const std::string & node_name)
  : LifecycleNode(node_name) {}

  rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn on_configure(
    const rclcpp_lifecycle::State &)
  {
    RCLCPP_INFO(get_logger(), "on_configure() is called.");
    // Data topic from the lc_talker node
    sub_data_ = this->create_subscription<std_msgs::msg::String>(
      "lifecycle_chatter", 10,
      std::bind(&LifecycleListener::data_callback, this, std::placeholders::_1));

    // Notification event topic. All state changes
    // are published here as TransitionEvents with
    // a start and goal state indicating the transition
    sub_notification_ = this->create_subscription<lifecycle_msgs::msg::TransitionEvent>(
      "/lc_talker/transition_event", 10,
      std::bind(&LifecycleListener::notification_callback, this, std::placeholders::_1));

    if (!sub_data_ || !sub_notification_) {
      RCLCPP_FATAL(get_logger(), "Could not create subscriber.");
      RCLCPP_FATAL(get_logger(), "sub_data_: %s", sub_data_ ? "true" : "false");
      RCLCPP_FATAL(get_logger(), "sub_notification_: %s", sub_notification_ ? "true" : "false");
      return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::ERROR;
    }
    return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::SUCCESS;
  }

  rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn on_shutdown(
    const rclcpp_lifecycle::State & state)
  {
    sub_data_.reset();
    sub_notification_.reset();
    LifecycleNode::on_shutdown(state);
    return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::SUCCESS;
  }

  void data_callback(std_msgs::msg::String::ConstSharedPtr msg)
  {
    RCLCPP_INFO(get_logger(), "data_callback: %s", msg->data.c_str());
  }

  void notification_callback(lifecycle_msgs::msg::TransitionEvent::ConstSharedPtr msg)
  {
    RCLCPP_INFO(
      get_logger(), "notify callback: Transition from state %s to %s",
      msg->start_state.label.c_str(), msg->goal_state.label.c_str());
  }

private:
  std::shared_ptr<rclcpp_lifecycle::LifecycleSubscription<std_msgs::msg::String>> sub_data_;
  std::shared_ptr<rclcpp_lifecycle::LifecycleSubscription<lifecycle_msgs::msg::TransitionEvent>>
  sub_notification_;
};

int main(int argc, char ** argv)
{
  // force flush of the stdout buffer.
  // this ensures a correct sync of all prints
  // even when executed simultaneously within the launch file.
  setvbuf(stdout, NULL, _IONBF, BUFSIZ);

  rclcpp::init(argc, argv);

  auto lc_listener = std::make_shared<LifecycleListener>("lc_node_listener");
  rclcpp::spin(lc_listener->get_node_base_interface());

  rclcpp::shutdown();

  return 0;
}
