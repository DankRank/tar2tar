#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "tarhelpers.h"
int main(int argc, char *argv[])
{
	char namebuf[4096 + 1];
	char buf[512];
	int fd;
	long offset = 0;
	int has_longname = 0;

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

		if (!is_extended_type(buf[156])) {
			if (has_longname)
				printf("%ld %s\n", offset, namebuf);
			else if (is_ustar(buf) && buf[345])
				printf("%ld %.155s/%.100s\n", offset, &buf[345], &buf[0]);
			else
				printf("%ld %.100s\n", offset, &buf[0]);
			has_longname = 0;
			if ((offset = lseek(fd, get_size(buf), SEEK_CUR)) == -1) {
				perror("lseek");
				return 1;
			}
		} else {
			int rest = get_size(buf);
			if (buf[156] == 'L') {
				int toread = rest < 4096 ? rest : 4096;
				rest -= toread;
				if (read_fully(fd, namebuf, toread))
					return 1;
				namebuf[toread] = '\0';
				has_longname = 1;
			}
			if (lseek(fd, rest, SEEK_CUR) == -1) {
				perror("lseek");
				return 1;
			}
		}
	}
}
