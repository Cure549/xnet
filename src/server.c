#include "xnet_base.h"
#include "xnet_addon_chat.h"

int main(void)
{
	xnet_box_t *xnet = xnet_create("127.0.0.1", 47007, 5, 300);
	if (NULL == xnet) {
		return -1;
	}
	xnet_integrate_chat_addon(xnet);
	xnet_create_user(xnet->userbase, (char *)"admin", (char *)"password", 3);
	xnet_create_user(xnet->userbase, (char *)"bob", (char *)"1234", 2);
	xnet_create_user(xnet->userbase, (char *)"tim", (char *)"spaces :(", 1);
	xnet_print_userbase(xnet->userbase);
	chat_create_room("The Hub");
	xnet_start(xnet);
	xnet_destroy(xnet);
}