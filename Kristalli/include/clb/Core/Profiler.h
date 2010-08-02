/** @file Profiler.h
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief The clb::ProfilerSystem namespace provides tools to perform execution time
		profiling and statistics counter calculations. 
*/		

#ifndef Profiler_h
#define Profiler_h

#include <string>
#include <list>
#include <map>

#include "clb/Time/Clock.h"
#include "clb/Core/Console.h"
#include "clb/Container/Tree.h"
#include "clb/Core/Types.h"

namespace clb
{

/// the type of statistics counters and profile block IDs.
typedef unsigned long profileid;

class ProfileBlock;

/// Holds recorded data of a profiling block or statistics counter.
class TreeNode
{
public:
	/** For a profile block, stores the number of clock ticks the block has consumed.
		For a statistic counter, this value is not used. 
		This variable stores "intermediate" results, get the reliable tick
		count from oldTicks instead. */
	tick_t ticks;

	/** If this profile block has been executed several times, these hold the min & 
		max number of ticks that was consumed in a single execution. */
	tick_t minTicks;
	tick_t maxTicks;

	/** For a profile block, stores the number of times the ProfileBlock object was
		created. For a statistic counter, records the current running counter value 
		since	last Reset().
		This variable stores "intermediate" results, get the reliable data from
		oldCalls instead. */
	int calls;

	/** For a profile block, stores the number of clock ticks the block consumed
		between the previous period of two consecutive Reset() calls. ClbLibApp resets
		the counters periodically, so this data should always be a reliable measure
		of ticks spent in the block over the previous elapsed period. 
		For a statistic counter, stores the total counter number since the start of
		application. */
	tick_t oldTicks;
	tick_t oldMinTicks;
	tick_t oldMaxTicks;

	/** For a profile block, stores the number of times the ProfileBlock object was
	created between the previous period of two consecutive Reset() calls. ClbLibApp
	resets the counters periodically, so this data is always "fresh". You can think
	of the oldTicks and oldCalls as "double-buffered" values of ticks and calls
	-variables. For a statistic counter, stores the accumulated counter value over
	the previous elapsed period. */
	int oldCalls;

	/** A unique ID for this node. The highest 8 bits of this value is reserved
		for flags, the lower bits constitute the ID. If this node is a statistic
		counter, then (id & cProfileStatistics) == 0, otherwise the node stores data
		for a ProfileBlock. The cProfileGlobal and cProfileHierarchy bits determine
		in which mode the profiling object is used. */
	profileid id;
};

/** @brief Provides a system-wide realtime performance profiling framework for clbLib and its client.

	Execution time profiling is achieved by creating ProfileBlock objects on the
	stack. ProfileBlock's constructor and destructor mark the current timestamps 
	and update the profiling statistics accordingly. You should avoid creating
	the ProfileBlock objects manually when not necessary, instead use the 
	PROFILE(), GPROFILE() and HPROFILE() macros, e.g.

		void func()
		{
			// start profiling of the whole function.
			PROFILE(myFuncExecutionTime);
			
			// do something..

			{
				PROFILE(customScope); // starts profiling block named "customScope"

				// do something..

			} // timing of "customScope" ends here.
		} // timing of "myFuncExecutionTime" ends here.
	
	Statistics counters are a way of calculating how often something happens. For
	example, the graphics library uses these counters to measure rendering 
	performance (transformed vertices/frame, effect changes/frame etc..) The counters
	are created automatically when execution reaches one, like: 
	
		void func2()
		{
			INCSTATS(myStatsCounter, 10); // increases "myStatsCounter" by 10.
		}

	There are two different modes of collecting perfomance and statistics counter
	data: the hierarchical tree and global mode. When you call PROFILE(), or
	INCSTATS(), the data is recorded in both modes. This is the default functionality.
	When you call HPROFILE() or HINCSTATS(), only the hierarchy tree is considered.
	In the func() above, customScope would now be rendered indented in respect of
	the myFuncExecutionTime block in the log to show the parent-child relationship. 
	The third way to take measures is to call GPROFILE() or GINCSTATS(). This causes
	the samples to be shown at the global level, i.e. the tree is flattened. */
namespace ProfilerSystem
{
	/// Begins a new sampling period by cleaning measured data.
	void Reset();

#ifdef PERFORMANCEPROFILING
	namespace
	{
		/// Increases hierarchical statistics counter value.
		void IncreaseCounterInHierarchy(profileid id, int amount);

		/// Increases global level statistics counter value.
		void IncreaseCounterAtGlobal(profileid id, int amount);

		/// Saves the measured data and resets them for a new sample period.
		void ResetChildren(Tree<TreeNode> &t);

		/// Recursively prints the node tree data to console.
		void PrintChildren(Tree<TreeNode> &t, int level);
	}

	/// Adds a NodeTree to the hierarchy. (ProfileBlock's constructor calls this)
	void AddNodeToHierarchy(profileid id);

	/// Adds a NodeTree at the global level. (ProfileBlock's constructor calls this)
	void AddNodeToGlobal(profileid id);

	/// Removes a NodeTree from the hierarchy (ProfileBlock's destructor calls this)
	void DeleteNodeFromHierarchy(profileid id, tick_t time);

	/// Removes a NodeTree at the global level (ProfileBlock's destructor calls this)
	void DeleteNodeFromGlobal(profileid id, tick_t time);

	/// Increases a statistics counter by the given value.
	void IncreaseCounter(profileid id, int amount);

	/// Prints node tree data to console.
	void Print();

	/// Returns a new unique ID to the profiling system.
	int NewUniqueID();

	/// Returns the ID associated to the given name, or 0 if a gprofile block by that name doesn't exist.
	profileid GetGProfileIDByName(const char *name);

	/// Returns profiling statistics of a global level counter, or 0 if not found.
	const TreeNode *GetGProfileBlockData(const char *name);

	/// Returns profiling statistics of a global level counter, or 0 if not found.
	const TreeNode *GetGProfileBlockData(profileid id);

	/// This flag is set in the IDs of hierarchical counters & ProfileBlocks.
	const int cProfileHierarchy = 0x80000000;

	/// This flag is set in the IDs of global level counters & ProfileBlocks.
	const int cProfileGlobal = 0x40000000;

	/// This flag is set in the ID of the NodeTree if the node is a stats counter.
	const int cProfileStatistics = 0x20000000;

	/// Contains the hierarchy level profiler & statistics data.
	extern Tree<TreeNode> root;

	/// List of global level profiler & statistics data.
	extern std::list<TreeNode> globalBlocks;

	/// Dictionary to convert node ID's to strings.
	extern std::map<profileid, std::string> blockStrings;

	/// Required for its Tick() functionality.
	extern Clock clock;

	/// Points to the currently active ProfileBlock, cached to minimize overhead.
	extern Tree<TreeNode> *current;

	/// Cache the current position in active globalBlocks list (the globalBlocks is accessed as a stack) for performance
	extern std::list<TreeNode>::iterator globalBlocksStackPos;
#endif
}

/** @brief Used to record performance profiling data. 

	A ProfileBlock object lives on the stack and measures the elapsed time until it goes out of scope. Do not
	create objects directly, but through the PROFILE(), GPROFILE() or HPROFILE() -macros. */
class ProfileBlock
{
public:
	inline ProfileBlock(profileid id_):id(id_)
	{
#ifdef PERFORMANCEPROFILING
		// Get the tick when the block enters the stack.
		start = ProfilerSystem::clock.Tick();

		// Add this node to the hierarchy or the global level, depending on the type.
		if (id_ & ProfilerSystem::cProfileHierarchy)
			ProfilerSystem::AddNodeToHierarchy(id);
		if (id_ & ProfilerSystem::cProfileGlobal)
			ProfilerSystem::AddNodeToGlobal(id);
#endif
	}

	inline ~ProfileBlock()
	{
#ifdef PERFORMANCEPROFILING
		// Get the tick when the block is removed from the stack.
		tick_t end = ProfilerSystem::clock.Tick();

		const tick_t elapsedTicks = end - start;

		if (id & ProfilerSystem::cProfileHierarchy)
			ProfilerSystem::DeleteNodeFromHierarchy(id, elapsedTicks);
		if (id & ProfilerSystem::cProfileGlobal)
			ProfilerSystem::DeleteNodeFromGlobal(id, elapsedTicks);
#endif
	}

	/// Saves the tick when the ProfileBlock entered the stack.
	tick_t start;

	/// The unique ID and flags for the ProfileBlock.
	profileid id;
};

#ifdef PERFORMANCEPROFILING

/** Creates a ProfileBlock in the tree hierarchy. 
	@param x The name for the block. Don't enclose in it quotes but do something
		like HPROFILE(myBlock); instead. */
#define HPROFILE(x) \
	static profileid __profiler_ ## x = 0; \
	if (__profiler_ ## x == 0) \
	{ \
		__profiler_ ## x = ProfilerSystem::NewUniqueID() | ProfilerSystem::cProfileHierarchy; \
		ProfilerSystem::blockStrings[__profiler_ ## x] = std::string(#x); \
	} \
	ProfileBlock _p_profiler_ ## x(__profiler_ ## x); 

/** Creates a ProfileBlock at the global level.
	@param x The name for the block. Don't enclose in it quotes but do something
	like GPROFILE(myBlock); instead. */
#define GPROFILE(x) \
	static profileid __profiler_ ## x = 0; \
	if (__profiler_ ## x == 0) \
	{ \
		__profiler_ ## x = ProfilerSystem::NewUniqueID() | ProfilerSystem::cProfileGlobal; \
		ProfilerSystem::blockStrings[__profiler_ ## x] = std::string(#x); \
	} \
	ProfileBlock _p_profiler_ ## x(__profiler_ ## x); 

/** Creates a ProfileBlock both in the hierarchy and at the global level.
	@param x The name for the block. Don't enclose in it quotes but do something
	like PROFILE(myBlock); instead. */
#define PROFILE(x) \
	static clb::profileid __profiler_ ## x = 0; \
	if (__profiler_ ## x == 0) \
	{ \
		__profiler_ ## x = clb::ProfilerSystem::NewUniqueID() | clb::ProfilerSystem::cProfileGlobal | clb::ProfilerSystem::cProfileHierarchy; \
		clb::ProfilerSystem::blockStrings[__profiler_ ## x] = std::string(#x); \
	} \
	clb::ProfileBlock _p_profiler_ ## x(__profiler_ ## x); 

/** Increments a statistics counter in the hierarchy.
	@param name The name for the counter. Don't enclose in it quotes but do something 
	like HINCSTATS(myCounter, 10); instead.
	@param amount The value to increase the counter by. */
#define HINCSTATS(name, amount) \
{ \
	static profileid __counter_ ## name = 0; \
	if (__counter_ ## name == 0) \
	{ \
		__counter_ ## name = ProfilerSystem::NewUniqueID() | \
									ProfilerSystem::cProfileHierarchy | \
									ProfilerSystem::cProfileStatistics; \
		ProfilerSystem::blockStrings[__counter_ ## name] = std::string(#name); \
	} \
	ProfilerSystem::IncreaseCounter(__counter_ ## name, amount); \
}

/** Increments a statistics counter at the global level.
	@param name The name for the counter. Don't enclose in it quotes but do something
	like GINCSTATS(myCounter, 10); instead.
	@param amount The value to increase the counter by. */
#define GINCSTATS(name, amount) \
{ \
	static profileid __counter_ ## name = 0; \
	if (__counter_ ## name == 0) \
	{ \
		__counter_ ## name = ProfilerSystem::NewUniqueID() | \
									ProfilerSystem::cProfileGlobal | \
									ProfilerSystem::cProfileStatistics; \
		ProfilerSystem::blockStrings[__counter_ ## name] = std::string(#name); \
	} \
	ProfilerSystem::IncreaseCounter(__counter_ ## name, amount); \
}

/** Increments a statistics counter in the hierarchy and at the global level.
	@param name The name for the counter. Don't enclose in it quotes but do something
	like INCSTATS(myCounter, 10); instead.
	@param amount The value to increase the counter by. */
#define INCSTATS(name, amount) \
{ \
	static profileid __counter_ ## name = 0; \
	if (__counter_ ## name == 0) \
	{ \
		__counter_ ## name = ProfilerSystem::NewUniqueID() | \
									ProfilerSystem::cProfileGlobal | \
									ProfilerSystem::cProfileHierarchy | \
									ProfilerSystem::cProfileStatistics; \
		ProfilerSystem::blockStrings[__counter_ ## name] = std::string( #name ); \
	} \
	ProfilerSystem::IncreaseCounter(__counter_ ## name, amount); \
}

#else // PERFORMANCEPROFILING undefined:
#define HPROFILE(x)
#define GPROFILE(x)
#define PROFILE(x)
#define HINCSTATS(a,b)
#define GINCSTATS(a,b)
#define INCSTATS(a,b)
#endif // ~PERFORMANCEPROFILING

}

#endif
