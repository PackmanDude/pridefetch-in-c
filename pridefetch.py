#!/usr/bin/env python3

import argparse
import distro
import random
from getpass import getuser
from socket import gethostname
from time import strftime
from time import gmtime
from subprocess import run

# Define a dictionary of all the flags and their colors
# Each color is the color for an individual row in the flag
flags = {
	'STRAIGHT':		[0, 255, 0, 255, 0],
#	'RAINBOW':		[196, 208, 226, 28, 20, 90],
#	'GAYMEN':		[23, 43, 115, 255, 117, 57, 55],
#	'BISEXUAL':		[198, 198, 97, 25, 25],
#	'LESBIAN':		[202, 209, 255, 255, 168, 161],
#	'PANSEXUAL':	[198, 198, 220, 220, 39, 39],
	'TRANS':		[81, 211, 255, 211, 81],
#	'NONBINARY':	[226, 226, 255, 255, 98, 98, 237, 237],
#	'DEMIBOY':		[244, 249, 117, 255, 117, 249, 244],
#	'DEMIGIRL':		[244, 249, 218, 255, 218, 249, 244],
#	'GENDERFLUID':	[211, 255, 128, 0, 63],
#	'AROMANTIC':	[71, 149, 255, 249, 0],
#	'AGENDER':		[0, 251, 255, 149, 255, 251, 0],
	'ASEXUAL':		[0, 0, 242, 242, 255, 255, 54, 54],
#	'GRAYSEXUAL':	[54, 242, 255, 242, 54],
	'UKRAINIAN':	[33, 33, 226, 226],
	'UPA':			[88, 88, 0, 0]
}

def color256(col, bg_fg):
	# Hacky alias around manually typing out escape codes every time (c) megabytesofrem
	return f'\033[{48 if bg_fg == "bg" else 38};5;{col}m'

def draw_info(flag_name):
	flag_name = flag_name.upper()	# Case-insensitive input
	isTwo_color = False
	# Workaround for flags with even count of rows: removes string with
	# architecture to prevent going underneath or line duplication (basically
	# UB for that shit).
	width = 20
	if flag_name == 'UKRAINIAN' or flag_name == 'UPA': isTwo_color=True
	if isTwo_color: width=15

	flag = flags[flag_name]
	curr_row = 0

	# Store the output of uname -srm
	uname_info = run(['uname', '-srm'], capture_output=True)
	uname_info = uname_info.stdout.decode().strip()

	# Make sure that the row color is different to the color of the hostname
	row_color = color256(flag[1] if flag[0] != flag[1] else flag[2], 'fg')

	reset = '\033[0m\033[39m'

	print()
	for row in flag:	# Just works™
		# Alternate displaying the information based on the current row
		if curr_row == 0:
			color = color256(flag[curr_row], 'fg')
			user = getuser()
			host = gethostname()
			row_info = f'{color}\033[1m{user}@{host}{reset}'
		elif curr_row == 1:
			distribution = distro.name()
			row_info = f'{row_color}os      {reset}{distribution or "N/A"}'
		elif curr_row == 2 and isTwo_color:
			kernel = uname_info.split(' ')[1]
			row_info = f'{row_color}kern    {reset}{kernel}'
		elif curr_row == 2 and not isTwo_color:
			arch = uname_info.split(' ')[2]
			row_info = f'{row_color}arch    {reset}{arch}'
		elif curr_row == 3 and isTwo_color:
			uptime = run(['uptime', '-p'], capture_output=True)
			time = uptime.stdout.decode().partition(' ')[2]
			row_info = f'{row_color}uptime  {reset}{time}'
		elif curr_row == 3 and not isTwo_color:
			kernel = uname_info.split(' ')[1]
			row_info = f'{row_color}kern    {reset}{kernel}'
		elif curr_row == 4:
			uptime = run(['uptime', '-p'], capture_output=True)
			time = uptime.stdout.decode().partition(' ')[2]
			row_info = f'{row_color}uptime  {reset}{time}'
		else:
			row_info = ''

		if row != 'P':	# Still no shit what this supposed to mean
			print(f' {color256(row, "bg")}{" " * width}\033[49m {row_info}')
		else:
			# Print just the info, along with the padding for the box
			# Guy above me, no box!!!! Your done
			print(f' {" " * width}\033[49m {row_info}')

		curr_row += 1

def main():
	parser = argparse.ArgumentParser()
	parser.add_argument('-f', '--flag', help='Displays the chosen FLAG')
	parser.add_argument('-c', '--choose', help='Choose a flag at random from this list')
	parser.add_argument('-l', '--list', action='store_true', help='Lists all the flags')

	args = parser.parse_args()

	if args.flag:
		draw_info(args.flag)
	else:
		draw_info('UPA')
	if args.choose:
		# Choose a flag at random from a list of comma-seperated flags
		flag_choices = args.choose.split(',')
		draw_info(random.choice(flag_choices))
	if args.list:
		print('Available flags:')
		print(', '.join(flags))

if __name__ == '__main__':
	main()
