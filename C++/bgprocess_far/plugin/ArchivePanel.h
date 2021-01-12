#ifndef ARCHIVEPANEL_H
#define ARCHIVEPANEL_H


//typedef std::pair<WIN32_FIND_DATAW, std::wstring> FILEDESC;

typedef struct
{
	WIN32_FIND_DATAW FindData;
	std::wstring path;
	std::wstring subpath;
}FILEDESC;
typedef std::vector<FILEDESC> FILELIST;

typedef std::stack<FILELIST::iterator> CURRENTFILESTACK;

//typedef tree<FILEDESC> FILETREE;


struct ArchivePanel
{	
	//FILETREE filetree;
	FILELIST files;
	FILELIST current_filemap;

//	FILETREE::iterator i_curlevel;

	std::string ArchiveName;
	std::string CurrentFolder;
	CURRENTFILESTACK CurrentFileStack;

	//const char *Dir;

	ArchivePanel();
	~ArchivePanel();
};


#endif// ARCHIVEPANEL_H