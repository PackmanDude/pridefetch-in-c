#define _XOPEN_SOURCE 1
#include <assert.h>
#ifdef _GNU_SOURCE
# include <getopt.h>
#endif
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <time.h>
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

#define COLOR_BUFFER_SIZE sizeof "\033[?8;5;???m"
#define RESET "\033[0m"
#define numof(array) (sizeof array / sizeof *array)
#define LRAND48_MAX ((1L << 31) - 1)

typedef struct
{
	const char *name;
	const unsigned char *rows;
	const size_t row_count;
} Flag;

static Flag flags[] =
{
	{ "RAINBOW", (const unsigned char[]){196, 208, 226, 28, 20, 90}, 6},
	{ "MLM", (const unsigned char[]){23, 43, 115, 255, 117, 57, 55}, 7},
	{ "LESBIAN", (const unsigned char[]){202, 209, 255, 255, 168, 161}, 6},
	{ "BISEXUAL", (const unsigned char[]){198, 198, 97, 25, 25}, 5},
	{ "PANSEXUAL", (const unsigned char[]){198, 198, 220, 220, 39, 39}, 6},
	{ "TRANS", (const unsigned char[]){81, 211, 255, 211, 81}, 5},
	{ "NONBINARY", (const unsigned char[]){226, 226, 255, 255, 98, 98, 237, 237}, 8},
	{ "DEMIBOY", (const unsigned char[]){244, 249, 117, 255, 117, 249, 244}, 7},
	{ "DEMIGIRL", (const unsigned char[]){244, 249, 218, 255, 218, 249, 244}, 7},
	{ "GENDERFLUID", (const unsigned char[]){211, 255, 128, 0, 63}, 5},
	{ "AROMANTIC", (const unsigned char[]){71, 149, 255, 249, 0}, 5},
	{ "AGENDER", (const unsigned char[]){0, 251, 255, 149, 255, 251, 0}, 7},
	{ "ASEXUAL", (const unsigned char[]){0, 0, 242, 242, 255, 255, 54, 54}, 8},
	{ "GRAYSEXUAL", (const unsigned char[]){54, 242, 255, 242, 54}, 5},
	{ "UKRAINIAN", (const unsigned char[]){33, 33, 33, 11, 11, 11}, 6},
	{ "UPA", (const unsigned char[]){88, 88, 88, 0, 0, 0}, 6}
};

enum DrawAt {bg, fg};

void
color256(char *str, unsigned char color, enum DrawAt bg_fg)
{
	assert(bg_fg >= bg && bg_fg <= fg);
	if (unlikely(snprintf(str, COLOR_BUFFER_SIZE, "\033[%d8;5;%um",
		4 - bg_fg, color) < 0)) exit(EXIT_FAILURE);
}

// borrowed from procps
void
format_uptime(char *str, size_t str_len, long uptime_secs)
{
	int updecades = uptime_secs / (60L * 60 * 24 * 365 * 10);
	int upyears = uptime_secs / (60L * 60 * 24 * 365) % 10;
	int upweeks = uptime_secs / (60L * 60 * 24 * 7) % 52;
	int updays = uptime_secs / (60L * 60 * 24) % 7;
	int uphours = uptime_secs / (60 * 60) % 24;
	int upminutes = uptime_secs / 60 % 60;
	size_t pos = 0;
	int temp;

	if (updecades)
	{
		temp = snprintf(str, str_len, "%d %s",
			updecades, updecades != 1 ? "decades" : "decade");
		if (unlikely(temp < 0)) exit(EXIT_FAILURE);
		if ((unsigned)temp >= str_len) return;
		else pos += temp;
	}
	if (upyears)
	{
		temp = snprintf(str + pos, str_len - pos,
			"%s%d %s", pos ? ", " : "",
			upyears, upyears != 1 ? "years" : "year");
		if (unlikely(temp < 0)) exit(EXIT_FAILURE);
		if ((unsigned)temp >= str_len - pos) return;
		else pos += temp;
	}
	if (upweeks)
	{
		temp = snprintf(str + pos, str_len - pos,
			"%s%d %s", pos ? ", " : "",
			upweeks, upweeks != 1 ? "weeks" : "week");
		if (unlikely(temp < 0)) exit(EXIT_FAILURE);
		if ((unsigned)temp >= str_len - pos) return;
		else pos += temp;
	}
	if (updays)
	{
		temp = snprintf(str + pos, str_len - pos,
			"%s%d %s", pos ? ", " : "",
			updays, updays != 1 ? "days" : "day");
		if (unlikely(temp < 0)) exit(EXIT_FAILURE);
		if ((unsigned)temp >= str_len - pos) return;
		else pos += temp;
	}
	if (uphours)
	{
		temp = snprintf(str + pos, str_len - pos,
			"%s%d %s", pos ? ", " : "",
			uphours, uphours != 1 ? "hours" : "hour");
		if (unlikely(temp < 0)) exit(EXIT_FAILURE);
		if ((unsigned)temp >= str_len - pos) return;
		else pos += temp;
	}
	if (unlikely(snprintf(str + pos, str_len - pos,
		"%s%d %s", pos ? ", " : "",
		upminutes, upminutes != 1 ? "minutes" : "minute") < 0))
		exit(EXIT_FAILURE);
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
	const char *username = getlogin();
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
	FILE *pipe = popen(". /etc/os-release && [ \"$NAME\" ] && printf %s \"$NAME\"", "r");
	if (unlikely(!pipe))
	{
		perror("popen");
		exit(EXIT_FAILURE);
	}
	char distro_name[64] = "N/A";
	if (unlikely(!fgets(distro_name, sizeof distro_name, pipe) && ferror(pipe)))
	{
		perror("fgets");
		exit(EXIT_FAILURE);
	}
	if (unlikely(pclose(pipe) == -1))
	{
		perror("pclose");
		exit(EXIT_FAILURE);
	}

	/// 2. Drawing
	/// 2.1 Setting variables
	char primary[COLOR_BUFFER_SIZE];
	color256(primary, flag->rows[0], fg);
	char secondary[COLOR_BUFFER_SIZE];
	size_t secondary_row = 0;
	while (++secondary_row < flag->row_count - 1 && flag->rows[secondary_row] == flag->rows[0]);
	color256(secondary, flag->rows[secondary_row], fg);
	// ensure 3:2 aspect ratio for terminal with 2x5 character size
	const size_t width = flag->row_count * 3.75 + .5;

	/// 2.2 Output
	if (unlikely(putchar('\n') == EOF))
	{
		perror("putchar");
		exit(EXIT_FAILURE);
	}
	char row_info[256 + COLOR_BUFFER_SIZE + sizeof RESET];
	for (size_t current_row = 0; current_row < flag->row_count; ++current_row)
	{
		switch (current_row)
		{
			case 0:
			{
				if (unlikely(snprintf(row_info, sizeof row_info,
					"%s\033[1m%s@%s%s", primary, username, osname.nodename,
					RESET) < 0)) exit(EXIT_FAILURE);
				break;
			}
			case 1:
			{
				if (unlikely(snprintf(row_info, sizeof row_info, "%sos%10s%s",
					secondary, RESET, distro_name ) < 0)) exit(EXIT_FAILURE);
				break;
			}
			case 2:
			{
				if (unlikely(snprintf(row_info, sizeof row_info, "%sarch%8s%s",
					secondary, RESET, osname.machine) < 0)) exit(EXIT_FAILURE);
				break;
			}
			case 3:
			{
				char uptime[256];
				format_uptime(uptime, sizeof uptime, sys_info.uptime);
				if (unlikely(snprintf(row_info, sizeof row_info, "%suptime%6s%s",
					secondary, RESET, uptime) < 0)) exit(EXIT_FAILURE);
				break;
			}
			case 4:
			{
				if (unlikely(snprintf(row_info, sizeof row_info, "%skern%8s%s",
					secondary, RESET, osname.release) < 0)) exit(EXIT_FAILURE);
				break;
			}
			default:
			{
				*row_info = '\0';
			}
		}
		char color[COLOR_BUFFER_SIZE];
		color256(color, flag->rows[current_row], bg);
		char row[width + 1];
		memset(row, ' ', width);
		row[width] = '\0';
		if (unlikely(printf(" %s%s\033[49m %s\n", color, row, row_info) < 0))
			exit(EXIT_FAILURE);
	}
	if (unlikely(putchar('\n') == EOF))
	{
		perror("putchar");
		exit(EXIT_FAILURE);
	}
}

int
main(int argc, char *argv[])
{
	int option;
#ifdef _GNU_SOURCE
	const struct option longopts[] =
	{
		{ "flag", required_argument, NULL, 'f' },
		{ "choose", required_argument, NULL, 'c' },
		{ "list", no_argument, NULL, 'l' },
		{ "help", no_argument, NULL, 'h' },
		{ 0 }
	};
	while ((option = getopt_long(argc, argv, "f:c:lh", longopts, NULL)) != -1)
#else
	while ((option = getopt(argc, argv, "f:c:lh")) != -1)
#endif
	{
		switch (option)
		{
			case 'f':
			{
				bool found = false;
				for (size_t flag = 0; flag < numof(flags); ++flag)
				{
					if (!strcmp(optarg, flags[flag].name))
					{
						draw_info(&flags[flag]);
						found = true;
						break;
					}
				}
				if (!found)
				{
					if (unlikely(fputs("No flag with that name was found.\n",
						stderr) == EOF)) perror("fputs");
					exit(EXIT_FAILURE);
				}
				break;
			}
			case 'c': // TODO: fix it.
			{
				Flag *list[numof(flags) * numof(flags)];
				size_t i = 0;
				for (char *token = strtok(optarg, ", "); token != NULL; token = strtok(NULL, ", "))
				{
					for (size_t flag = 0; flag < numof(flags); ++flag)
					{
						if (!strcmp(token, flags[flag].name))
						{
							list[i] = &flags[flag];
							break;
						}
					}
					if (++i == numof(list)) break;
				}
				srand48(time(NULL));
				long flag = lrand48();
				while (flag >= LRAND48_MAX - (long)(LRAND48_MAX % i))
					flag = lrand48();
				draw_info(list[flag % i]);
				break;
			}
			case 'l':
			{
				if (unlikely(puts("Available flags:") == EOF))
				{
					perror("puts");
					exit(EXIT_FAILURE);
				}
				for (size_t flag = 0; flag < numof(flags) - 1; ++flag)
					if (unlikely(printf("%s, ", flags[flag].name) < 0))
						exit(EXIT_FAILURE);
				if (unlikely(printf("%s.\n", flags[numof(flags) - 1].name) < 0))
					exit(EXIT_FAILURE);
				break;
			}
			case 'h':
			{
				if (unlikely(puts("Options:\n"
					"  -f, --flag FLAG\n"
					"    Display the specified flag.\n"
					"  -c, --choose FLAG1, FLAG2, FLAGN\n"
					"    Choose a flag randomly from the specified list.\n"
					"  -l, --list\n"
					"    List all flags.") == EOF))
				{
					perror("puts");
					exit(EXIT_FAILURE);
				}
				break;
			}
			default:
			{
				exit(EXIT_FAILURE);
			}
		}
	}
}
