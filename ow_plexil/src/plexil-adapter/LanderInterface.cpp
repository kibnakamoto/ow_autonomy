// The Notices and Disclaimers for Ocean Worlds Autonomy Testbed for
// Exploration Research and Simulation can be found in README.md in
// the root directory of this repository.

// ow_plexil
#include "LanderInterface.h"
#include "subscriber.h"

// PLEXIL
#include <ArrayImpl.hh>

// ROS
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>

// C++
#include <thread>
#include <vector>
#include <functional>

using namespace owl_msgs;
using namespace PLEXIL;

using std::hash;
using std::copy;
using std::string;
using std::thread;
using std::string;
using std::vector;
using std::make_unique;

const string Name_ArmMoveCartesian        = "ArmMoveCartesian";
const string Name_ArmMoveCartesianGuarded = "ArmMoveCartesianGuarded";
const string Name_ArmMoveJoint            = "ArmMoveJoint";
const string Name_ArmStop                 = "ArmStop";
const string Name_ArmStow                 = "ArmStow";
const string Name_ArmUnstow               = "ArmUnstow";
const string Name_CameraCapture           = "CameraCapture";
const string Name_PanTiltMoveJoints       = "PanTiltMoveJoints";
const string Name_TaskDeliverSample       = "TaskDeliverSample";

static vector<string> LanderOpNames = {
  Name_ArmMoveCartesian,
  Name_ArmMoveCartesianGuarded,
  Name_ArmMoveJoint,
  Name_ArmStop,
  Name_ArmStow,
  Name_ArmUnstow,
  Name_CameraCapture,
  Name_PanTiltMoveJoints,
  Name_TaskDeliverSample
};

void LanderInterface::initialize()
{
  static bool initialized = false;

  if (not initialized) {
    PlexilInterface::initialize();

    for (auto name : LanderOpNames) {
      registerLanderOperation (name);
    }

    // Initialize action clients

    m_armMoveCartesianClient =
      make_unique<ArmMoveCartesianActionClient>(Name_ArmMoveCartesian, true);
    m_armMoveCartesianGuardedClient =
      make_unique<ArmMoveCartesianGuardedActionClient>
      (Name_ArmMoveCartesianGuarded, true);
    m_armMoveJointClient =
      make_unique<ArmMoveJointActionClient>(Name_ArmMoveJoint, true);
    m_armStopClient =
      make_unique<ArmStopActionClient>(Name_ArmStop, true);
    m_armStowClient =
      make_unique<ArmStowActionClient>(Name_ArmStow, true);
    m_armUnstowClient =
      make_unique<ArmUnstowActionClient>(Name_ArmUnstow, true);
    m_cameraCaptureClient =
      make_unique<CameraCaptureActionClient>(Name_CameraCapture, true);
    m_panTiltMoveJointsClient =
      make_unique<PanTiltMoveJointsActionClient>(Name_PanTiltMoveJoints, true);
    m_taskDeliverSampleClient =
      make_unique<TaskDeliverSampleActionClient>(Name_TaskDeliverSample, true);

    // Connect to action servers
    
    connectActionServer (m_armMoveCartesianClient, Name_ArmMoveCartesian,
                         "/ArmMoveCartesian/status");
    connectActionServer (m_armMoveCartesianGuardedClient, Name_ArmMoveCartesianGuarded,
                         "/ArmMoveCartesianGuarded/status");
    connectActionServer (m_armMoveJointClient, Name_ArmMoveJoint,
                         "/ArmMoveJoint/status");
    connectActionServer (m_armStopClient, Name_ArmStop, "/ArmStop/status");
    connectActionServer (m_armStowClient, Name_ArmStow, "/ArmStow/status");
    connectActionServer (m_armUnstowClient, Name_ArmUnstow, "/ArmUnstow/status");
    connectActionServer (m_taskDeliverSampleClient, Name_TaskDeliverSample,
                         "/TaskDeliverSample/status");
    connectActionServer (m_panTiltMoveJointsClient, Name_PanTiltMoveJoints);
    connectActionServer (m_cameraCaptureClient, Name_CameraCapture,
                         "/CameraCapture/status");
  }
}


/////////////////////////////// Unified Lander Interface ////////////////////////

void LanderInterface::taskDeliverSample (int id)
{
  if (! markOperationRunning (Name_TaskDeliverSample, id)) return;
  thread action_thread (&LanderInterface::runNullaryAction<
                        TaskDeliverSampleActionClient,
                        TaskDeliverSampleGoal,
                        TaskDeliverSampleResultConstPtr,
                        TaskDeliverSampleFeedbackConstPtr>,
                        this, id, Name_TaskDeliverSample,
                        std::ref(m_taskDeliverSampleClient));
  action_thread.detach();
}


void LanderInterface::armMoveCartesian (int frame,
                                       bool relative,
                                       const vector<double>& position,
                                       const vector<double>& orientation,
                                       int id)
{
  if (! markOperationRunning (Name_ArmMoveCartesian, id)) return;

  geometry_msgs::Quaternion qm;

  // Deal with type of orientation
  if (orientation.size() == 3) { // assume Euler angle
    tf2::Quaternion q;
    // Yaw, pitch, roll, respectively.
    q.setEuler (orientation[2], orientation[1], orientation[0]);
    q.normalize();  // Recommended in ROS docs, not sure if needed here.
    qm = tf2::toMsg(q);
  }
  else { // assume quaternion orientation
    qm.x = orientation[0];
    qm.y = orientation[1];
    qm.z = orientation[2];
    qm.w = orientation[3];
  }

  geometry_msgs::Pose pose;
  pose.position.x = position[0];
  pose.position.y = position[1];
  pose.position.z = position[2];
  pose.orientation = qm;
  thread action_thread (&LanderInterface::armMoveCartesianAction, this,
                        frame, relative, pose, id);
  action_thread.detach();
}

void LanderInterface::armMoveCartesianAction (int frame,
                                             bool relative,
                                             const geometry_msgs::Pose& pose,
                                             int id)
{
  ArmMoveCartesianGoal goal;
  goal.frame = frame;
  goal.relative = relative;
  goal.pose = pose;
  string opname = Name_ArmMoveCartesian;

  runAction<actionlib::SimpleActionClient<ArmMoveCartesianAction>,
            ArmMoveCartesianGoal,
            ArmMoveCartesianResultConstPtr,
            ArmMoveCartesianFeedbackConstPtr>
    (opname, m_armMoveCartesianClient, goal, id,
     default_action_active_cb (opname),
     default_action_feedback_cb<ArmMoveCartesianFeedbackConstPtr> (opname),
     default_action_done_cb<ArmMoveCartesianResultConstPtr> (opname));
}

void LanderInterface::armMoveCartesianGuarded (int frame, bool relative,
                                              const vector<double>& position,
                                              const vector<double>& orientation,
                                              double force_threshold,
                                              double torque_threshold,int id)
{
  if (! markOperationRunning (Name_ArmMoveCartesianGuarded, id)) return;

  geometry_msgs::Quaternion qm;

  // Deal with type of orientation
  if (orientation.size() == 3) { // assume Euler angle
    tf2::Quaternion q;
    // Yaw, pitch, roll, respectively.
    q.setEuler (orientation[2], orientation[1], orientation[0]);
    q.normalize();  // Recommended in ROS docs, not sure if needed here.
    qm = tf2::toMsg(q);
  }
  else { // assume quaternion orientation
    qm.x = orientation[0];
    qm.y = orientation[1];
    qm.z = orientation[2];
    qm.w = orientation[3];
  }

  geometry_msgs::Pose pose;
  pose.position.x = position[0];
  pose.position.y = position[1];
  pose.position.z = position[2];
  pose.orientation = qm;
  thread action_thread (&LanderInterface::armMoveCartesianGuardedAction,
                        this, frame, relative, pose,
                        force_threshold, torque_threshold, id);
  action_thread.detach();
}

void LanderInterface::armMoveCartesianGuardedAction (int frame, bool relative,
                                                    const geometry_msgs::Pose& pose,
                                                    double force_threshold,
                                                    double torque_threshold,
                                                    int id)
{
  ArmMoveCartesianGuardedGoal goal;
  goal.frame = frame;
  goal.relative = relative;
  goal.pose = pose;
  goal.force_threshold = force_threshold;
  goal.torque_threshold = torque_threshold;
  string opname = Name_ArmMoveCartesianGuarded;

  runAction<actionlib::SimpleActionClient<ArmMoveCartesianGuardedAction>,
            ArmMoveCartesianGuardedGoal,
            ArmMoveCartesianGuardedResultConstPtr,
            ArmMoveCartesianGuardedFeedbackConstPtr>
    (opname, m_armMoveCartesianGuardedClient, goal, id,
     default_action_active_cb (opname),
     default_action_feedback_cb<ArmMoveCartesianGuardedFeedbackConstPtr> (opname),
     default_action_done_cb<ArmMoveCartesianGuardedResultConstPtr> (opname));
}

void LanderInterface::armMoveJoint (bool relative,
                                   int joint, double angle,
                                   int id)
{
  if (! markOperationRunning (Name_ArmMoveJoint, id)) return;
  thread action_thread (&LanderInterface::armMoveJointAction,
                        this, relative, joint, angle, id);
  action_thread.detach();
}

void LanderInterface::armMoveJointAction (bool relative,
                                         int joint, double angle,
                                         int id)
{
  ArmMoveJointGoal goal;
  goal.relative = relative;
  goal.joint = joint;
  goal.angle = angle;
  string opname = Name_ArmMoveJoint;  // shorter version

  ROS_INFO ("Starting ArmMoveJoint (relative=%d, joint=%d, angle=%f)",
            goal.relative, goal.joint, goal.angle);

  runAction<actionlib::SimpleActionClient<ArmMoveJointAction>,
            ArmMoveJointGoal,
            ArmMoveJointResultConstPtr,
            ArmMoveJointFeedbackConstPtr>
    (opname, m_armMoveJointClient, goal, id,
     default_action_active_cb (opname),
     default_action_feedback_cb<ArmMoveJointFeedbackConstPtr> (opname),
     default_action_done_cb<ArmMoveJointResultConstPtr> (opname));
}

void LanderInterface::armStop (int id)
{
  if (! markOperationRunning (Name_ArmStop, id)) return;
  thread action_thread (&LanderInterface::runNullaryAction<
                        ArmStopActionClient,
                        ArmStopGoal,
                        ArmStopResultConstPtr,
                        ArmStopFeedbackConstPtr>,
                        this, id, Name_ArmStop, std::ref(m_armStopClient));
  action_thread.detach();
}

void LanderInterface::armStow (int id)
{
  if (! markOperationRunning (Name_ArmStow, id)) return;
  //  thread action_thread (&LanderInterface::armStowAction, this, id);
  thread action_thread (&LanderInterface::runNullaryAction<
                        ArmStowActionClient,
                        ArmStowGoal,
                        ArmStowResultConstPtr,
                        ArmStowFeedbackConstPtr>,
                        this, id, Name_ArmStow, std::ref(m_armStowClient));
  action_thread.detach();
}

void LanderInterface::armUnstow (int id)
{
  if (! markOperationRunning (Name_ArmUnstow, id)) return;
  thread action_thread (&LanderInterface::runNullaryAction<
                        ArmUnstowActionClient,
                        ArmUnstowGoal,
                        ArmUnstowResultConstPtr,
                        ArmUnstowFeedbackConstPtr>,
                        this, id, Name_ArmUnstow, std::ref(m_armUnstowClient));
  action_thread.detach();
}

void LanderInterface::panTiltMoveJoints (double pan_degrees,
                                         double tilt_degrees,
                                         int id)
{
  if (! markOperationRunning (Name_PanTiltMoveJoints, id)) return;
  thread action_thread (&LanderInterface::panTiltMoveJointsAction, this,
                        pan_degrees, tilt_degrees, id);
  action_thread.detach();
}

void LanderInterface::panTiltMoveJointsAction (double pan_degrees,
                                               double tilt_degrees,
                                               int id)
{
  PanTiltMoveJointsGoal goal;
  goal.pan = pan_degrees * D2R;
  goal.tilt = tilt_degrees * D2R;
  std::stringstream args;
  args << goal.pan << ", " << goal.tilt;
  string opname = Name_PanTiltMoveJoints;
  runAction<actionlib::SimpleActionClient<PanTiltMoveJointsAction>,
            PanTiltMoveJointsGoal,
            PanTiltMoveJointsResultConstPtr,
            PanTiltMoveJointsFeedbackConstPtr>
    (opname, m_panTiltMoveJointsClient, goal, id,
     default_action_active_cb (opname, args.str()),
     default_action_feedback_cb<PanTiltMoveJointsFeedbackConstPtr> (opname),
     default_action_done_cb<PanTiltMoveJointsResultConstPtr> (opname));
}

void LanderInterface::cameraCapture (int id)
{
  if (! markOperationRunning (Name_CameraCapture, id)) return;
  thread action_thread (&LanderInterface::runNullaryAction<
                        CameraCaptureActionClient,
                        CameraCaptureGoal,
                        CameraCaptureResultConstPtr,
                        CameraCaptureFeedbackConstPtr>,
                        this, id, Name_CameraCapture,
                        std::ref(m_cameraCaptureClient));
  action_thread.detach();
}
