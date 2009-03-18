// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ConsoleServiceInterface_h
#define incl_Interfaces_ConsoleServiceInterface_h

#include "ServiceInterface.h"

namespace Foundation
{
    namespace Console
    {
        struct CommandResult
        {
            //! Set to true if command was completed succesfully, false otherwise
            bool success_;
            //! Print out reason for failure (or success).
            std::string why_;
            //! Set true for delayed execution. Set true if you wish threadsafe execution of the command.
            bool delayed_;
        };
        //! Returns a succesful CommandResult
        __inline static CommandResult ResultSuccess(const std::string &why = std::string()) { CommandResult result = { true, why, false }; return result; }
        //! Returns a failure CommandResult
        __inline static CommandResult ResultFailure(const std::string &why = std::string()) { CommandResult result = { false, why, false }; return result; }
        //! Returns a delayed CommandResult
        __inline static CommandResult ResultDelayed() { CommandResult result = { false, std::string(), true }; return result; }

        //! Interface for console command callback
        class CallbackInterface
        {
        public:
            CallbackInterface() {}
            virtual ~CallbackInterface() {}

            virtual CommandResult operator()(const Core::StringVector &params) = 0;
        };
        typedef boost::shared_ptr<CallbackInterface> CallbackPtr;

        //! functor for console commands
        template <typename T>
        class Callback : public CallbackInterface
        {
        public:
            typedef CommandResult (T::*CallbackFunction)(const Core::StringVector&);

           //! constructor taking object and member function pointer
           Callback(T *object, CallbackFunction function) : Object(object), function_(function) { }

           //! destructor
           virtual ~Callback() {}

           //! copy constructor
           Callback(const Callback &rhs)
           {
              this->Object = rhs.Object;
              this->FunctionPtr = rhs.FunctionPtr;
           }

           //! Calls the function
           virtual CommandResult operator()(const Core::StringVector &params)
           {
              return (*Object.*function_)(params);
           }       

        private:
           //! pointer to the object
           T *Object;
           //! pointer to the member function
           CallbackFunction function_;
        };

        //! A console command
        struct Command
        {
            //! internal name for the command, case insensitive
            std::string name_;
            //! short description of the command
            std::string description_;
            //! callback for the command
            CallbackPtr callback_;
        };

        //! Bind a member function to a command callback
        template <typename T>
        static CallbackPtr Bind(T *object, typename Callback<T>::CallbackFunction function)
        {
            return CallbackPtr(new Callback<T>(object, function));
        }

        //! Interface for scene managers
        class ConsoleServiceInterface : public ServiceInterface
        {
        public:
            //! default constructor
            ConsoleServiceInterface() {}

            //! destructor
            virtual ~ConsoleServiceInterface() {}

            //! Add a command to the debug console
            virtual void RegisterCommand(const Command &command) = 0;

            //! Parse and execute command line
            /*! 
                Threadsafe
            */
            virtual CommandResult ExecuteCommand(const std::string &commandline) = 0;

            //! Execute command
            /*! Threadsafe
                
                \param name Name of the command to execute
                \param params Parameters to pass to the command
            */
            virtual CommandResult ExecuteCommand(const std::string &name, const Core::StringVector &params) = 0;
        };
    }
}

#endif

