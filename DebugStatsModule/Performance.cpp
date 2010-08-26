
#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Performance.h"

#include "MemoryLeakCheck.h"

namespace PDH
{

    PerformanceMonitor::PerformanceMonitor()
    {}

    PerformanceMonitor::~PerformanceMonitor()
    {}

    int PerformanceMonitor::GetThreadCount()
    {
        Query hQuery;
#ifdef _DEBUG
        Counter counter =  hQuery.AddCounter(TEXT("Process(viewerd)"), TEXT("Thread Count"));
#else
        Counter counter =  hQuery.AddCounter(TEXT("Process(viewer)"), TEXT("Thread Count"));
#endif
        hQuery.Collect();
       
        return int(counter.asLong()); 
    }

    long PerformanceMonitor::GetAvgDiskRead()
    {
        Query hQuery;
        
        Counter counter =  hQuery.AddCounter(TEXT("PhysicalDisk(_Total)"), TEXT("Avg. Disk Bytes/Read"));
        hQuery.Collect();
       
        return counter.asLong(); 

        
    }
            
            
    long PerformanceMonitor::GetAvgDiskWrite()
    {
       Query hQuery;
        
       Counter counter =  hQuery.AddCounter(TEXT("PhysicalDisk(_Total)"), TEXT("Avg. Disk Bytes/Write"));
       hQuery.Collect();
       
       return counter.asLong(); 

    }

}