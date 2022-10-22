#include "xnet_base.h"

int main(void)
{
	xnet_box *xnet = xnet_create("127.0.0.1", 47007, 5, 100);
	xnet_start(xnet);
	xnet_destroy(xnet);
}