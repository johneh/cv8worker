#include <stdint.h>
struct ipaddr_s;
struct mill_fd_s;

int ssl_read(struct mill_fd_s *mfd, void *buf, int len, int64_t deadline);
int ssl_write(struct mill_fd_s *mfd, const void *buf, int len, int64_t deadline);
struct mill_fd_s *ssl_connect(struct ipaddr_s *addr, int64_t deadline);
struct mill_fd_s *ssl_accept(struct mill_fd_s *lsock, int64_t deadline);
int ssl_serv_init(const char *cert_file, const char *key_file);
const char *ssl_errstr(struct mill_fd_s *mfd);

int ssl_handshake(struct mill_fd_s *mfd, int64_t deadline);
void ssl_close(struct mill_fd_s *mfd);
void ssl_closefd(struct mill_fd_s *mfd);
