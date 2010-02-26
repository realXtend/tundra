// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Framework.h"
#include "Application.h"
#include "ConfigurationManager.h"

namespace Foundation
{

Application::Application(Framework *framework)
{
    framework_ = framework;
}

Application::~Application()
{
    framework_ = 0;
}

std::string Application::Name()
{
    assert (framework_);
    static const std::string app_name("application_name");

    return framework_->GetDefaultConfig().GetSetting<std::string>(Framework::ConfigurationGroup(), app_name);
}

std::string Application::Version()
{
    assert (framework_);
    static const std::string version_major("version_major");
    static const std::string version_minor("version_minor");
    const std::string &group = Framework::ConfigurationGroup();

    return ( framework_->GetDefaultConfig().GetSetting<std::string>(group, version_major) + "." +
            framework_->GetDefaultConfig().GetSetting<std::string>(group, version_minor) );
}

Framework *Application::framework_ = 0;

}
