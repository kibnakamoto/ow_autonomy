// The Notices and Disclaimers for Ocean Worlds Autonomy Testbed for Exploration
// Research and Simulation can be found in README.md in the root directory of
// this repository.

#ifndef Common_Adapter
#define Common_Adapter

// PLEXIL Interface adapter class to be specialized for each testbed.

// PLEXIL API
#include "InterfaceAdapter.hh"
#include "Command.hh"
#include "Value.hh"

#include <set>

using namespace PLEXIL;

class CommonAdapter : public InterfaceAdapter
{
public:
  // No default constructor, only this specialized one.
  CommonAdapter (AdapterExecInterface&, const pugi::xml_node&);
  ~CommonAdapter ();
  CommonAdapter (const CommonAdapter&) = delete;
  CommonAdapter& operator= (const CommonAdapter&) = delete;

  virtual bool initialize();
  virtual bool start();
  virtual bool stop();
  virtual bool reset();
  virtual bool shutdown();
  virtual void invokeAbort(Command *cmd);
  virtual void subscribe(const State& state);
  virtual void unsubscribe(const State& state);
  void propagateValueChange (const State&, const std::vector<Value>&) const;

protected:
  bool isStateSubscribed(const State& state) const;
  std::set<State> m_subscribedStates;
};

#endif
