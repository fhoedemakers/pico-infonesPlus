#ifndef ROMLISTER
#define ROMLISTER
#include <string>
#include <vector>
#include "ff.h"
#define ROMLISTER_MAXPATH 80
namespace Frens {

	class RomLister
	{

	public:
		struct RomEntry {
			char Path[ROMLISTER_MAXPATH];  // Without dirname
			bool IsDirectory;
		};
		RomLister( void *buffer, size_t buffersize, const char *allowedExtensions);
		~RomLister();
		RomEntry* GetEntries();
		char  *FolderName();
		size_t Count();
		void list(const char *directoryName);

	private:
		bool IsextensionAllowed(char *filename);
		char directoryname[FF_MAX_LFN];
		int length;
		size_t max_entries;
		RomEntry *entries;
		size_t numberOfEntries;
		int numberOfExtensions;
		char **extensions;

	};
}
#endif
