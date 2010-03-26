// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_LLSDUtilities_h
#define incl_LLSDUtilities_h

#include <string>
#include <map>

namespace RexTypes
{
    //! Advances to a named tag
    /*! \param tag Tag to search for
        \param data String data to search in
        \param index Current position in data, will be modified
        \return True if tag was found, false if end was reached without finding the tag
     */
    bool GetNextLLSDTag(const std::string& tag, const std::string& data, std::size_t& index);

    //! Advances to next tag and returns it
    /*! \param data String data to search in
        \param index Current position in data, will be modified
        \return Tag name + its attributes, or empty string if no more tags found
     */
    std::string GetNextLLSDTag(const std::string& data, std::size_t& index);

    //! Gets contents of the current tag
    /*! \param data String data to search in
        \param index Current position in data, will be modified
        \return Tag contents
     */
    std::string GetLLSDTagContent(const std::string& data, std::size_t& index);
    
    //! Parses a map of key,value pairs
    /*! \param data String data to construct a map from
        \return Map
     */
    std::map<std::string, std::string> ParseLLSDMap(const std::string& data);
}

#endif
