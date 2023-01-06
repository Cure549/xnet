#include "xnet_base.h"
#include "xnet_addon_chat.h"

int main(void)
{
	xnet_box_t *xnet = xnet_create("127.0.0.1", 47007, 5, 300);
	// xnet_integrate_chat_addon(xnet);
	xnet_start(xnet);
	xnet_destroy(xnet);
}