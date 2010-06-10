# Define target name
init_target (Interfaces)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB MOC_FILES ComponentInterface.h CommunicationsService.h UiSettingsServiceInterface.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

add_definitions (-DMODULE_EXPORTS)

use_package (BOOST)
use_package (POCO)
use_package (QT4)
use_modules (Core Foundation SceneManager Interfaces)

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS})

link_package (BOOST)
link_package (POCO)
link_package (QT4)
link_modules (Core Foundation SceneManager Interfaces)

final_target ()
