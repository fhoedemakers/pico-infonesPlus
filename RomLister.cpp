#include <stdio.h>
#include <pico/stdlib.h>
#include <string.h>
#include "RomLister.h"
#include "FrensHelpers.h"

#include "ff.h"

// class to listing directories and .NES files on sd card
namespace Frens
{
	// Buffer must have sufficient bytes to contain directory contents
	RomLister::RomLister(void *buffer, size_t buffersize)
	{
		entries = (RomEntry *)buffer;
		max_entries = buffersize / sizeof(RomEntry);
	}

	RomLister::~RomLister()
	{
	}

	RomLister::RomEntry *RomLister::GetEntries()
	{
		return entries;
	}

	char *RomLister::FolderName()
	{
		return directoryname;
	}
	size_t RomLister::Count()
	{
		return numberOfEntries;
	}

	void RomLister::list(const char *directoryName)
	{
		FRESULT fr;
		numberOfEntries = 0;
		strcpy(directoryname, directoryName);
		FILINFO file;
		if (directoryname == "")
		{
			return;
		}
		DIR dir;
		printf("chdir(%s)\n", directoryName);
		// for f_getcwd to work, set
    	//   #define FF_FS_RPATH		2
        // in ffconf.c
		fr = f_chdir(directoryName);
		if (fr != FR_OK)
		{
			printf("Error changing dir: %d\n", fr);
			return;
		}
		printf("Listing %s\n", ".");

		f_opendir(&dir, ".");
		while (f_readdir(&dir, &file) == FR_OK && file.fname[0] && numberOfEntries < max_entries)
		{
			if (strlen(file.fname) < ROMLISTER_MAXPATH)
			{
				struct RomEntry romInfo;
				strcpy(romInfo.Path, file.fname);
				romInfo.IsDirectory = file.fattrib & AM_DIR;
				if (!romInfo.IsDirectory && Frens::cstr_endswith(romInfo.Path, ".nes"))
				{
					entries[numberOfEntries++] = romInfo;
				}
				else
				{
					if (romInfo.IsDirectory && strcmp(romInfo.Path, "System Volume Information") != 0
					                        && strcmp(romInfo.Path, "SAVES") != 0
											&& strcmp(romInfo.Path, "EDFC") != 0)
					{
						entries[numberOfEntries++] = romInfo;
					}
				}
			}
		}
		f_closedir(&dir);
		
		int result;

		// (bubble) Sort 
		if (numberOfEntries > 1)
		{
			for (int pass = 0; pass < numberOfEntries - 1; ++pass)
			{
				for (int j = 0; j < numberOfEntries - 1 - pass; ++j)
				{
					result = strcmp(entries[j].Path, entries[j + 1].Path);
					if (result > 0)
					{
						RomEntry temp;
						temp = entries[j];
						entries[j] = entries[j + 1];
						entries[j + 1] = temp;
					}
				}
			}
		}
		printf("Sort done\n");
	}
}
