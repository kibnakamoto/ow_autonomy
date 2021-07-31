// The Notices and Disclaimers for Ocean Worlds Autonomy Testbed for Exploration
// Research and Simulation can be found in README.md in the root directory of
// this repository.

#ifndef Ow_Adapter
#define Ow_Adapter

// PLEXIL Interface adapter for OceanWATERS.

// ow_plexil
#include "CommonAdapter.h"

// PLEXIL
#include <Command.hh>
#include <Value.hh>

#include <set>

using namespace PLEXIL;

class OwAdapter : public CommonAdapter
{
public:
  // No default constructor, only this specialized one.
  OwAdapter (AdapterExecInterface&, const pugi::xml_node&);
  ~OwAdapter ();
  OwAdapter (const OwAdapter&) = delete;
  OwAdapter& operator= (const OwAdapter&) = delete;

  virtual bool initialize();
  virtual void lookupNow (State const& state, StateCacheEntry &entry);
};

extern "C" {
  void initOwAdapter();
}

#endif
