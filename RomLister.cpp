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
	RomLister::RomLister(void *buffer, size_t buffersize, const char *allowedExtensions)
	{
		entries = (RomEntry *)buffer;
		max_entries = buffersize / sizeof(RomEntry);
		const char *delimiters = ", ";
		extensions = cstr_split(allowedExtensions, delimiters, &numberOfExtensions);
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

	bool RomLister::IsextensionAllowed(char *filename)
	{
		if (numberOfExtensions == 0)
		{
			return true;
		}
		for (int i = 0; i < numberOfExtensions; i++)
		{
			if (Frens::cstr_endswith(filename, extensions[i]))
			{
				return true;
			}
		}
		return false;
	}
	void RomLister::list(const char *directoryName)
	{
		FRESULT fr;
		numberOfEntries = 0;
		strcpy(directoryname, directoryName);
		FILINFO file;
		RomEntry tempEntry;
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
		printf("Listing current directory, reading maximum %d entries.\n", max_entries);

		f_opendir(&dir, ".");
		while (f_readdir(&dir, &file) == FR_OK && file.fname[0])
		{
			if (numberOfEntries < max_entries)
			{
				if (strlen(file.fname) < ROMLISTER_MAXPATH)
				{
					struct RomEntry romInfo;
					strcpy(romInfo.Path, file.fname);
					romInfo.IsDirectory = file.fattrib & AM_DIR;
					// if (!romInfo.IsDirectory && Frens::cstr_endswith(romInfo.Path, ".nes"))
					if (!romInfo.IsDirectory && IsextensionAllowed(romInfo.Path))
					{
						entries[numberOfEntries++] = romInfo;
					}
					else
					{
						if (romInfo.IsDirectory && strcmp(romInfo.Path, "System Volume Information") != 0 && strcmp(romInfo.Path, "SAVES") != 0 && strcmp(romInfo.Path, "EDFC") != 0)
						{
							entries[numberOfEntries++] = romInfo;
						}
					}
				} else {
					printf("Filename too long: %s\n", file.fname);
				}
			}
			else
			{
				if ( numberOfEntries == max_entries)
				{
					printf("Max entries reached.\n");
				}
				printf("Skipping %s\n", file.fname);
			}
		}
		f_closedir(&dir);
		// (bubble) Sort
		if (numberOfEntries > 1)
		{
			for (int pass = 0; pass < numberOfEntries - 1; ++pass)
			{
				for (int j = 0; j < numberOfEntries - 1 - pass; ++j)
				{
					int result = 0;
					// Directories first in the list
					if (entries[j].IsDirectory && entries[j + 1].IsDirectory == false)
					{
						continue;
					}
					bool swap = false;
					if (entries[j].IsDirectory == false && entries[j + 1].IsDirectory)
					{
						swap = true;
					}
					else
					{
						result = strcasecmp(entries[j].Path, entries[j + 1].Path);
					}
					if (swap || result > 0)
					{
						tempEntry = entries[j];
						entries[j] = entries[j + 1];
						entries[j + 1] = tempEntry;
					}
				}
			}
		}
		printf("Sort done\n");
	}
}
