// The Notices and Disclaimers for Ocean Worlds Autonomy Testbed for Exploration
// Research and Simulation can be found in README.md in the root directory of
// this repository.

#ifndef common_commands_H
#define common_commands_H

// All PLEXIL commands available to the lander in both OceanWATERS and
// OWLAT.  This is the lander's command interface.

// Note that in PLEXIL, commands are asynchronous by design.
// Typically, plans should not call these commands directly, but
// instead use the library interface (defined in common-interface.h)
// that calls these commands synchonously.

Command arm_unstow();
Command arm_stow();
Command arm_stop();
Command arm_move_joint (Boolean relative, Integer joint, Real angle);

// Takes an Euler angle for orientation.
Command arm_move_cartesian (Integer frame,
                            Boolean relative,
                            Real position[3],
                            Real orientation[3]);

// Takes a quaternion for orientation.
Command arm_move_cartesian_q (Integer frame,
                              Boolean relative,
                              Real position[3],
                              Real orientation[4]);


#endif
