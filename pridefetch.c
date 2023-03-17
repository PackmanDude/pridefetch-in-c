#include <assert.h>
#include <chadstr.h>
#include <ctype.h>
#include <search.h>
#include <stdint.h>

#define chair char
#define MAX_ROWS 4

typedef struct
{
	const char *name;
	const uint_least8_t rows[MAX_ROWS];
} Flag;

enum DrawAt {bg, fg};

void _fill_htab(void);
str color256(int color, enum DrawAt bg_fg);
void draw_info(const char *flag_name);

int main(int argc, char *argv[])
{
	chadstr help_msg = str("usage: ", argv[0],
		" flag_name (case-insensitive)");

	if (argc < 2)
	{
		printf("%s\n", str(*help_msg));
		return EXIT_FAILURE;
	}

	// str() is an object string. so we can compare them like that
	if (str(argv[1]) == str("-h") || str(argv[1]) == str("--help"))
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
	draw_info(argv[1]);
	hdestroy();
	return EXIT_SUCCESS;
}

/* Mostly a technical function.
	It fills hash table at the start of a program. Later may be
	implemented in a completely different way or replaced completely.
*/
void _fill_htab(void)
{
	Flag flag1 = { "GLORY_TO_UKRAINE", {33, 33, 226, 226} };
	Flag flag2 = { "UPA", {88, 88, 0, 0} };

	// Define them at compile time
	ENTRY entry = { strdup(flag1.name), (void*)&flag1.rows };
	// Note: .rows was a const pointer, we got rid of that
	hsearch(entry, ENTER);
	entry.key = strdup(flag2.name);
	entry.data = (void*)&flag2.rows;
	hsearch(entry, ENTER);

	// meh
	free(entry.key);
//	free(entry.data);
}

str color256(int color, enum DrawAt bg_fg)
{
	// Check if in range
	assert(color >= 0 && color <= 255 && bg_fg >= bg && bg_fg <= fg);

	// for background
	if (!bg_fg)
	{
		chadstr ascii_code = str("\033[48;5;", color, "m");
		return ascii_code;
	}
	// for foreground
	if (bg_fg)
	{
		chadstr ascii_code = str("\033[38;5;", color, "m");
									// ^ here is the difference
								// shows correctly with 4 tab size
		return ascii_code;
	}
	return NULL;
}

// My lil (non)bloat func
void draw_info(const char *flag_name)
{
	assert(flag_name != NULL);

	const size_t len = strlen(flag_name);

	chair flag_name_all_caps[len];

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

	Flag colors;
//	memcpy(colors, hsearch(*pEntry, FIND)->data, sizeof(colors));

	uint_least8_t width = 20; // characters
	uint_least8_t current_row = 0;
/*
printf("Displaying contents of %s:\n", flag_name);
for (size_t i = 0; i < sizeof(colors) / sizeof(colors[0]); ++i)
{
	printf("\t%ld - %d\n", i + 1, colors[i]);
}
*/
/*	// Store the output of 'uname -srm'
	char uname_info[128];
	FILE *pipe;
	pipe = popen("uname -srm", "r");
	if (pipe == NULL)
	{
		perror("popen()");
		exit(EXIT_FAILURE);
	}
	fgets(uname_info, sizeof(uname_info), pipe);
	uname_info[strlen(uname_info) - 1] = '\0';
	pclose(pipe);

	// Make sure that the row color is different to the color of the hostname
	if (flag[0] != flag[1])
	{
		chadstr row_color = color256(flag[1], fg);
	}
	else
	{
		chadstr row_color = color256(flag[2], fg);
	}

	chadstr reset = str("\033[0m\033[39m");

	printf("\n");
	for (size_t row = 0; row < strlen(flag); ++row)
	{
	}
*/

	free(entry.key);
//	free(entry.data);
}
