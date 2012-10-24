// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "LoggingFunctions.h"
#include "zzip/zzip.h"

static bool CheckAndLogZzipError(zzip_error_t error)
{
    QString errorMsg;
    switch (error)
    {
        case ZZIP_NO_ERROR:
            break;
        case ZZIP_OUTOFMEM:
            errorMsg = "Out of memory.";
            break;            
        case ZZIP_DIR_OPEN:
        case ZZIP_DIR_STAT: 
        case ZZIP_DIR_SEEK:
        case ZZIP_DIR_READ:
            errorMsg = "Unable to read zip file.";
            break;            
        case ZZIP_UNSUPP_COMPR:
            errorMsg = "Unsupported compression format.";
            break;            
        case ZZIP_CORRUPTED:
            errorMsg = "Corrupted archive.";
            break;            
        default:
            errorMsg = "Unknown error.";
            break;            
    };

    if (!errorMsg.isEmpty())
        LogError("ZipAssetBundle:" + errorMsg);
    return !errorMsg.isEmpty();
}

static bool CheckAndLogArchiveError(zzip_dir *archive)
{
    if (archive)
        return CheckAndLogZzipError((zzip_error_t)zzip_error(archive));
    return true;
}