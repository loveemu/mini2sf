/*
** Simple utility to convert a NDS Rom to a 2SF.
** Based on EXE2PSF code, written by Neill Corlett
** Released under the terms of the GNU General Public License
**
** You need zlib to compile this.
** It's available at http://www.gzip.org/zlib/
*/

#define APP_NAME	"rom2sf"
#define APP_VER		"[2015-04-08]"
#define APP_DESC	"Numbered mini2sf generator"
#define APP_AUTHOR	"loveemu <http://github.com/loveemu/rom2sf>"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <string>
#include <sstream>
#include <map>

#include <zlib.h>

#include "mini2sf.h"
#include "nds2sf.h"
#include "cbyteio.h"
#include "cpath.h"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <sys/stat.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define _chdir(s)	chdir((s))
#define _mkdir(s)	mkdir((s), 0777)
#define _rmdir(s)	rmdir((s))
#endif

void printUsage(const char *cmd)
{
	const char *availableOptions[] = {
		"--help", "Show this help",
		"--psfby, --2sfby [name]", "Set creator name of 2SF",
	};

	printf("%s %s\n", APP_NAME, APP_VER);
	printf("======================\n");
	printf("\n");
	printf("%s. Created by %s.\n", APP_DESC, APP_AUTHOR);
	printf("\n");
	printf("Usage\n");
	printf("-----\n");
	printf("\n");
	printf("Syntax: `%s <Base name> <Offset> <Size> <Count>`\n", cmd);
	printf("\n");
	printf("### Options ###\n");
	printf("\n");

	for (int i = 0; i < sizeof(availableOptions) / sizeof(availableOptions[0]); i += 2)
	{
		printf("%s\n", availableOptions[i]);
		printf("  : %s\n", availableOptions[i + 1]);
		printf("\n");
	}
}

int main(int argc, char **argv)
{
	bool result;
	int argnum;
	int argi;

	long lvalue;
	char * strtol_endp;

	char *psfby = NULL;

	argi = 1;
	while (argi < argc && argv[argi][0] == '-')
	{
		if (strcmp(argv[argi], "--help") == 0)
		{
			printUsage(argv[0]);
			return EXIT_SUCCESS;
		}
		else if (strcmp(argv[argi], "--psfby") == 0 || strcmp(argv[argi], "--2sfby") == 0) {
			if (argi + 1 >= argc) {
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}

			psfby = argv[argi + 1];
			argi++;
		}
		else
		{
			fprintf(stderr, "Error: Unknown option \"%s\"\n", argv[argi]);
			return EXIT_FAILURE;
		}
		argi++;
	}

	argnum = argc - argi;
	if (argnum != 4)
	{
		fprintf(stderr, "Error: Too few/more arguments.\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "Run \"%s --help\" for help.\n", argv[0]);
		return EXIT_FAILURE;
	}

	char * nds2sf_basename = argv[argi];

	char libname[PATH_MAX];
	sprintf(libname, "%s.2sflib", nds2sf_basename);

	lvalue = strtol(argv[argi + 1], &strtol_endp, 16);
	if (*strtol_endp != '\0' || errno == ERANGE || lvalue < 0) {
		fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi + 1]);
		return EXIT_FAILURE;
	}
	uint32_t offset = (uint32_t)lvalue;

	lvalue = strtol(argv[argi + 2], &strtol_endp, 10);
	if (*strtol_endp != '\0' || errno == ERANGE || lvalue < 0) {
		fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi + 2]);
		return EXIT_FAILURE;
	}
	size_t size = (size_t)lvalue;

	lvalue = strtol(argv[argi + 3], &strtol_endp, 10);
	if (*strtol_endp != '\0' || errno == ERANGE || lvalue < 0) {
		fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi + 3]);
		return EXIT_FAILURE;
	}
	uint32_t count = (uint32_t)lvalue;

	result = true;
	for (uint32_t num = 0; num < count; num++) {
		std::map<std::string, std::string> tags;
		tags["_lib"] = libname;

		if (psfby != NULL && strcmp(psfby, "") != 0) {
			tags["snsfby"] = psfby;
		}

		char nds2sf_path[PATH_MAX];
		sprintf(nds2sf_path, "%s-%04d.mini2sf", nds2sf_basename, num);

		if (NDS2SF::make_mini2sf(nds2sf_path, offset, size, num, tags)) {
			printf("Created %s\n", nds2sf_path);
		}
		else {
			printf("Error: Unable to create %s\n", nds2sf_path);
			result = false;
		}
	}

	return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
