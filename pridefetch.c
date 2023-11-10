#define __USE_POSIX2
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>

#ifdef __has_builtin
# if __has_builtin(__builtin_expect)
#  define likely(x) __builtin_expect(!!(x), 1)
#  define unlikely(x) __builtin_expect((x), 0)
# endif
#endif
#ifndef likely
# define likely(x) (x)
#endif
#ifndef unlikely
# define unlikely(x) (x)
#endif

#define MAX_ROWS 4
#define WIDTH 20
#define COLOR_BUFFER_SIZE sizeof "\033[?8;5;???m"

typedef struct
{
	const char *name;
//	int row_thickness;
	unsigned char rows[MAX_ROWS];
} Flag;

static Flag flags[] =
{
	{ "UKRAINIAN", {33, 33, 11, 11} },
	{ "UPA", {88, 88, 0, 0} }
};

enum DrawAt {bg, fg};

void
color256(char *str, unsigned char color, enum DrawAt bg_fg)
{
	assert(bg_fg >= bg && bg_fg <= fg);
	if (unlikely(snprintf(str, COLOR_BUFFER_SIZE, "\033[%d8;5;%um",
		4 - bg_fg, color) < 0)) exit(EXIT_FAILURE);
}

void
draw_info(const Flag *flag)
{
	/// 1. Gathering
	struct sysinfo sys_info;
	if (unlikely(sysinfo(&sys_info)))
	{
		perror("sysinfo");
		exit(EXIT_FAILURE);
	}
	char *username = getlogin();
	if (unlikely(!username))
	{
		perror("getlogin");
		exit(EXIT_FAILURE);
	}
	struct utsname osname;
	if (unlikely(uname(&osname) == -1))
	{
		perror("uname");
		exit(EXIT_FAILURE);
	}
	FILE *pipe = popen(". /etc/os-release; [ \"$NAME\" ] && printf %s \"$NAME\"", "r");
	if (unlikely(!pipe))
	{
		perror("popen");
		exit(EXIT_FAILURE);
	}
	char distro_name[64];
	if (!fgets(distro_name, sizeof distro_name, pipe))
	{
		if (ferror(pipe)) perror("pipe: fgets");
		exit(EXIT_FAILURE);
	}
	pclose(pipe);

	/// 2. Drawing
	/// 2.1 Setting variables
	char primary[COLOR_BUFFER_SIZE];
	color256(primary, flag->rows[0], fg);
	char secondary[COLOR_BUFFER_SIZE];
	color256(secondary, flag->rows[0] != flag->rows[1]
			? flag->rows[1]
			: flag->rows[2],
		fg);
	const char *reset = "\033[0m";

	/// 2.2 Output
	if (unlikely(putchar('\n') == EOF && ferror(stdout)))
	{
		perror("putchar");
		exit(EXIT_FAILURE);
	}
	char row_info[256 + sizeof "\033[?8;5;???m" + sizeof reset];
	for (size_t current_row = 0; current_row < sizeof flag->rows / sizeof *flag->rows; ++current_row)
	{
		switch (current_row)
		{
			case 0:
				// TODO: Read $PS1 instead.
				if (unlikely(snprintf(row_info, sizeof row_info,
					"%s\033[1m%s@%s%s", primary, username, osname.nodename,
					reset) < 0)) exit(EXIT_FAILURE);
				break;
			case 1:
				if (unlikely(snprintf(row_info, sizeof row_info, "%s%s%s",
					secondary, distro_name, reset) < 0)) exit(EXIT_FAILURE);
				break;
			case 2:
				if (unlikely(snprintf(row_info, sizeof row_info, "%s%s%s",
					secondary, osname.machine, reset) < 0)) exit(EXIT_FAILURE);
				break;
			case 3:
				if (unlikely(snprintf(row_info, sizeof row_info, "%s%s%s",
					secondary, osname.release, reset) < 0)) exit(EXIT_FAILURE);
				break;
			case 4:
				if (unlikely(snprintf(row_info, sizeof row_info, "%s%ld%s",
					secondary, sys_info.uptime, reset) < 0)) exit(EXIT_FAILURE);
				break;
			default:
				*row_info = '\0';
		}
		char color[COLOR_BUFFER_SIZE];
		color256(color, flag->rows[current_row], bg);
		char row[WIDTH + 1];
		memset(row, ' ', WIDTH);
		row[WIDTH] = '\0';
		if (unlikely(printf(" %s%s\033[49m %s\n", color, row, row_info) < 0))
			exit(EXIT_FAILURE);
	}
	if (unlikely(putchar('\n') == EOF && ferror(stdout)))
	{
		perror("putchar");
		exit(EXIT_FAILURE);
	}
}

int
main(int argc, char *argv[])
{
	draw_info(&flags[0]);
}
