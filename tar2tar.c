#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "tarhelpers.h"
int main(int argc, char *argv[])
{
	char buf[8192];
	int fd;
	long size;
	int is_extended;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s [archive]\n", argv[0]);
		return 1;
	}
	
	if ((fd = open(argv[1], O_RDONLY)) == -1) {
		perror("open");
		return 1;
	}

	while (fgets(buf, 512, stdin)) {
		if (lseek(fd, strtol(buf, NULL, 0), SEEK_SET) == -1) {
			perror("lseek");
			return 1;
		}
		do {
			if (read_fully(fd, buf, 512))
				return 1;

			switch (check_checksum(buf)) {
			case 1:
				fprintf(stderr, "checksum error\n");
				return 1;
			case 2:
				memset(buf, 0, 1024);
				if (write_fully(STDOUT_FILENO, buf, 1024))
					return 1;
				return 0;
			}

			is_extended = is_extended_type(buf[156]);
			if (write_fully(STDOUT_FILENO, buf, 512))
				return 1;
			size = get_size(buf);
			while (size) {
				int toread = size > 8192 ? 8192 : size;
				if (read_fully(fd, buf, toread))
					return 1;
				if (write_fully(STDOUT_FILENO, buf, toread))
					return 1;
				size -= toread;
			}
		} while (is_extended);
	}
}
