#include <assert.h>
#include <chadstr.h>
#include <ctype.h>
#include <search.h>
#include <stdint.h>

#define MAX_ROWS 4

typedef struct
{
	const char *name;
	const uint_least8_t rows[MAX_ROWS];
} Flag;

enum DrawAt {bg, fg};

void _fill_htab(void);
str color256(enum DrawAt bg_fg, uint_least8_t color);
void draw_info(const char *flag_name, const char *argv[]);

int main(int argc, char *argv[])
{
	chadstr help_msg = str("usage: ", argv[0],
		" flag_name (case-insensitive)");

	if (argc < 2)
	{
		printf("%s\n", str(*help_msg));
		return EXIT_FAILURE;
	}

	if (strcmp(argv[1], "-h") || strcmp(argv[1], "--help"))
	{
		printf("%s\n", str(*help_msg));
		return EXIT_SUCCESS;
	}

	if (hcreate(32) == 0)
	{
		perror(argv[0]);
		return EXIT_FAILURE;
	}
	_fill_htab();
	draw_info(argv[1], (const char**)argv);
	hdestroy();
	return EXIT_SUCCESS;
}

void _fill_htab(void)
{
	Flag flag1 = { "GLORY_TO_UKRAINE", {33, 33, 226, 226} };
	Flag flag2 = { "UPA", {88, 88, 0, 0} };

	ENTRY entry = { strdup(flag1.name), (void*)&flag1.rows };
	hsearch(entry, ENTER);
	entry.key = strdup(flag2.name);
	entry.data = (void*)&flag2.rows;
	hsearch(entry, ENTER);
}

str color256(enum DrawAt bg_fg, uint_least8_t color)
{
	assert(bg_fg >= bg && bg_fg <= fg);
	return str("\033[", 4 - bg_fg, "8;5;", color, "m");
}

// My lil (non)bloat func
void draw_info(const char *flag_name, const char *argv[])
{
	assert(flag_name != NULL);

	const size_t len = strlen(flag_name);
	char flag_name_all_caps[len];

	for (size_t i = 0; i < len; ++i)
	{
		flag_name_all_caps[i] = toupper((unsigned char)flag_name[i]);
	}
	flag_name_all_caps[len] = '\0';

	ENTRY entry, *pEntry;
	entry.key = strdup(flag_name_all_caps);

	pEntry = hsearch(entry, FIND);
	if (pEntry == NULL)
	{
		fprintf(stderr, "'%s' was not found.\n", flag_name);
		free(entry.key);
		exit(EXIT_FAILURE);
	}

	// Compute width (in characters) according to the number of rows,
	// basically even-or-odd.
	uint_least8_t width = ((sizeof(*pEntry->data) / sizeof(pEntry->data[0])) % 1 == 0) ? 15 : 20;
	uint_least8_t current_row = 0;

	// TODO: Encapsulate it /***********************/
	FILE *fp = popen("/usr/bin/env uname -srm", "r");
	if (fp == NULL)
	{
		perror(argv[0]);
		pclose(fp);
		exit(EXIT_FAILURE);
	}
	char uname_info[1024];
	fgets(uname_info, sizeof(uname_info), fp);
	pclose(fp);
	if (*uname_info == '\0') exit(EXIT_FAILURE);
	/***********************************************/

	// Dont decay into lead pls 😭😭
	// reply: ok
	char reset[] = "\033[0m";
}
