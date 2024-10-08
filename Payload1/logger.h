#include <iostream>
#include <windows.h>
#include <sstream>
#include <string>

#pragma once


// Logger: handles receiving and then writing logs to a file
//===========================================================

class Logger
{	
private:
    std::wstringstream stream;
    FILE* file;
    //HANDLE file;
    

public:
    typedef std::ostream&  (*ManipFn)(std::ostream&);

    // initialize the log file as "<executable name>_log.txt"
    Logger()
    {
        /* Get the filename of the current module. */
        //wchar_t fileName[MAX_PATH + 4]= {0};
        //wchar_t exeName[MAX_PATH]= {0}; 
        //GetModuleFileName(NULL, exeName, MAX_PATH);
        
        char* base_path;
        size_t len;
        errno_t err = _dupenv_s(&base_path, &len, "USERPROFILE");
        char str_ObjectsPath[260];
        memset(&str_ObjectsPath[0], 0, sizeof(str_ObjectsPath));
        strcat_s(str_ObjectsPath, 260, base_path);
        strcat_s(str_ObjectsPath, 260, "\\payload1.txt");
        fopen_s(&file, str_ObjectsPath, "a+");
        delete base_path;
    }
    /*
    // override the << operator to redirect values to our internal stream
    template <typename T> 
    Logger& operator <<(T const& value) 
    {   
        stream << value;
        return *this;
    }

    // override the << operator to detect the std::endl stream manipulation
    Logger& operator<<(ManipFn manip)
    { 
        if (manip == static_cast<ManipFn>(std::endl))
            this->write_log();

        return *this;
    }
    */
    void write_log()
    {
        std::wstring log;
        DWORD logLength;   

        stream << L"\n";
        log= stream.str(); 
        logLength= wcslen(log.c_str())*sizeof(wchar_t);

        WriteFile(this->file, log.c_str(), logLength, NULL, NULL);
        
        stream.str(std::wstring()); // clear the stream
        stream.clear(); // clear errors
    }


    FILE* GetFile()
    {
        return this->file;
    }


    void close()
    {
        fclose(this->file);
    }
};


