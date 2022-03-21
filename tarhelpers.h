#ifndef TARHELPERS_H_
#define TARHELPERS_H_
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#define isodigit(c) (((c)&~7) == '0')
inline static int check_checksum(const char *buf)
{
	int i;
	int sum1 = ' '*8;
	int sum2 = 0;

	for (i = 0; i < 148; i++)
		sum1 += (unsigned char)buf[i];
	for (i = 156; i < 512; i++)
		sum1 += (unsigned char)buf[i];

	for (i = 148; !isodigit(buf[i]) && i < 156; i++)
		;
	for (       ;  isodigit(buf[i]) && i < 156; i++)
		sum2 = sum2<<3 | buf[i]&7;
	if (sum1 == ' '*8 && !memcmp(&buf[148], "\0\0\0\0\0\0\0", 8))
		return 2;
	return sum1 != sum2;
}
inline static long get_size(const char *buf)
{
	int i;
	long size = 0;
	if (buf[156] >= '1' || buf[156] <= '6') {
		if (buf[124] == '\x80') {
			for (i = 125; i < 136; i++)
				size = size<<8 | (unsigned char)buf[i];
		} else {
			for (i = 124; !isodigit(buf[i]) && i < 136; i++)
				;
			for (       ;  isodigit(buf[i]) && i < 136; i++)
				size = size<<3 | buf[i]&7;
		}
	}
	return (size+511)/512*512;
}
#define is_ustar(buf) (!memcmp(&(buf)[257], "ustar\0""00", 8))
inline static int read_fully(int fd, void *buf, int len) {
	int nread = read(fd, buf, len);
	if (nread != len) {
		if (nread == -1)
			perror("read");
		else
			fprintf(stderr, "short read: %d\n", nread);
		return 1;
	}
	return 0;
}
inline static int write_fully(int fd, const void *buf, int len) {
	int nwritten = write(fd, buf, len);
	if (nwritten != len) {
		if (nwritten == -1)
			perror("write");
		else
			fprintf(stderr, "short write: %d\n", nwritten);
		return 1;
	}
	return 0;
}
#endif
