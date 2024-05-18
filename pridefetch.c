#define _XOPEN_SOURCE
#include <assert.h>
#include <errno.h>
#ifdef _GNU_SOURCE
# include <getopt.h>
#endif
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
#define NUMOF(array) (sizeof array / sizeof *array)
#define FLAG(name, ...) { #name, (const unsigned char[]){__VA_ARGS__}, NUMOF(((const unsigned char[]){__VA_ARGS__})) }
#define PERROR_AND_EXIT(s) { perror(s); exit(EXIT_FAILURE); }
#define RESET "\033[m"

typedef struct
{
	const char *name;
	const unsigned char *rows;
	size_t row_count;
} Flag;

static const Flag flags[] =
{
	FLAG(AGENDER, 0, 251, 255, 149, 255, 251, 0),
	FLAG(AROMANTIC, 71, 149, 255, 249, 0),
	FLAG(ASEXUAL, 0, 0, 242, 242, 255, 255, 54, 54),
	FLAG(BISEXUAL, 198, 198, 97, 25, 25),
	FLAG(DEMIBOY, 244, 249, 117, 255, 117, 249, 244),
	FLAG(DEMIGIRL, 244, 249, 218, 255, 218, 249, 244),
	FLAG(GENDERFLUID, 211, 255, 128, 0, 63),
	FLAG(GRAYSEXUAL, 54, 242, 255, 242, 54),
	FLAG(LESBIAN, 202, 209, 255, 255, 168, 161),
	FLAG(MLM, 23, 43, 115, 255, 117, 57, 55),
	FLAG(NONBINARY, 226, 226, 255, 255, 98, 98, 237, 237),
	FLAG(PANSEXUAL, 198, 198, 220, 220, 39, 39),
	FLAG(RAINBOW, 196, 208, 226, 28, 20, 90),
	FLAG(TRANS, 81, 211, 255, 211, 81),
	FLAG(UKRAINIAN, 33, 33, 33, 11, 11, 11),
	FLAG(UPA, 88, 88, 88, 0, 0, 0)
};

enum DrawAt { bg, fg };

static void
color256(char * restrict str, unsigned char color, enum DrawAt bg_fg)
{
	assert(bg_fg >= bg && bg_fg <= fg);
	if (unlikely(snprintf(str, COLOR_BUFFER_SIZE, "\033[%d8;5;%um",
		4 - bg_fg, color) < 0)) PERROR_AND_EXIT("snprintf")
}

// borrowed from procps
static void
format_uptime(char * restrict str, size_t str_len, long uptime_secs)
{
	const int updecades = uptime_secs / (60L * 60 * 24 * 365 * 10);
	const int upyears = uptime_secs / (60L * 60 * 24 * 365) % 10;
	const int upweeks = uptime_secs / (60L * 60 * 24 * 7) % 52;
	const int updays = uptime_secs / (60L * 60 * 24) % 7;
	const int uphours = uptime_secs / (60 * 60) % 24;
	const int upminutes = uptime_secs / 60 % 60;
	size_t pos = 0;
	int temp;

	if (updecades)
	{
		temp = snprintf(str, str_len, "%d %s",
			updecades, updecades != 1 ? "decades" : "decade");
		if (unlikely(temp < 0)) PERROR_AND_EXIT("snprintf")
		if ((unsigned)temp >= str_len) return;
		else pos += temp;
	}
	if (upyears)
	{
		temp = snprintf(str + pos, str_len - pos,
			"%s%d %s", pos ? ", " : "",
			upyears, upyears != 1 ? "years" : "year");
		if (unlikely(temp < 0)) PERROR_AND_EXIT("snprintf")
		if ((unsigned)temp >= str_len - pos) return;
		else pos += temp;
	}
	if (upweeks)
	{
		temp = snprintf(str + pos, str_len - pos,
			"%s%d %s", pos ? ", " : "",
			upweeks, upweeks != 1 ? "weeks" : "week");
		if (unlikely(temp < 0)) PERROR_AND_EXIT("snprintf")
		if ((unsigned)temp >= str_len - pos) return;
		else pos += temp;
	}
	if (updays)
	{
		temp = snprintf(str + pos, str_len - pos,
			"%s%d %s", pos ? ", " : "",
			updays, updays != 1 ? "days" : "day");
		if (unlikely(temp < 0)) PERROR_AND_EXIT("snprintf")
		if ((unsigned)temp >= str_len - pos) return;
		else pos += temp;
	}
	if (uphours)
	{
		temp = snprintf(str + pos, str_len - pos,
			"%s%d %s", pos ? ", " : "",
			uphours, uphours != 1 ? "hours" : "hour");
		if (unlikely(temp < 0)) PERROR_AND_EXIT("snprintf")
		if ((unsigned)temp >= str_len - pos) return;
		else pos += temp;
	}
	if (upminutes || uptime_secs < 60 * 60)
	{
		if (unlikely(snprintf(str + pos, str_len - pos,
			"%s%d %s", pos ? ", " : "",
			upminutes, upminutes != 1 ? "minutes" : "minute") < 0))
			PERROR_AND_EXIT("snprintf")
	}
}

static void
draw_info(const Flag * restrict flag)
{
	/// 1. Gathering
	struct sysinfo sys_info;
	if (unlikely(sysinfo(&sys_info) == -1)) PERROR_AND_EXIT("sysinfo")
	const char *username = getlogin();
	if (unlikely(!username)) PERROR_AND_EXIT("getlogin")
	struct utsname osname;
	if (unlikely(uname(&osname) == -1)) PERROR_AND_EXIT("uname")
	FILE *a_pipe = popen(". /etc/os-release && printf %s \"$NAME\"", "r");
	if (unlikely(!a_pipe))
	{
		if (!errno) errno = ENOMEM;
		PERROR_AND_EXIT("popen");
	}
	char distro_name[64] = "N/A";
	if (unlikely(!fgets(distro_name, sizeof distro_name, a_pipe) && ferror(a_pipe)))
	{
		perror("fgets");
		if (unlikely(pclose(a_pipe) == -1)) perror("pclose");
		exit(EXIT_FAILURE);
	}
	if (unlikely(pclose(a_pipe) == -1)) PERROR_AND_EXIT("pclose")

	/// 2. Drawing
	/// 2.1. Setting variables
	char primary[COLOR_BUFFER_SIZE];
	color256(primary, flag->rows[0], fg);
	char secondary[COLOR_BUFFER_SIZE];
	{
		size_t secondary_row = 0;
		while (secondary_row < flag->row_count - 1
			&& flag->rows[secondary_row] == flag->rows[0]) ++secondary_row;
		color256(secondary, flag->rows[secondary_row], fg);
	}
	// ensure 3:2 aspect ratio for terminal with 2x5 character size
	const size_t width = flag->row_count * 3.75 + .5;

	/// 2.2. Output
	if (unlikely(putchar('\n') == EOF)) PERROR_AND_EXIT("putchar")
	char row_info[256 + COLOR_BUFFER_SIZE - 1 + sizeof RESET - 1];
	for (size_t current_row = 0; current_row < flag->row_count; ++current_row)
	{
		switch (current_row)
		{
			case 0:
			{
				if (unlikely(snprintf(row_info, sizeof row_info,
					"%s\033[1m%s@%s" RESET, primary, username, osname.nodename)
					< 0)) PERROR_AND_EXIT("snprintf")
				break;
			}
			case 1:
			{
				if (unlikely(snprintf(row_info, sizeof row_info, "%sos%-10s%s",
					secondary, RESET, distro_name) < 0))
					PERROR_AND_EXIT("snprintf")
				break;
			}
			case 2:
			{
				if (unlikely(snprintf(row_info, sizeof row_info, "%sarch%-8s%s",
					secondary, RESET, osname.machine) < 0))
					PERROR_AND_EXIT("snprintf")
				break;
			}
			case 3:
			{
				char uptime[256];
				format_uptime(uptime, sizeof uptime, sys_info.uptime);
				if (unlikely(snprintf(row_info, sizeof row_info, "%suptime%-6s%s",
					secondary, RESET, uptime) < 0))
					PERROR_AND_EXIT("snprintf")
				break;
			}
			case 4:
			{
				if (unlikely(snprintf(row_info, sizeof row_info, "%skern%-8s%s",
					secondary, RESET, osname.release) < 0))
					PERROR_AND_EXIT("snprintf")
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
		if (unlikely(printf(" %s%s" RESET " %s\n", color, row, row_info) < 0))
			PERROR_AND_EXIT("printf")
	}
	if (unlikely(putchar('\n') == EOF)) PERROR_AND_EXIT("putchar")
}

static void
display_help(void)
{
#ifdef _GNU_SOURCE
	if (unlikely(puts("Options:\n"
		"  -f, --flag FLAG\n"
		"    Display the specified flag.\n"
		"  -c, --choose FLAG1, FLAG2, FLAGN\n"
		"    Choose a flag randomly from the specified list.\n"
		"  -l, --list\n"
		"    List all flags.") == EOF)) PERROR_AND_EXIT("puts")
#else
	if (unlikely(puts("Options:\n"
		"  -f FLAG\n"
		"    Display the specified flag.\n"
		"  -c FLAG1, FLAG2, FLAGN\n"
		"    Choose a flag randomly from the specified list.\n"
		"  -l\n"
		"    List all flags.") == EOF)) PERROR_AND_EXIT("puts")
#endif
}

static int
comparator(const void * restrict key, const void * restrict flag)
{
	return strcmp(key, ((const Flag *)flag)->name);
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
				const Flag *flag = bsearch(optarg, flags, NUMOF(flags),
					sizeof *flags, comparator);
				if (!flag)
				{
					if (unlikely(fputs("No flag with that name was found.\n",
						stderr) == EOF)) perror("fputs");
					return EXIT_FAILURE;
				}
				draw_info(flag);
				break;
			}
			case 'c': // kinda works
			{
				const Flag *choices[NUMOF(flags) * NUMOF(flags)];
				size_t i_choice = 0;
				for (char *token = strtok(optarg, ", "); token != NULL; token = strtok(NULL, ", "))
				{
					const Flag *flag = bsearch(token, flags, NUMOF(flags), sizeof *flags, comparator);
					if (!flag)
					{
						if (unlikely(fputs("No flag with that name was found.\n",
							stderr) == EOF)) perror("fputs");
						return EXIT_FAILURE;
					}
					choices[i_choice] = flag;
					if (++i_choice == NUMOF(choices)) break;
				}
				srand48(time(NULL));
				draw_info(choices[(size_t)(drand48() * i_choice)]);
				break;
			}
			case 'l':
			{
				if (unlikely(puts("Available flags:") == EOF))
					PERROR_AND_EXIT("puts")
				for (size_t flag = 0; flag < NUMOF(flags) - 1; ++flag)
					if (unlikely(printf("%s, ", flags[flag].name) < 0))
						PERROR_AND_EXIT("printf")
				if (unlikely(printf("%s.\n", flags[NUMOF(flags) - 1].name) < 0))
					PERROR_AND_EXIT("printf")
				break;
			}
			case 'h':
			{
				display_help();
				break;
			}
			default:
			{
				return EXIT_FAILURE;
			}
		}
	}
	if (optind == 1 || optind == 2 && !strcmp(argv[optind - 1], "--"))
	{
		display_help();
		return EXIT_FAILURE;
	}
}
