
#ifndef SMALL_DLL_LOADER
#define	SMALL_DLL_LOADER
#include "Utils.h"
typedef void(*func_ptr)(void*);

#if defined( _WIN32 )
#include <Windows.h>
#include <strsafe.h>

typedef HMODULE DLLHandle;
void UnloadDLL(DLLHandle* DLLHANDLE)
{
	FreeLibrary(*DLLHANDLE);
	//*DLLHANDLE = 0;
}

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
void PrintLastError()
{
	LPTSTR lpszFunction = TEXT((char*)"GetProcessId");
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError(); 
	if(dw == 0)
	{
		LOG("No errors \n");
		return; //No error message has been recorded
	}

	FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, NULL );

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
			(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
	StringCchPrintf((LPTSTR)lpDisplayBuf, 
			LocalSize(lpDisplayBuf) / sizeof(TCHAR),
			TEXT("%s failed with error %d: %s"), 
			lpszFunction, dw, lpMsgBuf); 
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);

}

int load_DLL(DLLHandle* DLLHANDLE,const char* name)
{

	//if(*DLLHandle != 0){
		UnloadDLL(DLLHANDLE);
	//}
	CopyFile(name, "temp.dll", 0);
	*DLLHANDLE = LoadLibrary("temp.dll");

	if (!(*DLLHANDLE))
	{
		PrintLastError();
		return 0;
	}
	return 1;
}

func_ptr load_DLL_function(DLLHandle DLLHANDLE,const char* name)
{
	return (func_ptr)GetProcAddress(DLLHANDLE, name);
}
#endif // !_WIN32
#ifdef __linux__

#include <fcntl.h>
#include <dlfcn.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
int cp(const char* infile, const char* outfile)
{
/*
M.I.T   N B B E R L I C E N S E
Copyright 2018  eetut

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifdef _WIN32 //|| _WIN64
    //  Windows
    CopyFileA(infile, outfile, false);
#elif __APPLE__
    //  OSX
    copyfile(infile, outfile, NULL, COPYFILE_DATA);
#elif defined(__linux)
    //  Linux
    int read_fd;
    int write_fd;
    struct stat stat_buf;
    off_t offset = 0;

    /* Open the input file. */
    read_fd = open (infile, O_RDONLY);
	if(read_fd == -1){
	 return -1;
	}
    /* Stat the input file to obtain its size. */
    fstat (read_fd, &stat_buf);
    /* Open the output file for writing, with the same permissions as the
       source file. */
    write_fd = open (outfile, O_WRONLY | O_CREAT, stat_buf.st_mode);
    /* Blast the bytes from one file to the other. */
    sendfile (write_fd, read_fd, &offset, stat_buf.st_size);
    /* Close up. */
    close (read_fd);
    close (write_fd);
#endif
	return 0;
}



#if 0
int cp(const char *to, const char *from)
{

    int fd_to, fd_from;
    char buf[4096];
    ssize_t nread;
    int saved_errno;

    fd_from = open(from, O_RDONLY);
    if (fd_from < 0)
        return -1;

    fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd_to < 0)
        goto out_error;

    while (nread = read(fd_from, buf, sizeof buf), nread > 0)
    {
        char *out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = write(fd_to, out_ptr, nread);

            if (nwritten >= 0)
            {
                nread -= nwritten;
                out_ptr += nwritten;
            }
            else if (errno != EINTR)
            {
                goto out_error;
            }
        } while (nread > 0);
    }

    if (nread == 0)
    {
        if (close(fd_to) < 0)
        {
            fd_to = -1;
            goto out_error;
        }
        close(fd_from);

        /* Success! */
        return 0;
    }

  out_error:
    saved_errno = errno;

    close(fd_from);
    if (fd_to >= 0)
        close(fd_to);

    errno = saved_errno;
    return -1;
}
#endif
typedef void* DLLHandle;
 //void * dlopen(const char *filename, int flag);

int load_DLL(DLLHandle* DLLHANDLE,const char* name)
{
	if(cp(name,"./temp.lib") == -1) {
		printf("failed to copy file \n");
		return false;
	}

	*DLLHANDLE = dlopen ("./temp.lib", RTLD_LAZY);
	if(!(*DLLHANDLE))
	{
		fputs (dlerror(), stderr);
		return 0;
	}
	return true;
}

func_ptr load_DLL_function(DLLHandle DLLHANDLE,const char* name)
{
		//CopyFile(name, "temp.dll", 0);
	func_ptr ret = (func_ptr)dlsym(DLLHANDLE, name);
	char* error = NULL;
    if ((error = dlerror()) != NULL)  
	{
        fputs(error, stderr);
        return NULL;
    }
	return ret;
}

void UnloadDLL(DLLHandle* DLLHANDLE)
{

	dlclose(*DLLHANDLE);
	*DLLHANDLE = 0;
}

#endif


#endif // !SMALL_DLL_LOADER
