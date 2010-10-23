/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   DoxygenDocReader.h
 *  @brief  Provides an utility function for extracting html documentation blocks for class symbols from the generated doxygen documentation files.
 */

#ifndef incl_ECEditorModule_DoxygenDocReader_h
#define incl_ECEditorModule_DoxygenDocReader_h

#include <QString>

namespace DoxygenDocReader
{
    /// Tries to read the documentation for the given functionName from the doxygen-generated documentation
    /// that is stored in the folder doxygenDocDirectory.
    /// @param doxygenDocDirectory The directory that contains the doxygen documentation. The doxygen docs must have been
    ///          generated as a flat list, i.e. without folders.
    /// @param functionName The function to find documentation for. Only functions in a class are supported, and the full 
    ///          namespace qualifier of the class must be specified. The return value and parameters are stripped if they are
    ///          present, so it doesn't matter if you call 'void Foo::foo(int,int)' or 'Foo::foo'. The latter form is preferred 
    ///          (without any whitespaces) to avoid the symbol name parsing getting confused. Function overloads are not 
    ///          currently supported, and only the first found function is returned.
    /// @param documentationOut [out] The returned documentation block is returned here as html web content. Do not pass in
    ///          a zero pointer.
    /// @return True if the extraction succeeded, or false otherwise. If this function returns false, documentationOut
    ///          will contain a displayable error message in html form.
    /// Known issues: The symbol parser does not do any king of proper C++ symbol name tokenization and parsing, but only
    /// performs simple string matching. Therefore as an example, calling with functionName=="void *Foo::foo(int)" will cause
    /// it to look for a class named '*Foo' and not 'Foo'. If a parameter list is present, make sure it does not contain
    /// multiple parentheses (e.g. an embedded macro), or the extraction will not work. The best result can be guaranteed if
    /// functionName does not contain the return value or the parameter list (no parentheses at all), and there are no whitespaces
    /// in the string.
    bool GetSymbolDocumentation(QString doxygenDocDirectory, QString functionName, QString *documentationOut);

    /// Tries to read the documentation for the given function name, using a set of built-in locations for the doxygen docs.
    /// @see GetSymbolDocumentation(QString doxygenDocDirectory, QString functionName, QString *documentationOut) "" for
    /// the parameters.
    /// @param docPathOut [out] This will return the path where the documentation was found, if it was found.
    bool GetSymbolDocumentation(QString functionName, QString *documentationOut, QUrl *docPathOut);
}


#endif
