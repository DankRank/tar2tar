#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "tarhelpers.h"
int main(int argc, char *argv[])
{
	char buf[512];
	int fd;
	long offset = 0;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s [archive]\n", argv[0]);
		return 1;
	}
	
	if ((fd = open(argv[1], O_RDONLY)) == -1) {
		perror("open");
		return 1;
	}

	for (;;) {
		if (read_fully(fd, buf, 512))
			return 1;

		switch (check_checksum(buf)) {
		case 1:
			fprintf(stderr, "checksum error\n");
			return 1;
		case 2:
			return 0;
		}

		if (is_ustar(buf) && buf[345])
			printf("%ld %.155s/%.100s\n", offset, &buf[345], &buf[0]);
		else
			printf("%ld %.100s\n", offset, &buf[0]);

		if ((offset = lseek(fd, get_size(buf), SEEK_CUR)) == -1) {
			perror("lseek");
			return 1;
		}
	}
}
