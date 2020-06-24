
long sys_open(const char *filename, int flags, int mode);
long sys_close(unsigned int fd);
int sys_read(unsigned int fd, char *buf, size_t count);
int sys_write(unsigned int fd, char *buf, size_t count);
