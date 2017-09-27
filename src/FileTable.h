#ifndef _FILETABLE_H_
#define _FILETABLE_H_

#define TABLE_SIZE 65535

#include "tree.hh"
#include <unordered_map>
#include <string>

typedef struct
{
    char *path;
    unsigned int nPathLen;
    unsigned char *handle;
    bool bCached;
} FILE_ITEM;

typedef struct _FILE_TABLE
{
	 tree_node_<FILE_ITEM>* pItems[TABLE_SIZE];
    _FILE_TABLE *pNext;
} FILE_TABLE;

typedef struct _CACHE_LIST
{
    FILE_ITEM *pItem;
    _CACHE_LIST *pNext;
} CACHE_LIST;

class CFileTable
{
    public:
    CFileTable();
    ~CFileTable();
    unsigned long long GetIDByPath(char *path);
    unsigned char *GetHandleByPath(char *path);
    char *GetPathByHandle(unsigned char *handle);
	FILE_ITEM FindItemByPath(char *path);
    bool RemoveItem(char *path);
	void RenameFile(char *pathFrom, char *pathTo);

    protected:
		tree_node_<FILE_ITEM>* AddItem(char *path);

    private:
    FILE_TABLE *m_pFirstTable, *m_pLastTable;
    unsigned int m_nTableSize;
    CACHE_LIST *m_pCacheList;

	FILE_ITEM GetItemByID(unsigned char *nID);
    void PutItemInCache(FILE_ITEM *pItem);
	// std::unordered_map<long long, tree_node_<FILE_ITEM>*> FileItemStorage;
	// std::unordered_map<std::string, FILE_ITEM> FilePathItemStorage;
	

};

extern bool FileExists(char *path);
extern unsigned long long GetFileID(char *path);
extern unsigned char *GetFileHandle(char *path);
extern char *GetFilePath(unsigned char *handle);
extern int RenameFile(char *pathFrom, char *pathTo);
extern int RenameDirectory(char *pathFrom, char *pathTo);
extern int RemoveFolder(char *path);
extern char* GetRealPathPath(char *path);
extern bool RemoveFile(char *path);


extern char* GetPathByFileIdentifier(unsigned char *handle);
extern unsigned char* GetFileIdentifierByPath(char *path);
extern FILE_ITEM GetFileItemFromPath(char *path);
void stream2hex(const std::string str, std::string& hexstr, bool capital = false);
void hex2stream(const std::string hexstr, std::string& str);
extern unsigned long long GetFileIdentifierByPathLongLong(char *path);

#endif
