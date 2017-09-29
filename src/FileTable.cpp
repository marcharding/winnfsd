#include "FileTable.h"
#include <string.h>
#include <io.h>
#include <stdio.h>
#include <windows.h>
#include <sys/stat.h>
#include <vector>
#include <string>
#include <iostream>
#include <string>
#include "pathcch.h" 
#pragma comment(lib, "pathcch.lib")

#define FHSIZE 32
#define NFS3_FHSIZE 64

static CFileTable g_FileTable;

// FILEID-16CHARS-32HEX|VOLUMEID8HEX4CHAR
std::unordered_map<unsigned char*, char*> FileIdentifierToPathMap;
std::unordered_map<char*, unsigned char*> PathToFileIdentifierMap;
std::unordered_map<char*, long long> FileIdentifierLongLognToPathMap;

CFileTable::CFileTable()
{

}

CFileTable::~CFileTable()
{
 
}

unsigned long long CFileTable::GetIDByPath(char *path)
{
	path = GetRealPathPath(path);

	if (FileIdentifierLongLognToPathMap.count(path)) {
		return FileIdentifierLongLognToPathMap[path];
	}

	return GetFileIdentifierByPathLongLong(path);
	unsigned char *handle;

	// printf("\n\nPATH %s\n", path);
	path = GetRealPathPath(path);

	
//	printf("REAL %s\n\n", path);
	handle = GetFileIdentifierByPath(path);
	if (handle == NULL) {
		return 0;
	}

	/*
	unsigned long long returnfalse[NFS3_FHSIZE];
	
	memset(returnfalse, 0, NFS3_FHSIZE * sizeof(unsigned char));
	memcpy(returnfalse, handle, NFS3_FHSIZE * sizeof(unsigned char));
	
	return returnfalse;
	*/


	/** drive letter in unc \\?\D:\ */
	unsigned char * letter[16];

	/** file id 32 hex chars */
	unsigned char * fileId[32];

	memcpy_s(fileId, sizeof(fileId), handle, 32);
	memcpy_s(letter, sizeof(letter), handle + 32, 16);

	unsigned long long ulli2 = strtoull(reinterpret_cast<const char*>(fileId), NULL, 16);
	printf("REAL %032llx\n\n", ulli2);
    return ulli2;
}

unsigned char *CFileTable::GetHandleByPath(char *path)
{
	return GetFileIdentifierByPath(path);
}

char *CFileTable::GetPathByHandle(unsigned char *handle)
{
	return GetPathByFileIdentifier(handle);
}

FILE_ITEM CFileTable::FindItemByPath(char *path)
{
	return GetFileItemFromPath(path);
}

FILE_ITEM CFileTable::GetItemByID(unsigned char *nID)
{
	return GetFileItemFromPath(GetPathByFileIdentifier(nID));
}

bool FileExists(char *path)
{
    int handle;
    struct _finddata_t fileinfo;

    handle = _findfirst(path, &fileinfo);
    _findclose(handle);

    return handle == -1 ? false : strcmp(fileinfo.name, strrchr(path, '\\') + 1) == 0;  //filename must match case
}

unsigned long long GetFileID(char *path)
{
    return g_FileTable.GetIDByPath(path);
}

unsigned char *GetFileHandle(char *path)
{
    return g_FileTable.GetHandleByPath(path);
}

char *GetFilePath(unsigned char *handle)
{
    return g_FileTable.GetPathByHandle(handle);
}

int RenameFile(char *pathFrom, char *pathTo)
{
	// must be done this way so the new filename gehts a new id, nut ossibsle aotherwise with nfs
	// as a subseqend getattr call with the old file name MUST fail but doenst becuase the file id is still the ranem when using just rename
	// CopyFile(pathFrom, pathTo, 0);
	errno_t errorNumber = rename(pathFrom, pathTo);
	// errno_t errorNumber = remove(pathFrom);
	if (errorNumber == 0) { //success
		// FileIdentifierToPathMap[PathToFileIdentifierMap[pathFrom]] = pathTo;
		// PathToFileIdentifierMap[pathTo] = PathToFileIdentifierMap[pathFrom];
		pathFrom = GetRealPathPath(pathFrom);
		FileIdentifierToPathMap.erase(PathToFileIdentifierMap[pathFrom]);
		PathToFileIdentifierMap.erase(pathFrom);
		FileIdentifierLongLognToPathMap.erase(pathFrom);
		
		return errorNumber;
	}
	else {
		return errorNumber;
	}
}

int RenameDirectory(char *pathFrom, char *pathTo)
{
	errno_t errorNumber = RenameFile(pathFrom, pathTo);

	const char* dotFile = "\\.";
	const char* backFile = "\\..";

	char* dotDirectoryPathFrom;
	char* dotDirectoryPathTo;
	char* backDirectoryPathFrom;
	char* backDirectoryPathTo;
	dotDirectoryPathFrom = (char *)malloc(strlen(pathFrom) + 1 + 3);
	strcpy_s(dotDirectoryPathFrom, (strlen(pathFrom) + 1), pathFrom);
	strcat_s(dotDirectoryPathFrom, (strlen(pathFrom) + 5), dotFile);

	dotDirectoryPathTo = (char *)malloc(strlen(pathTo) + 1 + 3);
	strcpy_s(dotDirectoryPathTo, (strlen(pathTo) + 1), pathTo);
	strcat_s(dotDirectoryPathTo, (strlen(pathTo) + 5), dotFile);

	backDirectoryPathFrom = (char *)malloc(strlen(pathFrom) + 1 + 4);
	strcpy_s(backDirectoryPathFrom, (strlen(pathFrom) + 1), pathFrom);
	strcat_s(backDirectoryPathFrom, (strlen(pathFrom) + 6), backFile);

	backDirectoryPathTo = (char *)malloc(strlen(pathTo) + 1 + 4);
	strcpy_s(backDirectoryPathTo, (strlen(pathTo) + 1), pathTo);
	strcat_s(backDirectoryPathTo, (strlen(pathTo) + 6), backFile);

	pathFrom = GetRealPathPath(pathFrom);
	FileIdentifierToPathMap.erase(PathToFileIdentifierMap[pathFrom]);
	PathToFileIdentifierMap.erase(pathFrom);
	FileIdentifierLongLognToPathMap.erase(pathFrom);

	return errorNumber;
}

bool RemoveFile(char *path)
{
	int nMode = 0;
	nMode |= S_IREAD;
	nMode |= S_IWRITE;
	_chmod(path, nMode);

	unsigned char * fileHandle = GetFileIdentifierByPath(path);

    if (remove(path) == 0){
		path = GetRealPathPath(path);
		FileIdentifierToPathMap.erase(PathToFileIdentifierMap[path]);
		PathToFileIdentifierMap.erase(path);
		FileIdentifierLongLognToPathMap.erase(path);
		return true;
    }
    return false;
}

int RemoveFolder(char *path)
{
	int nMode = 0;
	unsigned long errorCode = 0;
	nMode |= S_IREAD;
	nMode |= S_IWRITE;
	_chmod(path, nMode);

    if (RemoveDirectory(path) != 0) {
        const char* dotFile = "\\.";
        const char* backFile = "\\..";

        char* dotDirectoryPath;
        char* backDirectoryPath;
        dotDirectoryPath = (char *)malloc(strlen(path) + 1 + 3);
        strcpy_s(dotDirectoryPath, (strlen(path) + 1), path);
        strcat_s(dotDirectoryPath, (strlen(path) + 5), dotFile);
        backDirectoryPath = (char *)malloc(strlen(path) + 1 + 4);
        strcpy_s(backDirectoryPath, (strlen(path) + 1), path);
        strcat_s(backDirectoryPath, (strlen(path) + 6), backFile);

		path = GetRealPathPath(path);
		FileIdentifierToPathMap.erase(PathToFileIdentifierMap[path]);
		PathToFileIdentifierMap.erase(path);
		FileIdentifierLongLognToPathMap.erase(path);
        return 0;
    }
    errorCode = GetLastError();
    return errorCode;
}

char* GetPathByFileIdentifier(unsigned char *handle)
{
	
	if (FileIdentifierToPathMap.count(handle) && FileIdentifierToPathMap[handle] != NULL) {
		printf("GetPathByFileIdentifier cached: %s", FileIdentifierToPathMap[handle]);
		return FileIdentifierToPathMap[handle];
	}

	
	/** drive letter in unc \\?\D:\ */
	unsigned char * letter[8];

	/** file id 32 hex chars */
	unsigned char * fileId[32];

	memcpy_s(fileId, sizeof(fileId), handle, 32);
	memcpy_s(letter, sizeof(letter), handle + 32 , 8);

	long long index = strtoull(reinterpret_cast<const char*>(fileId), NULL, 16);

	// long long index = strtoull(reinterpret_cast<const char*>(handle), NULL, 16);
	// printf("LETTEr from index: %i \n\n", sizeof(index));
	// printf("LETTEr from handele _%s_", letter);

	HANDLE hDisk, hFile;
	FILE_ID_DESCRIPTOR fileIDDesc;
	
	hDisk = CreateFile(reinterpret_cast<const char*>(letter), FILE_READ_EA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_DIRECTORY | FILE_FLAG_BACKUP_SEMANTICS, NULL);

	// printf("HANDLE %i", hDisk);

	fileIDDesc.dwSize = 24;              // expected by OpenFileById

	fileIDDesc.FileId.QuadPart = index;         // FileID
	fileIDDesc.Type = FileIdType; // enum value

	hFile = OpenFileById(hDisk,
		&fileIDDesc,
		FILE_READ_EA,
		FILE_SHARE_READ,
		NULL,
		0);
	
	// printf("HANDLE %i", hFile);

	TCHAR Path[MAX_PATH];

	DWORD dwRet;

	dwRet = GetFinalPathNameByHandle(hFile, Path, MAX_PATH, VOLUME_NAME_DOS);
/*
	if (dwRet < MAX_PATH)
	{
		// printf("FINAL %s\n\n", Path);
	}
	*/

	if (hFile == INVALID_HANDLE_VALUE) {
		return false;
	}
	
	CloseHandle(hDisk);
	CloseHandle(hFile);

	std::string str(Path);
	char* chr = _strdup(str.c_str());
	// printf("FINAL %s\n\n", chr);

	/*
	if (!PathToFileIdentifierMap.count(chr)) {
		PathToFileIdentifierMap.insert(std::make_pair(chr, handle));
	}
	if (!FileIdentifierToPathMap.count(handle)) {
		FileIdentifierToPathMap.insert(std::make_pair(handle, chr));
	}
	*/

	return chr;

	// TODO: https://stackoverflow.com/questions/23777349/how-to-get-volume-serial-number
	// https://stackoverflow.com/questions/3091301/how-to-get-file-path-from-ntfs-index-number !!!
}


// this is normally only called one, after the file handle is used
unsigned char* GetFileIdentifierByPath(char *path)
{
	// get the real file path (resolve .. etc.)
	path = GetRealPathPath(path);

	if (PathToFileIdentifierMap.count(path) && PathToFileIdentifierMap[path] != NULL) {
		printf("GetPathByFileIdentifier cached: %llx", PathToFileIdentifierMap[path]);
		return PathToFileIdentifierMap[path];
	}

	// get the volumne name
	char volumeNameBuffer[8];
	GetVolumePathName(path, volumeNameBuffer, MAX_PATH);
	
	unsigned long long fileId = GetFileIdentifierByPathLongLong(path);

	

	
	char buffer[NFS3_FHSIZE];
	_snprintf_s(buffer, NFS3_FHSIZE, "%032llx%s", fileId, volumeNameBuffer);

	unsigned char *handle;
	handle = new unsigned char[NFS3_FHSIZE];
	memset(handle, 0, NFS3_FHSIZE * sizeof(unsigned char));
	memcpy(handle, buffer, NFS3_FHSIZE * sizeof(unsigned char));

	PathToFileIdentifierMap.insert(std::make_pair(path, handle));
	FileIdentifierToPathMap.insert(std::make_pair(handle, path));
	FileIdentifierLongLognToPathMap.insert(std::make_pair(path, fileId));
	
	return handle;
}

unsigned long long GetFileIdentifierByPathLongLong(char *path)
{
	DWORD fileAttr;
	BY_HANDLE_FILE_INFORMATION lpFileInformation;
	HANDLE hFile;
	DWORD dwFlagsAndAttributes;

	fileAttr = GetFileAttributes(path);

	if (path == NULL || fileAttr == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	dwFlagsAndAttributes = 0;
	if (fileAttr & FILE_ATTRIBUTE_DIRECTORY) {
		dwFlagsAndAttributes = FILE_ATTRIBUTE_DIRECTORY | FILE_FLAG_BACKUP_SEMANTICS;
	}
	else if (fileAttr & FILE_ATTRIBUTE_ARCHIVE) {
		dwFlagsAndAttributes = FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_OVERLAPPED;
	}
	else if (fileAttr & FILE_ATTRIBUTE_NORMAL) {
		dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED;
	}
	if (fileAttr & FILE_ATTRIBUTE_REPARSE_POINT) {
		dwFlagsAndAttributes = FILE_ATTRIBUTE_REPARSE_POINT | FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS;
	}

	hFile = CreateFile(path, FILE_READ_EA, FILE_SHARE_READ, NULL, OPEN_EXISTING, dwFlagsAndAttributes, NULL);

	if (hFile == INVALID_HANDLE_VALUE) {
		return false;
	}

	GetFileInformationByHandle(hFile, &lpFileInformation);
	CloseHandle(hFile);

	LARGE_INTEGER fileId;
	fileId.HighPart = lpFileInformation.nFileIndexHigh;
	fileId.LowPart = lpFileInformation.nFileIndexLow;

	return fileId.QuadPart;
}

FILE_ITEM GetFileItemFromPath(char *path)
{
	FILE_ITEM item;

	item.path = new char[strlen(path) + 1];
	strcpy_s(item.path, (strlen(path) + 1), path);  //path
	item.nPathLen = strlen(item.path);  //path length
	item.handle = GetFileIdentifierByPath(path);
	item.bCached = false;  //not in the cache

	return item;
}




char* GetRealPathPath(char *path) {



	bool ret = false;
	TCHAR lpBuffer1[MAX_PATH];
	LPTSTR lpFname1 = NULL;

	// TODO: GetFullPathNameW for  32,767 wide characters instead of just 255
	GetFullPathName(path, MAX_PATH, lpBuffer1, &lpFname1);
		
		
		std::string str(lpBuffer1);
		char* chr = _strdup(str.c_str());
		return chr;
}

/*
char* GetDriveLetterFromVolumeID(char *path) {


	TCHAR volumeName[MAX_PATH + 1] = { 0 };
	TCHAR fileSystemName[MAX_PATH + 1] = { 0 };
	DWORD serialNumber = 0;
	DWORD maxComponentLen = 0;
	DWORD fileSystemFlags = 0;
	if (GetVolumeInformation(
		_T("C:\\"),
		volumeName,
		ARRAYSIZE(volumeName),
		&serialNumber,
		&maxComponentLen,
		&fileSystemFlags,
		fileSystemName,
		ARRAYSIZE(fileSystemName)))
	{
		printf(_T("Volume Name: %s\n"), volumeName);
		printf(_T("Serial Number: %lu\n"), serialNumber);
		printf(_T("File System Name: %s\n"), fileSystemName);
		printf(_T("Max Component Length: %lu\n"), maxComponentLen);
	}
}

*/