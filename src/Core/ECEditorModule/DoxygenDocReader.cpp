/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   DoxygenDocReader.cpp
 *  @brief  Provides an utility function for extracting html documentation blocks for class symbols from the generated doxygen documentation files.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "LoggingFunctions.h"

#include "DoxygenDocReader.h"

#include "MemoryLeakCheck.h"

static QString GuessClassDocumentationFilename(QString functionName)
{
    // Escape all underscores. The doxygen filenames use underscore as the escape character.
    functionName.replace("_", "__");

    // The doxygen filenames also espace uppercase letters to lowercase ones. So, if we have an uppercase letter, say 'G', replace it with "_g".
    for(int i = functionName.length()-1; i >= 0; --i)
        if (functionName[i].isUpper())
            functionName.replace(i, 1, QString("_") + functionName[i].toLower());

    // The namespace scope is also escaped with the following special sequences.
    functionName.replace("::", "_1_1");

    QString filename = QString("class") + functionName + ".html";

    return filename;
}

// Removes the return value of the function, i.e. 'void Foo::foo()' -> 'Foo::foo()'
static QString StripSymbolReturnValue(QString symbol)
{
    if (symbol.contains(" "))
        symbol = symbol.right(symbol.length() - symbol.indexOf(" "));
    return symbol.trimmed();
}

// Removes the parameter list of the function, i.e. 'void Foo::foo()' -> 'void Foo::foo'
static QString StripSymbolParameterList(QString symbol)
{
    if (symbol.contains("("))
        symbol.truncate(symbol.lastIndexOf("("));
    return symbol.trimmed();
}

// Guesses the HTML filename where doxygen stores the documentation for the given function in.
static QString GuessFunctionDocumentationFilename(QString functionName)
{
    functionName = functionName.trimmed();

    // The functionName must be a full namespace-qualified name without spaces or the signatures.
    if (functionName.contains("::"))
        functionName.truncate(functionName.lastIndexOf("::"));
    functionName = StripSymbolReturnValue(functionName);

    return GuessClassDocumentationFilename(functionName.trimmed());
}

QString ErrorDocumentation(QString doxygenDocDirectory, QString expectedDocFilename, QString functionName, QString errorReason)
{
    QString documentation = 
        "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"
        "<html xmlns=\"http://www.w3.org/1999/xhtml\">"
        "<head>"
        "<meta http-equiv=\"Content-Type\" content=\"text/xhtml;charset=UTF-8\"/>"
        "<title>RealXtend Viewer: Framework Class Reference</title>"
        "<link href=\"tabs.css\" rel=\"stylesheet\" type=\"text/css\"/>"
        "<link href=\"doxygen.css\" rel=\"stylesheet\" type=\"text/css\"/>"
        "</head>"
        "<body>" 
        "<div class=\"memitem\">"
        "<div class=\"memproto\">"
              "<table class=\"memname\">"
                "<tr>"
                  "<td class=\"memname\">" + functionName + "</td>"
                "</tr>"
              "</table>"
        "</div>"
        "<div class=\"memdoc\">"
        "<p>" + QString("Failed to find documentation from directory \"") + doxygenDocDirectory + "\" and file \"" + expectedDocFilename + 
        "\" for function \"" + functionName + "\". Reason: " + errorReason + "</p>"
        "</div>"
        "</div>"        
        "</body>"
        "</html>";

    return documentation;
}

bool DoxygenDocReader::GetSymbolDocumentation(QString doxygenDocDirectory, QString functionName, QString *documentationOut)
{
    if (!documentationOut)
        return false;

    // Do a bit of manual whitespace coalescing to allow the rest of this function be able to assume "whitespace==spacebar".
    functionName.replace('\t', ' ');
    functionName.replace('\n', ' ');
    functionName.replace('\r', ' ');

    functionName = StripSymbolParameterList(functionName);

    functionName.replace('*', ' '); // Do a simple hack to avoid the case "void *Foo::foo" -> class name == "*Foo".

    ///\todo proper cross-platform path lookup.
    QFile file(doxygenDocDirectory + "/" + GuessFunctionDocumentationFilename(functionName));
    LogDebug("Trying to open file: " + doxygenDocDirectory + "/" + GuessFunctionDocumentationFilename(functionName));

    if (!file.open(QIODevice::ReadOnly))
    {
        *documentationOut = ErrorDocumentation(doxygenDocDirectory, GuessFunctionDocumentationFilename(functionName), functionName, "Failed to open file.");
        return false;
    }

    QByteArray contents = file.readAll();

    functionName = StripSymbolReturnValue(functionName);

    int doxytagStart = contents.indexOf("<!-- doxytag: member=\"" + functionName + "\"");
    if (doxytagStart == -1)
    {
        *documentationOut = ErrorDocumentation(doxygenDocDirectory, GuessFunctionDocumentationFilename(functionName), functionName, "Failed to find doxytag in file.");
        return false;
    }

    // The documentation we want starts at the position 'doxytagStart'. Now find how far we want to extract from there.
    int nextDoxytagStart = contents.indexOf("<!-- doxytag: member=", doxytagStart+1);
    if (nextDoxytagStart == -1) // Next tag was not found, meaning we hit the end of the document. Take everything.
        nextDoxytagStart = contents.length();

    // Now we just generate the final documentation string.

    ///\todo We should just extract this data from the doc file as well to get any changes to the css.
    *documentationOut = 
        "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"
        "<html xmlns=\"http://www.w3.org/1999/xhtml\">"
        "<head>"
        "<meta http-equiv=\"Content-Type\" content=\"text/xhtml;charset=UTF-8\"/>"
        "<title>RealXtend Viewer: Framework Class Reference</title>"
        "<link href=\"tabs.css\" rel=\"stylesheet\" type=\"text/css\"/>"
        "<link href=\"doxygen.css\" rel=\"stylesheet\" type=\"text/css\"/>"
        "</head>"
        "<body>" 
        + contents.mid(doxytagStart, nextDoxytagStart - doxytagStart) +
        "</body>"
        "</html>";

    return true;
}

bool DoxygenDocReader::GetSymbolDocumentation(QString functionName, QString *documentationOut, QUrl *docPathOut)
{
    const char documentationPath1[] = "../doc/html"; // In the git build sources, the doxygen docs are generated to this directory relative to the /bin.
    const char documentationPath2[] = "./dox"; // In compiled builds, let's store the documentation here.
    // Idea: Could add here as a third path the http://www.realxtend.org/doxygen/ url, and use online documentation if no local one is installed.

    QString docPath = documentationPath2;
    bool success = GetSymbolDocumentation(documentationPath1, functionName, documentationOut);
    if (!success)
    {
        success = GetSymbolDocumentation(documentationPath2, functionName, documentationOut);
        //docPath = documentationPath2;
    }

    QDir dir(docPath);
    dir.makeAbsolute();
    QUrl styleSheetPath = QUrl::fromLocalFile(dir.path() + "/");
    if (docPathOut)
        *docPathOut = styleSheetPath;

    return success;
}
