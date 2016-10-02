#ifndef _FILETREE_H_
#define _FILETREE_H_
#include <string>
#include "FileTable.h"

class CFileTree
{
	public:
		bool static const debug = false;
		FILE_ITEM AddItem(const char *absolutePath, unsigned char* handle);
		void RemoveItem(const char *absolutePath);
		void RenameItem(char *absolutePathFrom, char *absolutePathTo);

		tree_node_<FILE_ITEM>* FindFileItemForPath(const char *absolutePath);

		void GetNodeFullPath(tree_node_<FILE_ITEM>* node, std::string& path);
		
	protected:
		tree_node_<FILE_ITEM>* findNodeFromRootWithPath(const char *path);
		tree_node_<FILE_ITEM>* findNodeWithPathFromNode(std::string path, tree_node_<FILE_ITEM>* node);
		tree_node_<FILE_ITEM>* findParentNodeFromRootForPath(const char *path);
};
extern void DisplayTree(tree_node_<FILE_ITEM>* node, int level);

#endif
