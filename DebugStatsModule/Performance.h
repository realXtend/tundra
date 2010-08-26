#ifndef PERFORMANCE_H_
#define PERFORMANCE_H_

#ifdef Q_WS_WIN

#include <pdh.h>
#include <PDHMsg.h>

#endif 

#include <string>
#include <sstream>
#include <iomanip>

#include "GlobalAllocObject.h"

#include <QString>

namespace PDH
{

#ifndef Q_WS_WIN
    
    struct HCOUNTER {};
    typedef unsigned int PDH_STATUS;
    typedef unsigned int DWORD;
    typedef std::string LPCTSTR;

#define ERROR_SUCCESS 1
#define PDH_FMT_DOUBLE 2
#define PDH_FMT_LONG 3

    struct PDF_FMT_COUNTERVALUE 
    { 
        double doubleValue() { return 0.0; } 
    };
    
    struct HQUERY {};
  
    PDF_STATUS PdhGetFormattedCounterValue( void*, DWORD, DWORD&, PDF_FMT_COUNTERVALUE*) { return PDF_STATUS(); }
    void PdhOpenQuery(int, int, HQUERY*) {}
    void PdhCloseQuery(HQUERY query) {}
    PDH_STATUS  PdhAddCounter (HQuery, LPCTSTR, int, Counter&) { return PDH_STATUS(); };
    PDH_STATUS PdhCollectQueryData(HQuery) {  return PDH_STATUS(); };

#endif

/**
 * @brief PerformanceMonitor is as interface class for M$ performance monitor API. 
 */
 
class PerformanceMonitor 
    {
    public: 
        PerformanceMonitor();
        virtual ~PerformanceMonitor();

        /**
         * Reads from M$ performance API that how many threads are used to run this application. 
         * @return viewer thread count. 
         */
        int GetThreadCount();

        /**
         * Reads from M$ performace API that how many disk bytes are average read from all disk. 
         */
        long GetAvgDiskRead();
        
        /**
         * Reads from M$ performace API that how many disk bytes are average write to all disk. 
         */
        long GetAvgDiskWrite();
       

    };


class Counter : public SharedGlobalObject<HCOUNTER>
    {
    public:
        PDH_STATUS GetFormatted(DWORD fmt, PDH_FMT_COUNTERVALUE*  fmtValue) const
        {
            DWORD  ctrType;
            return PdhGetFormattedCounterValue (*(get()->handle()), fmt, &ctrType, fmtValue);
        }
   
       double asDouble() const
       {
          PDH_FMT_COUNTERVALUE   fmtValue;
          PDH_STATUS pdhStatus = GetFormatted(PDH_FMT_DOUBLE, &fmtValue);
          if (pdhStatus != ERROR_SUCCESS) 
            return -1.0;

          return fmtValue.doubleValue ;
       }

       std::string asDoubleString() const 
       {
          std::stringstream st;
          PDH_FMT_COUNTERVALUE fmtValue;
          PDH_STATUS pdhStatus = GetFormatted(PDH_FMT_DOUBLE, &fmtValue);
          if (pdhStatus == ERROR_SUCCESS) 
             st << std::setprecision(20) << fmtValue.doubleValue ;
          else 
              return "DebugStats::Perfomance.h : asDoubleString() error value were not converted";

          return st.str();
       }

       long asLong() const 
       {
          PDH_FMT_COUNTERVALUE fmtValue;
          PDH_STATUS      pdhStatus = GetFormatted(PDH_FMT_LONG, &fmtValue);
           if (pdhStatus != ERROR_SUCCESS) 
             return -1;

          return fmtValue.longValue;
       }

       std::string asLongString() const 
       {
          std::stringstream st;
          PDH_FMT_COUNTERVALUE   fmtValue;
          PDH_STATUS pdhStatus = GetFormatted(PDH_FMT_LONG, &fmtValue);
           if (pdhStatus == ERROR_SUCCESS) 
              st << fmtValue.longValue;
           else 
               return std::string("DebugStats::Perfomance.h : asLongString() error value were not converted");

          return st.str();
       }
    };


class Query
{
       HQUERY h;
    public:
        Query()   
        { 
            if ( PdhOpenQuery (0, 0, &h) != ERROR_SUCCESS )
                std::cout<<" DebugStats::Perfomance.h : Error in opening pdhquery";
           
        }
        ~Query()  { PdhCloseQuery (h); }

       operator HQUERY() { return h;}
       
       Counter AddCounter(TCHAR* objectName, TCHAR* counterName )
       {

          Counter pCounterHandle;

    #ifdef Q_WS_WIN

          
          PDH_COUNTER_PATH_ELEMENTS pdhCpe;
          PDH_STATUS pdhStatus;

          TCHAR szAvailBytes[256] = TEXT("");
          DWORD dwBufferSize = sizeof(szAvailBytes);

          pdhCpe.szMachineName = NULL;
          pdhCpe.szObjectName = objectName;

          pdhCpe.szInstanceName = NULL;
          pdhCpe.szParentInstance = NULL;
          pdhCpe.dwInstanceIndex = -1;
          pdhCpe.szCounterName = counterName;
         
          if ((pdhStatus=PdhMakeCounterPath(&pdhCpe,szAvailBytes,&dwBufferSize, 0)) == ERROR_SUCCESS) 
          {
              pdhStatus = PdhAddCounter (h,szAvailBytes, 0, pCounterHandle);
              if ( pdhStatus != ERROR_SUCCESS )
              {
                  std::cout<<" DebugStats::Perfomance.h : Error counter were not added"<<std::endl;
              }
              
                   
          
          }
#endif
          
          return pCounterHandle;
       }
       
       PDH_STATUS Collect() const 
       {
          return PdhCollectQueryData (h); 
       }
};

}// namespace PDH

#endif //PERFORMANCE_H_