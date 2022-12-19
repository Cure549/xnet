#include "xnet_addon_chat.h"
/* 
Contains all necessary structs and functions related to adding chat server capabilities to a xnet server. 
*/

int xnet_integrate_chat_addon(xnet_box_t *xnet)
{
    // Example of what needs to be done:
    // hashmap_insert(xnet->addon_hashmap, 201, chat_perform_send_msg);
    // hashmap_insert(xnet->addon_hashmap, 202, chat_perform_join_room);
    // hashmap_insert(xnet->addon_hashmap, 203, chat_perform_leave_room);
    // hashmap_insert(xnet->addon_hashmap, 204, chat_perform_show_rooms);

    (void) xnet;
    return 3;
}