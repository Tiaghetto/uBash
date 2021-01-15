all:
	gcc main.c commands.c parsing.c process_handling.c memory_handling.c error_handling.c -Wno-incompatible-pointer-types -lreadline
