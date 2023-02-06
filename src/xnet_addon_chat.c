#include "xnet_addon_chat.h"

chat_main_t chat_base;
pthread_mutex_t main_mutex;

static int assign_user_to_room(xnet_active_connection_t *client, char *room_name);

int xnet_integrate_chat_addon(xnet_box_t *xnet)
{
    xnet_insert_feature(xnet, 201, chat_perform_send_msg);
    xnet_insert_feature(xnet, 202, chat_perform_join_room);
    xnet_insert_feature(xnet, 203, chat_perform_debug);
    pthread_mutex_init(&main_mutex, NULL);
    return 0;
}

int chat_perform_send_msg(xnet_box_t *xnet, xnet_active_connection_t *client)
{
    // pthread_mutex_lock(&main_mutex);
    puts("entered chat_perform_send_msg");
    chat_send_msg_root_t packet_root = {0};
    read(client->client_event.data.fd, &packet_root.from_client, sizeof(packet_root.from_client));
    printf("%d (%s)\n", ntohl(packet_root.from_client.length), packet_root.from_client.msg);
    usleep(3000000);
    // (void)xnet;
    // puts("send_msg");

    int t = send(client->client_event.data.fd, "SERVER RESP", 11, 0);
    printf("%d\n", t);

    // xnet_create_user(xnet->userbase, (char *)"admin", (char *)"password", 3);
    (void)xnet;
    (void)client;
    puts("leaving chat_perform_send_msg");
    
    // pthread_mutex_unlock(&main_mutex);
    return 0;
}

int chat_perform_join_room(xnet_box_t *xnet, xnet_active_connection_t *client)
{
    (void)xnet;
    (void)client;
    puts("join room");
    xnet_login_user(xnet->userbase, (char *)"admin", (char *)"password", client);
    assign_user_to_room(client, (char *)"The Hub");
    printf("%s joined room %s\n", client->account->username, "The Hub");
    return 0;
}

int chat_perform_debug(xnet_box_t *xnet, xnet_active_connection_t *client)
{
    (void)xnet;
    (void)client;
    xnet_print_userbase(xnet->userbase);
    return 0;
}

int chat_create_room(const char *room_name)
{
    int err = 0;

    /* NULL Check */
    if (NULL == room_name) {
        err = E_GEN_NULL_PTR;
        goto handle_err;
    }

    /* Assume room is not available. */
    int avail_room = -1;

    /* Find a room. */
    for (size_t n = 0; n < MAX_ROOM_COUNT; n++) {
        /* Room found. */
        if (NULL == chat_base.rooms[n].name) {
            avail_room = n;
            break;
        }
    }

    /* Check if room was created. */
    if (-1 == avail_room) {
        err = E_GEN_NEGATIVE_NUM;
        goto handle_err;
    }

    /* Assign name. */
    chat_base.rooms[avail_room].name = room_name;

    return 0;

    /* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "chat_create_room()");
    return err;
}

// static int remove_user_from_room(xnet_active_connection_t *client, char *room_name)
// {

// }

static int assign_user_to_room(xnet_active_connection_t *client, char *room_name)
{
    int err = 0;

    /* NULL Check */
    if (NULL == client) {
        err = E_GEN_NULL_PTR;
        goto handle_err;
    }

    if (NULL == room_name) {
        err = E_GEN_NULL_PTR;
        goto handle_err;
    }

    /* Start off assuming room is not found. */
    int room_number = -1;

    /* Get index of room with match. */
    for (size_t n = 0; n < MAX_ROOM_COUNT; n++) {
        /* Room found if true. */
        if (0 == strncmp(chat_base.rooms[n].name, room_name, MAX_ROOM_NAME_LEN)) {
            room_number = n;
            break;
        }
    }

    /* Check if room was found. */
    if (-1 == room_number) {
        err = E_GEN_NEGATIVE_NUM;
        goto handle_err;
    }

    int avail_slot = -1;

    /* Find available spot in room. */
    for (size_t n = 0; n < MAX_USERS_IN_ROOM; n++) {
        if (NULL == chat_base.rooms[room_number].users[n]) {
            avail_slot = n;
            break;
        }
    }

    /* Check if slot was found. */
    if (-1 == avail_slot) {
        err = E_GEN_NEGATIVE_NUM;
        goto handle_err;
    }

    /* Assign user. */
    chat_base.rooms[room_number].users[avail_slot] = client;

    return 0;

    /* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "assign_user_to_room()");
    return err;
}