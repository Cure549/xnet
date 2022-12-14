#include "xnet_base.h"

int main(void)
{
	xnet_box_t *xnet = xnet_create("127.0.0.1", 47007, 5, 5);
	xnet_start(xnet);
	xnet_destroy(xnet);
}