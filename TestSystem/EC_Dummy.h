
#ifndef __incl_TestSystem_EC_Dummy_h__
#define __incl_TestSystem_EC_Dummy_h__

#include "ComponentInterface.h"
#include "Foundation.h"

namespace Test
{
    //! dummy componenent
    class EC_Dummy : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_Dummy);

    public:
        EC_Dummy &operator= (const EC_Dummy &other)
        {
            if (this != &other) // No self assignment
            {
                mDummyData1 = other.mDummyData1;
                mDummyData2 = other.mDummyData2;
            }

            return *this;
        }

        virtual ~EC_Dummy() {}
        
        int getDummyData1() { return mDummyData1; }
        void setDummyData1(int data) { mDummyData1 = data; }

        int getDummyData2() { return mDummyData2; }
        void setDummyData2(int data) { mDummyData2 = data; }
    private:
        //! default constructor, private since we implicitly use factory
        EC_Dummy() {}
        //! copy constructor, private since we implicitly use factory
        EC_Dummy(const EC_Dummy &rhs) : mDummyData1(rhs.mDummyData1), mDummyData2(rhs.mDummyData2) {}

        int mDummyData1;
        int mDummyData2;
    };
}

#endif
