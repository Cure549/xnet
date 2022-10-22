#include "xnet_utils.h"
#include <fcntl.h>

/* 
Any general utilities associated with aiding in server configuration and customization.

// Informs xnet server which operations from an addon should be allowed/are supported.
xnet_add_op_to_whitelist(server *, operation enum value);

// Informs xnet server of a specific addon to include in its suite of options.
xnet_give_addon(server *, XNET_FTP_ID);

// Generates a random session id, and returns it.
xnet_get_new_session(void);

// Compares a given session id, to a active user's session id.
xnet_is_session_valid(size_t session_id, user)

*/

int set_non_blocking(int sockfd)
{
	int flags, s;
	flags = fcntl(sockfd, F_GETFL, 0);
	if(flags == -1)
	{
		perror("fcntl");
		return -1;
	}
	flags |= O_NONBLOCK;
	s = fcntl(sockfd, F_SETFL, flags);
	if(s == -1)
	{
		perror("fcntl");
		return -1;
	}
	return 0;
}

void nfree(void **ptr)
{
    /* Avoids double free. */
    if (NULL == *ptr) {
        return;
    }

	free(*ptr);
	*ptr = NULL;
}