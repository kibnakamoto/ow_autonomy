// The Notices and Disclaimers for Ocean Worlds Autonomy Testbed for
// Exploration Research and Simulation can be found in README.md in
// the root directory of this repository.

// ow_plexil
#include "LanderAdapter.h"
#include "LanderInterface.h"
#include "adapter_support.h"
#include "subscriber.h"

// ROS
#include <ros/ros.h>
#include <geometry_msgs/Point.h>

// C++
#include <map>
using std::string;
using std::unique_ptr;

// PLEXIL API
#include <AdapterConfiguration.hh>
#include <AdapterFactory.hh>
#include <ArrayImpl.hh>
#include <Debug.hh>
#include <Expression.hh>
#include <StateCacheEntry.hh>
using namespace PLEXIL;

float LanderAdapter::PanMinDegrees  = -183.346; // -3.2 radians
float LanderAdapter::PanMaxDegrees  =  183.346; //  3.2 radians
float LanderAdapter::TiltMinDegrees = -89.38;   // Slightly more than -pi/2
float LanderAdapter::TiltMaxDegrees =  89.38;   // Slightly less than pi/2
float LanderAdapter::PanTiltInputTolerance =  0.0057; // 0.0001 R

static void arm_find_surface (Command* cmd, AdapterExecInterface* intf)
{
  int frame;
  bool relative;
  double distance, overdrive, force_threshold, torque_threshold;
  vector<double> const *position_vector = nullptr;
  vector<double> const *orientation_vector = nullptr;
  RealArray const *position = nullptr;
  RealArray const *orientation = nullptr;
  const vector<Value>& args = cmd->getArgValues();
  args[0].getValue(frame);
  args[1].getValue(relative);
  args[2].getValuePointer(position);
  args[3].getValuePointer(orientation);
  args[4].getValue(distance);
  args[5].getValue(overdrive);
  args[6].getValue(force_threshold);
  args[7].getValue(torque_threshold);
  position->getContentsVector(position_vector);
  orientation->getContentsVector(orientation_vector);
  unique_ptr<CommandRecord>& cr = new_command_record(cmd, intf);
  LanderAdapter::s_interface->armFindSurface (frame, relative,
                                              *position_vector, *orientation_vector,
                                              distance, overdrive,
                                              force_threshold, torque_threshold,
                                              g_cmd_id);
  acknowledge_command_sent(*cr);
}

static void arm_unstow (Command* cmd, AdapterExecInterface* intf)
{
  std::unique_ptr<CommandRecord>& cr = new_command_record(cmd, intf);
  LanderAdapter::s_interface->armUnstow (g_cmd_id);
  acknowledge_command_sent(*cr);
}

static void arm_stop (Command* cmd, AdapterExecInterface* intf)
{
  std::unique_ptr<CommandRecord>& cr = new_command_record(cmd, intf);
  LanderAdapter::s_interface->armStop (g_cmd_id);
  acknowledge_command_sent(*cr);
}

static void arm_stow (Command* cmd, AdapterExecInterface* intf)
{
  std::unique_ptr<CommandRecord>& cr = new_command_record(cmd, intf);
  LanderAdapter::s_interface->armStow (g_cmd_id);
  acknowledge_command_sent(*cr);
}

static void arm_move_cartesian (Command* cmd, AdapterExecInterface* intf)
{
  int frame;
  bool relative;
  vector<double> const *position_vector = nullptr;
  vector<double> const *orientation_vector = nullptr;
  RealArray const *position = nullptr;
  RealArray const *orientation = nullptr;
  // Get arguments.
  const vector<Value>& args = cmd->getArgValues();
  args[0].getValue(frame);
  args[1].getValue(relative);
  args[2].getValuePointer(position);
  args[3].getValuePointer(orientation);
  // Change real array into a vector.
  position->getContentsVector(position_vector);
  orientation->getContentsVector(orientation_vector);
  std::unique_ptr<CommandRecord>& cr = new_command_record(cmd, intf);
  LanderAdapter::s_interface->armMoveCartesian (frame, relative,
                                                *position_vector,
                                                *orientation_vector,
                                                g_cmd_id);
  acknowledge_command_sent(*cr);
}

static void arm_move_cartesian_guarded (Command* cmd,
                                        AdapterExecInterface* intf)
{
  int frame;
  bool relative;
  vector<double> const *position_vector = nullptr;
  vector<double> const *orientation_vector = nullptr;
  double force_threshold, torque_threshold;
  RealArray const *position = nullptr;
  RealArray const *orientation = nullptr;
  const vector<Value>& args = cmd->getArgValues();
  args[0].getValue(frame);
  args[1].getValue(relative);
  args[2].getValuePointer(position);
  args[3].getValuePointer(orientation);
  args[4].getValue(force_threshold);
  args[5].getValue(torque_threshold);

    //change real array into a vector
  position->getContentsVector(position_vector);
  orientation->getContentsVector(orientation_vector);
  std::unique_ptr<CommandRecord>& cr = new_command_record(cmd, intf);
  LanderAdapter::s_interface->armMoveCartesianGuarded (frame, relative,
                                                       *position_vector,
                                                       *orientation_vector,
                                                       force_threshold,
                                                       torque_threshold,
                                                       g_cmd_id);
  acknowledge_command_sent(*cr);
}

static void arm_move_joint (Command* cmd, AdapterExecInterface* intf)
{
  bool relative;
  int joint;
  double joint_pos_rad;
  const vector<Value>& args = cmd->getArgValues();
  args[0].getValue(relative);
  args[1].getValue(joint);
  args[2].getValue(joint_pos_rad);
  std::unique_ptr<CommandRecord>& cr = new_command_record(cmd, intf);
  LanderAdapter::s_interface->armMoveJoint (relative, joint, joint_pos_rad, g_cmd_id);
  acknowledge_command_sent(*cr);
}

static void task_deliver_sample (Command* cmd, AdapterExecInterface* intf)
{
  unique_ptr<CommandRecord>& cr = new_command_record(cmd, intf);
  LanderAdapter::s_interface->taskDeliverSample (g_cmd_id);
  acknowledge_command_sent(*cr);
}

static void task_discard_sample (Command* cmd, AdapterExecInterface* intf)
{
  int frame;
  bool relative;
  RealArray const* point = nullptr;
  vector<double> const* point_vector = nullptr;
  double height;

  const vector<Value>& args = cmd->getArgValues();
  args[0].getValue(frame);
  args[1].getValue(relative);
  args[2].getValuePointer(point);
  args[3].getValue(height);
  point->getContentsVector(point_vector);
  unique_ptr<CommandRecord>& cr = new_command_record(cmd, intf);
  LanderAdapter::s_interface->taskDiscardSample (frame, relative, *point_vector,
                                                 height, g_cmd_id);
  acknowledge_command_sent(*cr);
}

static void pan_tilt_move_joints (Command* cmd, AdapterExecInterface* intf)
{
  double pan_degrees, tilt_degrees;
  const vector<Value>& args = cmd->getArgValues();
  args[0].getValue (pan_degrees);
  args[1].getValue (tilt_degrees);
  if (! LanderAdapter::checkAngle ("pan", pan_degrees, LanderAdapter::PanMinDegrees,
                                   LanderAdapter::PanMaxDegrees,
                                   LanderAdapter::PanTiltInputTolerance) ||
      ! LanderAdapter::checkAngle ("tilt", tilt_degrees, LanderAdapter::TiltMinDegrees,
                                   LanderAdapter::TiltMaxDegrees,
                                   LanderAdapter::PanTiltInputTolerance)) {
    acknowledge_command_denied (cmd, intf);
  }
  else {
    unique_ptr<CommandRecord>& cr = new_command_record(cmd, intf);
    LanderAdapter::s_interface->panTiltMoveJoints (pan_degrees, tilt_degrees, g_cmd_id);
    acknowledge_command_sent(*cr);
  }
}

static void camera_capture (Command* cmd, AdapterExecInterface* intf)
{
  unique_ptr<CommandRecord>& cr = new_command_record(cmd, intf);
  LanderAdapter::s_interface->cameraCapture (g_cmd_id);
  acknowledge_command_sent(*cr);
}

static void arm_joint_acceleration (const State& state, StateCacheEntry &entry)
{
  const vector<PLEXIL::Value>& args = state.parameters();
  int joint;
  args[0].getValue(joint);
  entry.update(LanderAdapter::s_interface->getArmJointAcceleration(joint));
}

static void arm_joint_position (const State& state, StateCacheEntry &entry)
{
  const vector<PLEXIL::Value>& args = state.parameters();
  int joint;
  args[0].getValue(joint);
  entry.update(LanderAdapter::s_interface->getArmJointPosition(joint));
}

static void arm_joint_torque (const State& state, StateCacheEntry &entry)
{
  const vector<PLEXIL::Value>& args = state.parameters();
  int joint;
  args[0].getValue(joint);
  entry.update(LanderAdapter::s_interface->getArmJointTorque(joint));
}

static void arm_joint_velocity (const State& state, StateCacheEntry &entry)
{
  const vector<PLEXIL::Value>& args = state.parameters();
  int joint;
  args[0].getValue(joint);
  entry.update(LanderAdapter::s_interface->getArmJointVelocity(joint));
}

static void arm_pose (const State&, StateCacheEntry &entry)
{
  vector<Value> v;
  v.resize(7);
  vector<double> pose = LanderAdapter::s_interface->getArmPose();
  for (size_t i = 0; i < 7; i++) {
    // Conversion from double to PLEXIL::Real (float)
    v[i] = Value(static_cast<Real>(pose[i]));
  }
  entry.update(v);
}

static void arm_end_effector_ft (const State&, StateCacheEntry &entry)
{
  vector<Value> v;
  v.resize(6);
  vector<double> ft = LanderAdapter::s_interface->getArmEndEffectorFT();
  for (size_t i = 0; i < 6; i++) {
    // Conversion from double to PLEXIL::Real (float)
    v[i] = Value(static_cast<Real>(ft[i]));
  }
  entry.update(v);
}

static void pan_radians (const State& state, StateCacheEntry &entry)
{
  entry.update(LanderAdapter::s_interface->getPanRadians());
}

static void pan_degrees (const State& state, StateCacheEntry &entry)
{
  entry.update(LanderAdapter::s_interface->getPanRadians() * R2D);
}

static void tilt_radians (const State& state, StateCacheEntry &entry)
{
  entry.update(LanderAdapter::s_interface->getTiltRadians());
}

static void tilt_degrees (const State& state, StateCacheEntry &entry)
{
  entry.update(LanderAdapter::s_interface->getTiltRadians() * R2D);
}

static void battery_soc (const State& state, StateCacheEntry &entry)
{
  entry.update(LanderAdapter::s_interface->getBatterySOC());
}

static void battery_temp (const State& state, StateCacheEntry &entry)
{
  entry.update(LanderAdapter::s_interface->getBatteryTemperature());
}

static void battery_rul (const State& state, StateCacheEntry &entry)
{
  entry.update(LanderAdapter::s_interface->getBatteryRUL());
}

LanderInterface* LanderAdapter::s_interface = NULL;

LanderAdapter::LanderAdapter (AdapterExecInterface& execInterface,
                              const pugi::xml_node& configXml)
  : PlexilAdapter (execInterface, configXml)
{
  debugMsg("LanderAdapter", " created.");
}

bool LanderAdapter::initialize (LanderInterface* li)
{
  PlexilAdapter::initialize();
  s_interface = li;
  s_interface->setCommandStatusCallback (command_status_callback);

  // Commands

  g_configuration->registerCommandHandler("arm_find_surface", arm_find_surface);
  g_configuration->registerCommandHandler("arm_move_cartesian",
                                          arm_move_cartesian);
  g_configuration->registerCommandHandler("arm_move_cartesian_q",
                                          arm_move_cartesian);
  g_configuration->registerCommandHandler("arm_move_cartesian_guarded",
                                          arm_move_cartesian_guarded);
  g_configuration->registerCommandHandler("arm_move_cartesian_guarded_q",
                                          arm_move_cartesian_guarded);
  g_configuration->registerCommandHandler("arm_move_joint", arm_move_joint);
  g_configuration->registerCommandHandler("arm_stop", arm_stop);
  g_configuration->registerCommandHandler("arm_stow", arm_stow);
  g_configuration->registerCommandHandler("arm_unstow", arm_unstow);
  g_configuration->registerCommandHandler("camera_capture", camera_capture);
  g_configuration->registerCommandHandler("pan_tilt_move_joints",
                                          pan_tilt_move_joints);
  g_configuration->registerCommandHandler("task_deliver_sample",
                                          task_deliver_sample);
  g_configuration->registerCommandHandler("task_discard_sample",
                                          task_discard_sample);

  // Lookups
  g_configuration->registerLookupHandler("ArmJointAcceleration",
                                         arm_joint_acceleration);
  g_configuration->registerLookupHandler("ArmJointPosition",
                                         arm_joint_position);
  g_configuration->registerLookupHandler("ArmJointTorque",
                                         arm_joint_torque);
  g_configuration->registerLookupHandler("ArmJointVelocity",
                                         arm_joint_velocity);
  g_configuration->registerLookupHandler("ArmPose", arm_pose);
  g_configuration->registerLookupHandler("ArmEndEffectorForceTorque",
                                         arm_end_effector_ft);
  g_configuration->registerLookupHandler("PanRadians", pan_radians);
  g_configuration->registerLookupHandler("PanDegrees", pan_degrees);
  g_configuration->registerLookupHandler("TiltRadians", tilt_radians);
  g_configuration->registerLookupHandler("TiltDegrees", tilt_degrees);
  g_configuration->registerLookupHandler("BatteryStateOfCharge",
                                         battery_soc);
  g_configuration->registerLookupHandler("BatteryRemainingUsefulLife",
                                         battery_rul);
  g_configuration->registerLookupHandler("BatteryTemperature",
                                         battery_temp);

  debugMsg("LanderAdapter", " initialized.");
  return true;
}

bool LanderAdapter::checkAngle (const char* name, double val,
                                double min, double max, double tolerance)
// NOTE: tolerance is needed because there is a loss of precision in
// the angle somewhere between its encoding in PLEXIL and when it gets
// to this function.  Guessing a 32/64 bit conversion somewhere.
{
  if (val < min - tolerance || val > max + tolerance) {
    ROS_WARN ("Requested %s %f out of valid range [%f %f], "
              "rejecting PLEXIL command.", name, val, min, max);
    return false;
  }
  return true;
}

extern "C" {
  void initlander_adapter() {
    REGISTER_ADAPTER(LanderAdapter, "lander_adapter");
  }
}
