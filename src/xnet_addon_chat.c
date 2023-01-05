#include "xnet_addon_chat.h"
/* 
Contains all necessary structs and functions related to adding chat server capabilities to a xnet server. 
*/

int xnet_integrate_chat_addon(xnet_box_t *xnet)
{
    xnet_insert_feature(xnet, 201, chat_perform_send_msg);
    return 0;
}

int chat_perform_send_msg(xnet_box_t *xnet, xnet_active_connection_t *client)
{
    (void)xnet;
    chat_send_msg_root_t packet_root = {0};
    read(client->client_event.data.fd, &packet_root.from_client, sizeof(packet_root.from_client));
    printf("%d (%s)\n", ntohl(packet_root.from_client.length), packet_root.from_client.msg);
    return 0;
}