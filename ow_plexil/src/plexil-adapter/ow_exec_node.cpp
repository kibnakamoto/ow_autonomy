// The Notices and Disclaimers for Ocean Worlds Autonomy Testbed for Exploration
// Research and Simulation can be found in README.md in the root directory of
// this repository.

// ROS
#include <ros/ros.h>
#include <ros/package.h>

#include "PlexilPlanSelection.h"
#include "OwExecutive.h"
#include "OwInterface.h"

int main(int argc, char* argv[])
{
  // Initializations
  ros::init(argc, argv, "ow_exec_node");
  std::string initial_plan = "None";

  //checking if there is a plan given
  if(argc == 2 && std::string(argv[1]).compare("None") != 0) {
    std::string plan(argv[1]);
    initial_plan = plan;
  }
  if (! OwExecutive::instance()->initialize ("ow-config.xml")) {
    ROS_ERROR("Could not initialize Plexil executive, shutting down.");
    return 1;
  }
  OwInterface::instance()->initialize();
  PlexilPlanSelection plan_selection;
  plan_selection.initialize(initial_plan); //initialize pubs, subs, etc
  plan_selection.start(); //begin control loop
  ros::spin();
  return 0;  // We never actually get here!
}
