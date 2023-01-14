#include <assert.h>
#include <chadstr.h>
#include <ctype.h>
#include <search.h>
#include <stdint.h>

#define MAX_ROWS 4

typedef uint_least8_t Flag[MAX_ROWS];

const Flag GLORY_TO_UKRAINE = {33, 33, 226, 226};
const Flag UPA = {88, 88, 0, 0};

enum DrawAt {bg, fg};

// I'm too biased to do it without chadstr
str color256(int color, enum DrawAt bg_fg);

void draw_info(const char *flag_name);

int main(int argc, char *argv[])
{
	if (argc > 1)
	{
		hcreate(32);
		draw_info(argv[1]);
		hdestroy();
		return EXIT_SUCCESS;
	}
	printf("usage: %s <flag_name> (case-insensitive)\n", argv[0]);
	return EXIT_FAILURE;
}

str color256(int color, enum DrawAt bg_fg)
{
	assert(color >= 0 && color <= 255 && bg_fg >= bg && bg_fg <= fg);
	//char *ascii_code = malloc(sizeof(*ascii_code)*16);
	// for background
	if (!bg_fg)
	{
	// Maybe, some sunny day… I'll rewrite it.
	/*	snprintf(ascii_code, sizeof(*ascii_code)*16, "%s%d%s",
			"\033[48;5;", color, "m");
	*/	chadstr ascii_code = str("\033[48;5;", color, "m");
		return ascii_code;
	}
	// for foreground
	if (bg_fg)
	{
	/*	snprintf(ascii_code, sizeof(*ascii_code)*16, "%s%d%s",
			"\033[38;5;", color, "m");
	*/	chadstr ascii_code = str("\033[38;5;", color, "m");
		return ascii_code;
	}
	return NULL;
}

void draw_info(const char *flag_name)
{
	assert(flag_name != NULL);

	const size_t len = strlen(flag_name);

	char flag_name_all_caps[len];
	for (size_t i = 0; i < len; ++i)
	{
		// TODO: Make it pointer to a string
		flag_name_all_caps[i] = toupper(flag_name[i]);
	}
	flag_name_all_caps[len] = '\0';

	ENTRY entry, *pEntry = &entry;

	pEntry->key = malloc(len + 1);
	strcpy(pEntry->key, flag_name_all_caps);

	if (hsearch(*pEntry, FIND) == NULL)
	{
		fprintf(stderr, "'%s' was not found.\n", flag_name);
		free(pEntry->key);
		exit(EXIT_FAILURE);
	}

	Flag colors;
	memcpy(colors, hsearch(*pEntry, FIND)->data, sizeof(colors));
	free(pEntry->key);

	uint_least8_t width = 20;
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
*/}

