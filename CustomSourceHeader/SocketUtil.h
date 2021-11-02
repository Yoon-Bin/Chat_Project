#pragma once

#if _WIN64
#define SOCK_OPTNAME_DLL_PATH ("../CustomDll/64bit/SockOptNameStringTable.dll")
#else
#define SOCK_OPTNAME_DLL_PATH ("../CustomDll/32bit/SockOptNameStringTable.dll")
#endif

//#define SOCK_OPTNAME_DLL_PATH ("../CustomDll/64bit/SockOptNameStringTable.dll")
static std::string GetLastErrorAsString()
{
    //Get the error message ID, if any.

    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        return std::string("No Error"); //No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);
    
    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);

    return message;
}

static std::string GetSockOptNameAsString(const int optname)
{
    CHAR strText[25] = { 0, };
    HINSTANCE hdll = NULL;
    std::string ret;

    hdll = ::LoadLibraryA(SOCK_OPTNAME_DLL_PATH);

    if (hdll != NULL)
    {
        LoadStringA(hdll, optname, strText, 25);

        ret = strText;

        ::FreeLibrary(hdll);
    }
    else
    {
        ret = "SOCK_OPTNAME_DLL NOT LOADED";
    }

    return ret;
}

//static string GetLastErrorAsString(int optname)
//{
//    //Get the error message ID, if any.
//    stringstream ss;
//
//    DWORD errorMessageID = ::GetLastError();
//    if (errorMessageID == 0) {
//        return std::string(); //No error message has been recorded
//    }
//
//    LPSTR messageBuffer = nullptr;
//
//    //Ask Win32 to give us the string version of that message ID.
//    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
//    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
//        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
//
//    //Copy the error message into a std::string.
//    std::string message(messageBuffer, size);
//
//    ss << sockOptTable[optname].c_str() << message.c_str();
//
//    //Free the Win32's string's buffer.
//    LocalFree(messageBuffer);
//
//    return ss.str();
//}
//
//static string GetSockOptnameAsString(int optname)
//{
//    return sockOptTable[optname];
//}