// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CoreIntervalTimer_h
#define incl_CoreIntervalTimer_h

//! Delayed repeat timer.
/*! Can be used f.ex. to repeat key strokes.
    
    The timer has three important intervals, long, short and in-between.
    The timer starts out with the long interval and after in-between time
    has elapsed, it switches to short interval.
*/
class IntervalTimer
{
public:
    //! default constructor
    IntervalTimer() : long_interval_(0), short_interval_(0), interval_between_(0), elapsed_(0), elapsed_total_(0), delay_(0) { }
    IntervalTimer(const IntervalTimer &rhs) : 
        short_interval_(rhs.short_interval_)
      , long_interval_(rhs.long_interval_)
      , interval_between_(rhs.interval_between_)
      , delay_(rhs.delay_)
      , elapsed_(0), elapsed_total_(0)
    { }
   
    //! destructor
    ~IntervalTimer() {}

    IntervalTimer &operator =(const IntervalTimer &rhs)
    {
        if (this != &rhs)
        {
            long_interval_ = rhs.long_interval_;
            short_interval_ = rhs.short_interval_;
            interval_between_ = rhs.interval_between_;
            delay_ = rhs.delay_;
        }
        return *this;
    }

    //! reset counter to specified values, values in seconds
    void Reset(f64 long_interval, f64 short_interval, f64 interval_between)
    {
        long_interval_ = long_interval;
        short_interval_ = short_interval;
        interval_between_ = interval_between;
        Reset();
    }

    void Reset()
    {
        elapsed_ = 0;
        elapsed_total_ = 0;
        delay_ = long_interval_;
    }

    //! Tick the timer. Returns true if interval has passed.
    /*!
        \param elapsed_time elapsed time in seconds since last call
    */
    bool Tick(f64 elapsed_time)
    {
        elapsed_ += elapsed_time;
        elapsed_total_ += elapsed_time;
        if (elapsed_total_ > interval_between_)
        {
            elapsed_total_ = 0;
            delay_ = short_interval_;
        }

        if (elapsed_ > delay_)
        {
            elapsed_ = 0;
            
            return true;
        }
        return false;
    }

public:
    //! elapsed time
    f64 elapsed_;

    //! Total elapsed time since last reset
    f64 elapsed_total_;

    //! current delay
    f64 delay_;

    //! long interval between ticks
    f64 long_interval_;

    // Short interval between ticks
    f64 short_interval_;

    //! Delay between long and short interval
    f64 interval_between_;
};

#endif
