/*
# Software License Agreement (MIT License)
#
# Copyright (c) 2019, UFACTORY, Inc.
# All rights reserved.
#
# Author: Vinman <vinman.wen@ufactory.cc> <vinman.cub@gmail.com>
*/
#ifndef WRAPPER_XARM_API_H_
#define WRAPPER_XARM_API_H_

#include <iostream>
#include <functional>
#include <thread>
#include <vector>
#include <assert.h>
#include <cmath>
#include "xarm/core/common/data_type.h"
#include "xarm/core/xarm_config.h"
#include "xarm/core/linux/thread.h"
#include "xarm/core/instruction/uxbus_cmd.h"
#include "xarm/core/instruction/uxbus_cmd_ser.h"
#include "xarm/core/instruction/uxbus_cmd_tcp.h"
#include "xarm/core/instruction/uxbus_cmd_config.h"
#include "xarm/core/debug/debug_print.h"
#include "xarm/wrapper/common/utils.h"
#include "xarm/wrapper/common/timer.h"

#define DEFAULT_IS_RADIAN false
#define RAD_DEGREE 57.295779513082320876798154814105
#define TIMEOUT_10 10
#define NO_TIMEOUT -1
#define SDK_VERSION "1.4.0"

typedef unsigned int u32;
typedef float fp32;

class XArmAPI {
public:
	/*
	* @param port: ip-address(such as "192.168.1.185")
	*   Note: this parameter is required if parameter do_not_open is False
	* @param is_radian: set the default unit is radians or not, default is False
	* @param do_not_open: do not open, default is False, if true, you need to manually call the connect interface.
	* @param check_tcp_limit: reversed
	* @param check_joint_limit: reversed
	* @param check_cmdnum_limit: reversed
	* @param check_robot_sn: reversed
	* @param check_is_ready: check robot is ready to move or not, default is true
	* @param check_is_pause: check robot is pause or not, default is true
	*/
	XArmAPI(const std::string &port = "",
		bool is_radian = DEFAULT_IS_RADIAN,
		bool do_not_open = false,
		bool check_tcp_limit = true,
		bool check_joint_limit = true,
		bool check_cmdnum_limit = true,
		bool check_robot_sn = false,
		bool check_is_ready = true,
		bool check_is_pause = true);
	~XArmAPI(void);

public:
	int state; // state
	int mode; // mode
	int cmd_num; // cmd cache count
	fp32 *joints_torque; // joints torque, fp32[7]{servo-1, ..., servo-7}
	bool *motor_brake_states; // motor brake states, bool[8]{servo-1, ..., servo-7, reversed}
	bool *motor_enable_states; // motor enable states, bool[8]{servo-1, ..., servo-7, reversed}
	int error_code; // error code
	int warn_code; // warn code
	fp32 *tcp_load; // tcp load, fp32[4]{weight, x, y, z}
	int collision_sensitivity; // collision sensitivity
	int teach_sensitivity; // teach sensitivity
	int device_type; // device type
	int axis; // robot axis
	int master_id;
	int slave_id;
	int motor_tid;
	int motor_fid;
	unsigned char version[30]; // version
	unsigned char sn[40]; // sn
	int *version_number; // version numbre
	fp32 tcp_jerk; // tcp jerk
	fp32 joint_jerk; // joint jerk
	fp32 rot_jerk; // rot jerk
	fp32 max_rot_acc; // max rot acc
	fp32 *tcp_speed_limit; // fp32[2]{min, max}
	fp32 *tcp_acc_limit; // fp32[2]{min, max}
	fp32 last_used_tcp_speed;
	fp32 last_used_tcp_acc;

	fp32 *angles; // fp32[7]{servo-1, ..., servo-7}
	fp32 *last_used_angles; // fp32[7]{servo-1, ..., servo-7}
	fp32 *joint_speed_limit; // fp32[2]{min, max}
	fp32 *joint_acc_limit; // fp32[2]{min, max}
	fp32 last_used_joint_speed;
	fp32 last_used_joint_acc;
	fp32 *position; // fp32[6]{x, y, z, roll, pitch, yaw}
	fp32 *last_used_position; // fp32[6]{x, y, z, roll, pitch, yaw}
	fp32 *tcp_offset; // fp32[6]{x, y, z, roll, pitch, yaw}
	fp32 *gravity_direction; // fp32[3]{x_direction, y_direction, z_direction}

	fp32 realtime_tcp_speed;
	fp32 *realtime_joint_speeds;

	fp32 *world_offset; // fp32[6]{x, y, z, roll, pitch, yaw}
	fp32 *temperatures;

	bool default_is_radian;

public:
	/*
	* xArm has error/warn or not, only available in socket way
	*/
	bool has_err_warn(void);

	/*
	* xArm has error or not, only available in socket way
	*/
	bool has_error(void);

	/*
	* xArm has warn or not, only available in socket way
	*/
	bool has_warn(void);

	/*
	* xArm is connected or not
	*/
	bool is_connected(void);

	/*
	* xArm is reported or not, only available in socket way
	*/
	bool is_reported(void);

	/*
	* Connect to xArm
	* @param port: port name or the ip address
	* return:
		0: success
		-1: port is empty
		-2: tcp control connect failed
		-3: tcp report connect failed
	*/
	int connect(const std::string &port = "");

	/*
	* Disconnect
	*/
	void disconnect(void);

	/*no use please*/
	void _recv_report_data(void);

	/*
	* Get the xArm version
	* @param version:
	* return: see the API code documentation for details.
	*/
	int get_version(unsigned char version[40]);

	/*
	* Get the xArm sn
	* @param robot_sn:
	* return: see the API code documentation for details.
	*/
	int get_robot_sn(unsigned char robot_sn[40]);

	/*
	* Get the xArm state
	* @param: the state of xArm
		1: in motion
		2: sleeping
		3: suspended
		4: stopping
	* return: see the API code documentation for details.
	*/
	int get_state(int *state);

	/*
	* Shutdown the xArm controller system
	* @param value:
		1: remote shutdown
	* return: see the API code documentation for details.
	*/
	int shutdown_system(int value = 1);

	/*
	* Get the cmd count in cache
	* return: see the API code documentation for details.
	*/
	int get_cmdnum(int *cmdnum);

	/*
	* Get the controller error and warn code
	* return: see the API code documentation for details.
	*/
	int get_err_warn_code(int err_warn[2]);

	/*
	* Get the cartesian position
	* @param pose: the position of xArm, like [x(mm), y(mm), z(mm), roll(rad or °), pitch(rad or °), yaw(rad or °)]
		if default_is_radian is true, the value of roll/pitch/yaw should be in radians
		if default_is_radian is false, The value of roll/pitch/yaw should be in degrees
	* return: see the API code documentation for details.
	*/
	int get_position(fp32 pose[6]);

	/*
	* Get the servo angle
	* @param angles: the angles of the servos, like [servo-1, ..., servo-7]
		if default_is_radian is true, the value of servo-1/.../servo-7 should be in radians
		if default_is_radian is false, The value of servo-1/.../servo-7 should be in degrees
	* return: see the API code documentation for details.
	*/
	int get_servo_angle(fp32 angles[7]);

	/*
	* Motion enable
	* @param enable: enable or not
	* @param servo_id: servo id, 1-8, 8(enable/disable all servo)
	* return: see the API code documentation for details.
	*/
	int motion_enable(bool enable, int servo_id = 8);

	/*
	* Set the xArm state
	* @param state: state
		0: sport state
		3: pause state
		4: stop state
	* return: see the API code documentation for details.
	*/
	int set_state(int state);

	/*
	* Set the xArm mode
	* @param mode: mode
		0: position control mode
		1: servo motion mode
		2: joint teaching mode
		3: cartesian teaching mode (invalid)
	* return: see the API code documentation for details.
	*/
	int set_mode(int mode);

	/*
	* Attach the servo
	* @param servo_id: servo id, 1-8, 8(attach all servo)
	* return: see the API code documentation for details.
	*/
	int set_servo_attach(int servo_id);

	/*
	* Detach the servo, be sure to do protective work before unlocking to avoid injury or damage.
	* @param servo_id: servo id, 1-8, 8(detach all servo)
	* return: see the API code documentation for details.
	*/
	int set_servo_detach(int servo_id);

	/*
	* Clean the controller error, need to be manually enabled motion and set state after clean error
	* return: see the API code documentation for details.
	*/
	int clean_error(void);

	/*
	* Clean the controller warn
	* return: see the API code documentation for details.
	*/
	int clean_warn(void);

	/*
	* Set the arm pause time, xArm will pause sltime second
	* @param sltime: sleep second
	* return: see the API code documentation for details.
	*/
	int set_pause_time(fp32 sltime);

	/*
	* Set the sensitivity of collision
	* @param sensitivity: sensitivity value, 0~5
	* return: see the API code documentation for details.
	*/
	int set_collision_sensitivity(int sensitivity);

	/*
	* Set the sensitivity of drag and teach
	* @param sensitivity: sensitivity value, 1~5
	* return: see the API code documentation for details.
	*/
	int set_teach_sensitivity(int sensitivity);

	/*
	* Set the direction of gravity
	* @param gravity_dir: direction of gravity, such as [x(mm), y(mm), z(mm)]
	* return: see the API code documentation for details.
	*/
	int set_gravity_direction(fp32 gravity_dir[3]);

	/*
	* Clean current config and restore system default settings
	* Note:
		1. This interface will clear the current settings and restore to the original settings (system default settings)
	* return: see the API code documentation for details.
	*/
	int clean_conf(void);

	/*
	* Save config
	* Note:
		1. This interface can record the current settings and will not be lost after the restart.
		2. The clean_conf interface can restore system default settings
	* return: see the API code documentation for details.
	*/
	int save_conf(void);

	/*
	* Set the position
		MoveLine: Linear motion
		MoveArcLine: Linear arc motion with interpolation
	* @param pose: position, like [x(mm), y(mm), z(mm), roll(rad or °), pitch(rad or °), yaw(rad or °)]
		if default_is_radian is true, the value of roll/pitch/yaw should be in radians
		if default_is_radian is false, The value of roll/pitch/yaw should be in degrees
	* @param radius: move radius, if radius is None or radius less than 0, will MoveLine, else MoveArcLine
	* @param speed: move speed (mm/s, rad/s), default is this.last_used_tcp_speed
	* @param mvacc: move acceleration (mm/s^2, rad/s^2), default is this.last_used_tcp_acc
	* @param mvtime: reserved, 0
	* @param wait: whether to wait for the arm to complete, default is False
	* @param timeout: maximum waiting time(unit: second), default is no timeout, only valid if wait is true
	* return: see the API code documentation for details.
	*/
	int set_position(fp32 pose[6], fp32 radius = -1, fp32 speed = 0, fp32 acc = 0, fp32 mvtime = 0, bool wait = false, fp32 timeout = NO_TIMEOUT);
	int set_position(fp32 pose[6], fp32 radius = -1, bool wait = false, fp32 timeout = NO_TIMEOUT);
	int set_position(fp32 pose[6], bool wait = false, fp32 timeout = NO_TIMEOUT);

	/*
	* Movement relative to the tool coordinate system
	* @param pose: the coordinate relative to the current tool coordinate systemion, like [x(mm), y(mm), z(mm), roll(rad or °), pitch(rad or °), yaw(rad or °)]
		if default_is_radian is true, the value of roll/pitch/yaw should be in radians
		if default_is_radian is false, The value of roll/pitch/yaw should be in degrees
	* @param speed: move speed (mm/s, rad/s), default is this.last_used_tcp_speed
	* @param mvacc: move acceleration (mm/s^2, rad/s^2), default is this.last_used_tcp_acc
	* @param mvtime: reserved, 0
	* @param wait: whether to wait for the arm to complete, default is False
	* @param timeout: maximum waiting time(unit: second), default is no timeout, only valid if wait is true
	* return: see the API code documentation for details.
	*/
	int set_tool_position(fp32 pose[6], fp32 speed = 0, fp32 acc = 0, fp32 mvtime = 0, bool wait = false, fp32 timeout = NO_TIMEOUT);
	int set_tool_position(fp32 pose[6], bool wait = false, fp32 timeout = NO_TIMEOUT);

	/*
	* Set the servo angle
	* @param angles: angles, like [servo-1, ..., servo-7]
		if default_is_radian is true, the value of servo-1/.../servo-7 should be in radians
		if default_is_radian is false, The value of servo-1/.../servo-7 should be in degrees
	* @param servo_id: servo id, 1~7, specify the joint ID to set
	* @param angle: servo angle, use with servo_id parameters
	* @param speed: move speed (rad/s or °/s), default is this.last_used_joint_speed
		if default_is_radian is true, the value of speed should be in radians
		if default_is_radian is false, The value of speed should be in degrees
	* @param acc: move acceleration (rad/s^2 or °/s^2), default is this.last_used_joint_acc
		if default_is_radian is true, the value of acc should be in radians
		if default_is_radian is false, The value of acc should be in degrees
	* @param mvtime: reserved, 0
	* @param wait: whether to wait for the arm to complete, default is False
	* @param timeout: maximum waiting time(unit: second), default is no timeout, only valid if wait is true
	* return: see the API code documentation for details.
	*/
	int set_servo_angle(fp32 angles[7], fp32 speed = 0, fp32 acc = 0, fp32 mvtime = 0, bool wait = false, fp32 timeout = NO_TIMEOUT);
	int set_servo_angle(fp32 angles[7], bool wait = false, fp32 timeout = NO_TIMEOUT);
	int set_servo_angle(int servo_id, fp32 angle, fp32 speed = 0, fp32 acc = 0, fp32 mvtime = 0, bool wait = false, fp32 timeout = NO_TIMEOUT);
	int set_servo_angle(int servo_id, fp32 angle, bool wait = false, fp32 timeout = NO_TIMEOUT);

	/*
	* Servo_j motion, execute only the last instruction, need to be set to servo motion mode(this.set_mode(1))
	* @param angles: angles, like [servo-1, ..., servo-7]
		if default_is_radian is true, the value of servo-1/.../servo-7 should be in radians
		if default_is_radian is false, The value of servo-1/.../servo-7 should be in degrees
	* @param speed: reserved, move speed (rad/s or °/s)
		if default_is_radian is true, the value of speed should be in radians
		if default_is_radian is false, The value of speed should be in degrees
	* @param acc: reserved, move acceleration (rad/s^2 or °/s^2)
		if default_is_radian is true, the value of acc should be in radians
		if default_is_radian is false, The value of acc should be in degrees
	* @param mvtime: reserved, 0
	* return: see the API code documentation for details.
	*/
	int set_servo_angle_j(fp32 angles[7], fp32 speed = 0, fp32 acc = 0, fp32 mvtime = 0);

	/*
	* Servo cartesian motion, execute only the last instruction, need to be set to servo motion mode(this.set_mode(1))
	* @param pose: position, like [x(mm), y(mm), z(mm), roll(rad or °), pitch(rad or °), yaw(rad or °)]
		if default_is_radian is true, the value of roll/pitch/yaw should be in radians
		if default_is_radian is false, The value of roll/pitch/yaw should be in degrees
	* @param speed: reserved, move speed (mm/s)
	* @param mvacc: reserved, move acceleration (mm/s^2)
	* @param mvtime: reserved, 0
	* return: see the API code documentation for details.
	*/
	int set_servo_cartesian(fp32 pose[6], fp32 speed = 0, fp32 acc = 0, fp32 mvtime = 0);

	/*
	* The motion calculates the trajectory of the space circle according to the three-point coordinates.
	  The three-point coordinates are (current starting point, pose1, pose2).
	* @param pose1: cartesian position, [x(mm), y(mm), z(mm), roll(rad or °), pitch(rad or °), yaw(rad or °)]
		if default_is_radian is true, the value of roll/pitch/yaw should be in radians
		if default_is_radian is false, The value of roll/pitch/yaw should be in degrees
	* @param pose2: cartesian position, [x(mm), y(mm), z(mm), roll(rad or °), pitch(rad or °), yaw(rad or °)]
		if default_is_radian is true, the value of roll/pitch/yaw should be in radians
		if default_is_radian is false, The value of roll/pitch/yaw should be in degrees
	* @param percent: the percentage of arc length and circumference of the movement
	* @param speed: move speed (mm/s, rad/s), default is this.last_used_tcp_speed
	* @param mvacc: move acceleration (mm/s^2, rad/s^2), default is this.last_used_tcp_acc
	* @param mvtime: 0, reserved
	* @param wait: whether to wait for the arm to complete, default is False
	* @param timeout: maximum waiting time(unit: second), default is no timeout, only valid if wait is True
	* return: see the API code documentation for details.
	*/
	int move_circle(fp32 pose1[6], fp32 pose2[6], fp32 percent, fp32 speed = 0, fp32 acc = 0, fp32 mvtime = 0, bool wait = false, fp32 timeout = NO_TIMEOUT);

	/*
	* Move to go home (Back to zero)
	* @param speed: move speed (rad/s or °/s), default is 50 °/s
		if default_is_radian is true, the value of speed should be in radians
		if default_is_radian is false, The value of speed should be in degrees
	* @param acc: move acceleration (rad/s^2 or °/s^2), default is 1000 °/s^2
		if default_is_radian is true, the value of acc should be in radians
		if default_is_radian is false, The value of acc should be in degrees
	* @param mvtime: reserved, 0
	* @param wait: whether to wait for the arm to complete, default is False
	* @param timeout: maximum waiting time(unit: second), default is no timeout, only valid if wait is true
	* return: see the API code documentation for details.
	*/
	int move_gohome(fp32 speed = 0, fp32 acc = 0, fp32 mvtime = 0, bool wait = false, fp32 timeout = NO_TIMEOUT);
	int move_gohome(bool wait = false, fp32 timeout = NO_TIMEOUT);

	/*
	* Reset
	* @param wait: whether to wait for the arm to complete, default is False
	* @param timeout: maximum waiting time(unit: second), default is no timeout, only valid if wait is true
	* return: see the API code documentation for details.
	*/
	void reset(bool wait = false, fp32 timeout = NO_TIMEOUT);

	/*
	* Emergency stop
	*/
	void emergency_stop(void);

	/*
	* Set the tool coordinate system offset at the end
	* @param pose_offset: tcp offset, like [x(mm), y(mm), z(mm), roll(rad or °), pitch(rad or °), yaw(rad or °)]
		if default_is_radian is true, the value of roll/pitch/yaw should be in radians
		if default_is_radian is false, The value of roll/pitch/yaw should be in degrees
	* return: see the API code documentation for details.
	*/
	int set_tcp_offset(fp32 pose_offset[6]);

	/*
	* Set the load
	* @param weight: load weight (unit: kg)
	* @param center_of_gravity: tcp load center of gravity, like [x(mm), y(mm), z(mm)]
	* return: see the API code documentation for details.
	*/
	int set_tcp_load(fp32 weight, fp32 center_of_gravity[3]);

	/*
	* Set the translational jerk of Cartesian space
	* @param jerk: jerk (mm/s^3)
	* return: see the API code documentation for details.
	*/
	int set_tcp_jerk(fp32 jerk);

	/*
	* Set the max translational acceleration of Cartesian space
	* @param acc: max acceleration (mm/s^2)
	* return: see the API code documentation for details.
	*/
	int set_tcp_maxacc(fp32 acc);

	/*
	* Set the jerk of Joint space
	* @param jerk: jerk (°/s^3 or rad/s^3)
		if default_is_radian is true, the value of jerk should be in radians
		if default_is_radian is false, The value of jerk should be in degrees
	* return: see the API code documentation for details.
	*/
	int set_joint_jerk(fp32 jerk);

	/*
	* Set the max acceleration of Joint space
	* @param acc: max acceleration (°/s^2 or rad/s^2)
		if default_is_radian is true, the value of jerk should be in radians
		if default_is_radian is false, The value of jerk should be in degrees
	* return: see the API code documentation for details.
	*/
	int set_joint_maxacc(fp32 acc);

	/*
	* Get inverse kinematics
	* @param pose: source pose, like [x(mm), y(mm), z(mm), roll(rad or °), pitch(rad or °), yaw(rad or °)]
		if default_is_radian is true, the value of roll/pitch/yaw should be in radians
		if default_is_radian is false, The value of roll/pitch/yaw should be in degrees
	* @param angles: target angles, like [servo-1, ..., servo-7]
		if default_is_radian is true, the value of servo-1/.../servo-7 should be in radians
		if default_is_radian is false, The value of servo-1/.../servo-7 should be in degrees
	* return: see the API code documentation for details.
	*/
	int get_inverse_kinematics(fp32 pose[6], fp32 angles[7]);

	/*
	* Get forward kinematics
	* @param angles: source angles, like [servo-1, ..., servo-7]
		if default_is_radian is true, the value of servo-1/.../servo-7 should be in radians
		if default_is_radian is false, The value of servo-1/.../servo-7 should be in degrees
	* @param pose: target pose, like [x(mm), y(mm), z(mm), roll(rad or °), pitch(rad or °), yaw(rad or °)]
		if default_is_radian is true, the value of roll/pitch/yaw should be in radians
		if default_is_radian is false, The value of roll/pitch/yaw should be in degrees
	* return: see the API code documentation for details.
	*/
	int get_forward_kinematics(fp32 angles[7], fp32 pose[6]);

	/*
	* Check the tcp pose is in limit
	* @param pose: pose, like [x(mm), y(mm), z(mm), roll(rad or °), pitch(rad or °), yaw(rad or °)]
		if default_is_radian is true, the value of roll/pitch/yaw should be in radians
		if default_is_radian is false, The value of roll/pitch/yaw should be in degrees
	* @param limit: 1: limit, 0: no limit
	* return: see the API code documentation for details.
	*/
	int is_tcp_limit(fp32 pose[6], int *limit);

	/*
	* Check the joint is in limit
	* @param angles: angles, like [servo-1, ..., servo-7]
		if default_is_radian is true, the value of servo-1/.../servo-7 should be in radians
		if default_is_radian is false, The value of servo-1/.../servo-7 should be in degrees
	* @param limit: 1: limit, 0: no limit
	* return: see the API code documentation for details.
	*/
	int is_joint_limit(fp32 angles[7], int *limit);

	/*
	* Set the gripper enable
	* @param enable: enable or not
	* return: see the API code documentation for details.
	*/
	int set_gripper_enable(bool enable);

	/*
	* Set the gripper mode
	* @param mode: 1: location mode, 2: speed mode(no use), 3: torque mode(no use)
	* return: see the API code documentation for details.
	*/
	int set_gripper_mode(int mode);

	/*
	* Get the gripper position
	* @param pos: used to store the results obtained
	* return: see the API code documentation for details.
	*/
	int get_gripper_position(fp32 *pos);

	/*
	* Set the gripper position
	* @param pos: gripper position
	* @param wait: wait or not, default is false
	* @param timeout: maximum waiting time(unit: second), default is 10s, only valid if wait is true
	* return: see the API code documentation for details.
	*/
	int set_gripper_position(fp32 pos, bool wait = false, fp32 timeout = 10);

	/*
	* Set the gripper speed
	* @param speed:
	* return: see the API code documentation for details.
	*/
	int set_gripper_speed(fp32 speed);

	/*
	* Get the gripper error code
	* @param err: used to store the results obtained
	* return: see the API code documentation for details.
	*/
	int get_gripper_err_code(int *err);

	/*
	* Clean the gripper error
	* return: see the API code documentation for details.
	*/
	int clean_gripper_error(void);

	/*
	* Get the digital value of the Tool GPIO
	* @param io0_value: the digital value of Tool GPIO-0
	* @param io1_value: the digital value of Tool GPIO-1
	* return: see the API code documentation for details.
	*/
	int get_tgpio_digital(int *io0_value, int *io1_value);

	/*
	* Set the digital value of the specified Tool GPIO
	* @param ionum: ionum, 0 or 1
	* @param value: the digital value of the specified io
	* return: see the API code documentation for details.
	*/
	int set_tgpio_digital(int ionum, int value);

	/*
	* Get the analog value of the specified Tool GPIO
	* @param ionum: ionum, 0 or 1
	* @param value: the analog value of the specified tool io
	* return: see the API code documentation for details.
	*/
	int get_tgpio_analog(int ionum, fp32 *value);

	/*
	* Get the digital value of the specified Controller GPIO
	* @param digitals: the values of the controller GPIO
	* return: see the API code documentation for details.
	*/
	int get_cgpio_digital(int *digitals);

	/*
	* Get the analog value of the specified Controller GPIO
	* @param ionum: ionum, 0 or 1
	* @param value: the analog value of the specified controller io
	* return: see the API code documentation for details.
	*/
	int get_cgpio_analog(int ionum, fp32 *value);

	/*
	* Set the digital value of the specified Controller GPIO
	* @param ionum: ionum, 0 ~ 7
	* @param value: the digital value of the specified io
	* return: see the API code documentation for details.
	*/
	int set_cgpio_digital(int ionum, int value);

	/*
	* Set the analog value of the specified Controller GPIO
	* @param ionum: ionum, 0 or 1
	* @param value: the analog value of the specified io
	* return: see the API code documentation for details.
	*/
	int set_cgpio_analog(int ionum, int value);

	/*
	* Set the digital input functional mode of the Controller GPIO
	* @param ionum: ionum, 0 ~ 7
	* @param fun: functional mode
		0: general input
		1: external emergency stop
		2: reversed, protection reset
		3: reversed, reduced mode
		4: reversed, operating mode
		5: reversed, three-state switching signal
		11: offline task
		12: teaching mode
	* return: see the API code documentation for details.
	*/
	int set_cgpio_digital_input_function(int ionum, int fun);

	/*
	* Set the digital output functional mode of the specified Controller GPIO
	* @param ionum: ionum, 0 ~ 7
	* @param fun: functional mode
		0: general output
		1: emergency stop
		2: in motion
		11: has error
		12: has warn
		13: in collision
		14: in teaching
		15: in offline task
	* return: see the API code documentation for details.
	*/
	int set_cgpio_digital_output_function(int ionum, int fun);

	/*
	* Get the state of the Controller GPIO
	* @param state: contorller gpio module state and controller gpio module error code
		state[0]: contorller gpio module state
			state[0] == 0: normal
			state[0] == 1：wrong
			state[0] == 6：communication failure
		state[1]: controller gpio module error code
			state[1] == 0: normal
			state[1] != 0：error code
	* @param digit_io:
		digit_io[0]: digital input functional gpio state
		digit_io[1]: digital input configuring gpio state
		digit_io[2]: digital output functional gpio state
		digit_io[3]: digital output configuring gpio state
	* @param analog:
		analog[0]: analog-0 input value
		analog[1]: analog-1 input value
		analog[2]: analog-0 output value
		analog[3]: analog-1 output value
	* @param input_conf: digital input functional info
	* @param output_conf: digital output functional info
	* return: see the API code documentation for details.
	*/
	int get_cgpio_state(int *state, int *digit_io, fp32 *analog, int *input_conf, int *output_conf);

	/*
	* Register the report location callback
	*/
	int register_report_location_callback(void(*callback)(const fp32 *pose, const fp32 *angles));

	/*
	* Register the connect status changed callback
	*/
	int register_connect_changed_callback(void(*callback)(bool connected, bool reported));

	/*
	* Register the state status changed callback
	*/
	int register_state_changed_callback(void(*callback)(int state));

	/*
	* Register the mode changed callback
	*/
	int register_mode_changed_callback(void(*callback)(int mode));

	/*
	* Register the motor enable states or motor brake states changed callback
	*/
	int register_mtable_mtbrake_changed_callback(void(*callback)(int mtable, int mtbrake));

	/*
	* Register the error code or warn code changed callback
	*/
	int register_error_warn_changed_callback(void(*callback)(int err_code, int warn_code));

	/*
	* Register the cmdnum changed callback
	*/
	int register_cmdnum_changed_callback(void(*callback)(int cmdnum));

	/*
	* Register the temperature changed callback
	*/
	int register_temperature_changed_callback(void(*callback)(const fp32 *temps));

	/*
	* Register the value of counter changed callback
	*/
	int register_count_changed_callback(void(*callback)(int count));

	/*
	* Release the location report callback
	* @param callback: NULL means to release all callbacks;
	*/
	int release_report_location_callback(void(*callback)(const fp32 *pose, const fp32 *angles) = NULL);

	/*
	* Release the connect changed callback
	* @param callback: NULL means to release all callbacks for the same event
	*/
	int release_connect_changed_callback(void(*callback)(bool connected, bool reported) = NULL);

	/*
	* Release the state changed callback
	* @param callback: NULL means to release all callbacks for the same event
	*/
	int release_state_changed_callback(void(*callback)(int state) = NULL);

	/*
	* Release the mode changed callback
	* @param callback: NULL means to release all callbacks for the same event
	*/
	int release_mode_changed_callback(void(*callback)(int mode) = NULL);

	/*
	* Release the motor enable states or motor brake states changed callback
	* @param callback: NULL means to release all callbacks for the same event
	*/
	int release_mtable_mtbrake_changed_callback(void(*callback)(int mtable, int mtbrake) = NULL);

	/*
	* Release the error warn changed callback
	* @param callback: NULL means to release all callbacks for the same event
	*/
	int release_error_warn_changed_callback(void(*callback)(int err_code, int warn_code) = NULL);

	/*
	* Release the cmdnum changed callback
	* @param callback: NULL means to release all callbacks for the same event
	*/
	int release_cmdnum_changed_callback(void(*callback)(int cmdnum) = NULL);

	/*
	* Release the temperature changed callback
	* @param callback: NULL means to release all callbacks for the same event
	*/
	int release_temperature_changed_callback(void(*callback)(const fp32 *temps) = NULL);

	/*
	* Release the value of counter changed callback
	* @param callback: NULL means to release all callbacks for the same event
	*/
	int release_count_changed_callback(void(*callback)(int count) = NULL);

	/*
	* Get suction cup state
	* @param val:
		0: suction cup is off
		1: suction cup is on
	* return: see the API code documentation for details.
	*/
	int get_suction_cup(int *val);

	/*
	* Set suction cup
	* @param on: open suction cup or not
	* @param wait: wait or not, default is false
	* @timeout: maximum waiting time(unit: second), default is 10s, only valid if wait is true
	* return: see the API code documentation for details.
	*/
	int set_suction_cup(bool on, bool wait = false, float timeout = 3);

	/*
	* Get gripper version, only for debug
	* return: see the API code documentation for details.
	*/
	int get_gripper_version(unsigned char versions[3]);

	/*
	* Get servo version, only for debug
	* return: see the API code documentation for details.
	*/
	int get_servo_version(unsigned char versions[3], int servo_id = 1);

	/*
	* Get tool gpio version, only for debug
	* return: see the API code documentation for details.
	*/
	int get_tgpio_version(unsigned char versions[3]);

	/*
	* Reload dynamics, only for debug
	* return: see the API code documentation for details.
	*/
	int reload_dynamics(void);

	/*
	* Turn on/off reduced mode
	* @param on: on/off
	* return: see the API code documentation for details.
	*/
	int set_reduced_mode(bool on);

	/*
	* Set the maximum tcp speed of the reduced mode
	* @param speed: the maximum tcp speed
	* return: see the API code documentation for details.
	*/
	int set_reduced_max_tcp_speed(float speed);

	/*
	* Set the maximum joint speed of the reduced mode
	* @param speed: the maximum joint speed
		if default_is_radian is true, the value of speed should be in radians
		if default_is_radian is false, The value of speed should be in degrees
	* return: see the API code documentation for details.
	*/
	int set_reduced_max_joint_speed(float speed);

	/*
	* Get reduced mode
	* @param mode:
		0: reduced mode is on
		1: reduced mode is off
	* return: see the API code documentation for details.
	*/
	int get_reduced_mode(int *mode);

	/*
	* Get states of the reduced mode
	* @param on:
		0: reduced mode is on
		1: reduced mode is off
	* @param xyz_list: the tcp boundary, like [reduced_x_max, reduced_x_min, reduced_y_max, reduced_y_min, reduced_z_max, reduced_z_min],
	* @param tcp_speed: the maximum tcp speed of reduced mode
	* @param joint_speed: the maximum joint speed of reduced mode
		if default_is_radian is true, the value of speed should be in radians
		if default_is_radian is false, The value of speed should be in degrees
	* @param jrange: the joint range of the reduced mode, like [joint-1-min, joint-1-max, ..., joint-7-min, joint-7-max]
		if default_is_radian is true, the value of speed should be in radians
		if default_is_radian is false, The value of speed should be in degrees
	* @param fense_is_on:
		0: safety mode is on
		1: safety mode is off
	* @param collision_rebound_is_on:
		0: collision rebound is on
		1: collision rebound is off
	* return: see the API code documentation for details.
	*/
	int get_reduced_states(int *on, int *xyz_list, float *tcp_speed, float *joint_speed, float jrange[14] = NULL, int *fense_is_on = NULL, int *collision_rebound_is_on = NULL);

	/*
	* Set the boundary of the safety boundary mode
	* @param boundary: like [x_max(mm), x_min(mm), y_max(mm), y_min(mm), z_max(mm), z_min(mm)]
	* return: see the API code documentation for details.
	*/
	int set_reduced_tcp_boundary(int boundary[6]);

	/*
	* Set the joint range of the reduced mode
	* @param jrange: like [joint-1-min, joint-1-max, ..., joint-7-min, joint-7-max]
		if default_is_radian is true, the value of speed should be in radians
		if default_is_radian is false, The value of speed should be in degrees
	* return: see the API code documentation for details.
	*/
	int set_reduced_joint_range(float jrange[14]);

	/*
	* Turn on/off safety mode
	* @param on: on/off
	* return: see the API code documentation for details.
	*/
	int set_fense_mode(bool on);

	/*
	* Turn on/off collision rebound
	* @param on: on/off
	* return: see the API code documentation for details.
	*/
	int set_collision_rebound(bool on);

	/*
	* Set the base coordinate system offset at the end
	* @param pose_offset: tcp offset, like [x(mm), y(mm), z(mm), roll(rad or °), pitch(rad or °), yaw(rad or °)]
		if default_is_radian is true, the value of roll/pitch/yaw should be in radians
		if default_is_radian is false, The value of roll/pitch/yaw should be in degrees
	* return: see the API code documentation for details.
	*/
	int set_world_offset(float pose_offset[6]);

	/*
	* Start trajectory recording, only in teach mode, so you need to set joint teaching mode before.
	* return: see the API code documentation for details.
	*/
	int start_record_trajectory(void);

	/*
	* Stop trajectory recording
	* @param filename: the name to save
		If the filename is NULL, just stop recording, do not save, you need to manually call `save_record_trajectory` save before changing the mode. otherwise it will be lost
		the trajectory is saved in the controller box.
		this action will overwrite the trajectory with the same name
		empty the trajectory in memory after saving, so repeated calls will cause the recorded trajectory to be covered by an empty trajectory.
	* return: see the API code documentation for details.
	*/
	int stop_record_trajectory(char* filename = NULL);

	/*
	* Save the trajectory you just recorded
	* @param filename: the name to save
		the trajectory is saved in the controller box.
		this action will overwrite the trajectory with the same name
		empty the trajectory in memory after saving, so repeated calls will cause the recorded trajectory to be covered by an empty trajectory.
	* return: see the API code documentation for details.
	*/
	int save_record_trajectory(char* filename, float timeout = 10);

	/*
	* Load the trajectory
	* @param filename: the name of the trajectory to load
	* @param timeout: the maximum timeout waiting for loading to complete, default is 10 seconds.
	return: see the API code documentation for details.
	*/
	int load_trajectory(char* filename, float timeout = 10);

	/*
	* Playback trajectory
	* @param times: number of playbacks.
	* @param filename: the name of the trajectory to play back
		if filename is None, you need to manually call the `load_trajectory` to load the trajectory.
	* @param wait: whether to wait for the arm to complete, default is False.
	* @param double_speed: double speed, only support 1/2/4, default is 1, only available if version > 1.2.11
	return: see the API code documentation for details.
	*/
	int playback_trajectory(int times = 1, char* filename = NULL, bool wait = false, int double_speed = 1);

	/*
	* Get trajectory read/write status
	* @param status:
		0: no read/write
		1: loading
		2: load success
		3: load failed
		4: saving
		5: save success
		6: save failed
	* return: see the API code documentation for details.
	*/
	int get_trajectory_rw_status(int *status);

	/*
	* Reset counter value
	* return: see the API code documentation for details.
	*/
	int set_counter_reset(void);

	/*
	* Set counter plus 1
	* return: see the API code documentation for details.
	*/
	int set_counter_increase(void);

private:
	void _init(void);
	void _check_version(void);
	bool version_is_ge(int major = 1, int minor = 2, int revision = 11);
	void _check_is_pause(void);
	void _wait_stop(fp32 timeout);
	void _update_old(unsigned char *data);
	void _update(unsigned char *data);
	template<typename callable_vector, typename callable>
	inline int _register_event_callback(callable_vector&& callbacks, callable&& f);
	template<typename callable_vector, typename callable>
	inline int _release_event_callback(callable_vector&& callbacks, callable&& f);
	inline void _report_location_callback(void);
	inline void _report_connect_changed_callback(void);
	inline void _report_state_changed_callback(void);
	inline void _report_mode_changed_callback(void);
	inline void _report_mtable_mtbrake_changed_callback(void);
	inline void _report_error_warn_changed_callback(void);
	inline void _report_cmdnum_changed_callback(void);
	inline void _report_temperature_changed_callback(void);
	inline void _report_count_changed_callback(void);

private:
	std::string port_;
	bool check_tcp_limit_;
	bool check_joint_limit_;
	bool check_cmdnum_limit_;
	bool check_robot_sn_;
	bool check_is_ready_;
	bool check_is_pause_;
	// pthread_t report_thread_;
	std::thread report_thread_;
	std::mutex mutex_;
	std::condition_variable cond_;
	bool is_ready_;
	bool is_stop_;
	bool is_tcp_;
	bool is_old_protocol_;
	bool is_first_report_;
	bool is_sync_;

	int major_version_number_;
	int minor_version_number_;
	int revision_version_number_;

	long long sleep_finish_time_;

	int mt_brake_;
	int mt_able_;
	fp32 min_tcp_speed_;
	fp32 max_tcp_speed_;
	fp32 min_tcp_acc_;
	fp32 max_tcp_acc_;
	fp32 min_joint_speed_;
	fp32 max_joint_speed_;
	fp32 min_joint_acc_;
	fp32 max_joint_acc_;
	int sv3msg_[16];
	fp32 trs_msg_[5];
	fp32 p2p_msg_[5];
	fp32 rot_msg_[2];
	int count_;

	UxbusCmdTcp *cmd_tcp_;
	UxbusCmdSer *cmd_ser_;
	SocketPort *stream_tcp_;
	SocketPort *stream_tcp_report_;
	SerialPort *stream_ser_;
	Timer timer;

	std::vector<void(*)(const fp32*, const fp32*)> report_location_callbacks_;
	std::vector<void(*)(bool, bool)> connect_changed_callbacks_;
	std::vector<void(*)(int)> state_changed_callbacks_;
	std::vector<void(*)(int)> mode_changed_callbacks_;
	std::vector<void(*)(int, int)> mtable_mtbrake_changed_callbacks_;
	std::vector<void(*)(int, int)> error_warn_changed_callbacks_;
	std::vector<void(*)(int)> cmdnum_changed_callbacks_;
	std::vector<void(*)(const fp32*)> temperature_changed_callbacks_;
	std::vector<void(*)(int)> count_changed_callbacks_;
};

#endif
