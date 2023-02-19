#include <assert.h>
#include <chadstr.h>
#include <ctype.h>
#include <search.h>
#include <stdint.h>

#define MAX_ROWS 4
#define FLAGS_COUNT 2

typedef uint_least8_t Flag[MAX_ROWS];

enum DrawAt {bg, fg};

void _fill_htab(void);
str color256(int color, enum DrawAt bg_fg);
void draw_info(const char *flag_name);

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("usage: %s <flag_name> (case-insensitive)\n", argv[0]);
		return EXIT_FAILURE;
	}

	hcreate(32);
	_fill_htab();
	draw_info(argv[1]);
	hdestroy();
	return EXIT_SUCCESS;
}

/* Mostly a technical function.
	It fills hash table at start of the program. Later may be
	implemented in completely different way or replaced completely.
*/
void _fill_htab(void)
{
	const char *flags_names[] =
	{
		"GLORY_TO_UKRAINE",
		"UPA"
	};
	const uint_least8_t flags_colors[FLAGS_COUNT][MAX_ROWS] =
	{
		{33, 33, 226, 226},
		{88, 88, 0, 0}
	};

	ENTRY entry, *pEntry = &entry;

	for (int i = 0; i < FLAGS_COUNT; ++i)
	{
		pEntry->key = strdup(flags_names[i]);
		pEntry->data = malloc(sizeof(*flags_colors[0]) * MAX_ROWS);
		memcpy(pEntry->data, flags_colors[i],
			sizeof(*flags_colors[0]) * MAX_ROWS);

		if (hsearch(*pEntry, ENTER) == NULL)
		{
			fprintf(stderr, "entry failed.\n");
//			free(pEntry->key);
			free(pEntry->data);
			exit(EXIT_FAILURE);
		}
	}
//	free(pEntry->key);
	free(pEntry->data);
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

// (not)My lil bloat func
void draw_info(const char *flag_name)
{
	assert(flag_name != NULL);

	const size_t len = strlen(flag_name);

	char flag_name_all_caps[len];
	char *pFlag_name_all_caps = flag_name_all_caps;
	for (size_t i = 0; i < len; ++i)
	{
		pFlag_name_all_caps[i] = toupper((unsigned char)flag_name[i]);
	}
	pFlag_name_all_caps[len] = '\0';

	ENTRY entry, *pEntry = &entry;
	pEntry->key = strdup(pFlag_name_all_caps);

	if (hsearch(*pEntry, FIND) == NULL)
	{
		fprintf(stderr, "'%s' was not found.\n", flag_name);
		free(pEntry->key);
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

	free(pEntry->key);
//	free(pEntry->data);
}
