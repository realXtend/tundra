// For conditions of distribution and use, see copyright notice in license.txt

/// @file QtUtils.h
/// @brief Cross-platform utility functions using Qt.

#ifndef incl_Foundation_QtUtils_h
#define incl_Foundation_QtUtils_h

#include "CoreTypes.h"
#include <QStringList>

namespace Foundation
{
    class QtUtils
    {
    public:
        /// Default constructor.
        QtUtils() {}

        /// Default destructor.
        virtual ~QtUtils() {}

        /** Opens the OS's open file dialog.
         *  @param filter The files to be shown.
         *  @param caption Dialog's caption.
         *  @param dir Working directory.
         *  @return The absolute path to the chosen file.
         */
        static std::string GetOpenFileName(
            const std::string &filter,
            const std::string &caption,
            const std::string &dir);

        /** Same as GetOpenFileName but for multiple files.
         *  @return List of absolute paths to the chosen files.
         */
        static StringList GetOpenFileNames(
            const std::string &filter,
            const std::string &caption,
            const std::string &dir);

        /** Same as GetOpenFileNames but with hardcoded reX filters.
         *  @return List of absolute paths to the chosen files.
         */
        static StringList GetOpenRexFileNames(const std::string &dir);

        /// Same as GetOpenFileNames but return QStringList instead of std::list<std::string>.
        static QStringList GetOpenRexFilenames(const std::string &dir);

        /// @return The absolute path of the application's current directory.
        static std::string GetCurrentPath();
    };
}

#endif