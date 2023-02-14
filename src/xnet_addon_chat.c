#include "xnet_addon_chat.h"

chat_main_t chat_base;
pthread_mutex_t chat_lock;

static int assign_user_to_room(xnet_active_connection_t *client, char *room_name);

int xnet_integrate_chat_addon(xnet_box_t *xnet)
{
    xnet_insert_feature(xnet, CHAT_LOGIN_OP, chat_perform_login);
    xnet_insert_feature(xnet, CHAT_WHISPER_OP, chat_perform_whisper);
    xnet_insert_feature(xnet, CHAT_JOIN_OP, chat_perform_join_room);
    xnet_insert_feature(xnet, CHAT_DEBUG_OP, chat_perform_debug);
    pthread_mutex_init(&chat_lock, NULL);
    return 0;
}

int chat_perform_login(xnet_box_t *xnet, xnet_active_connection_t *client)
{
    int return_code = RC_ACTION_SUCCESS;

    printf("Socket [%d] is performing 'chat_perform_login()'\n", client->socket);

    chat_login_packet_t packets = {0};
    read(client->socket, &packets.from_client.username_length, sizeof(int));

    /* Ensure username is proper length. */
    packets.from_client.username_length = ntohl(packets.from_client.username_length);
    if (XNET_MAX_USERNAME_LEN < packets.from_client.username_length) {
        return_code = RC_FAILED_LOGIN;
        goto return_packet;
    }

    read(client->socket, &packets.from_client.username, packets.from_client.username_length);

    read(client->socket, &packets.from_client.password_length, sizeof(int));

    /* Ensure password is proper length. */
    packets.from_client.password_length = ntohl(packets.from_client.password_length);
    if (XNET_MAX_PASSWD_LEN < packets.from_client.password_length) {
        return_code = RC_FAILED_LOGIN;
        goto return_packet;
    }

    read(client->socket, &packets.from_client.password, packets.from_client.password_length);

    /* Attempt to login to account. */
    int login_attempt = xnet_login_user(xnet->userbase, packets.from_client.username, packets.from_client.password, client);
    if (0 != login_attempt) {
        return_code = RC_FAILED_LOGIN;
        goto return_packet;
    }

/* Send feedback to client. */
return_packet:
    packets.to_client.opcode_relation = htons(CHAT_LOGIN_OP);
    packets.to_client.return_code = htons(return_code);
    send(client->socket, &packets.to_client, sizeof(packets.to_client), 0);

    printf("Socket [%d] finished performing 'chat_perform_login()' with code [%d]\n", client->socket, return_code);
    return 0;
}

int chat_perform_whisper(xnet_box_t *xnet, xnet_active_connection_t *client)
{
    int return_code = 0;

    printf("Socket [%d] is performing 'chat_perform_whisper()'\n", client->socket);

    chat_whisper_packet_t packets = {0};

    if (false == client->account->is_logged_in) {
        return_code = RC_FAILED_WHISPER;
        goto return_packet;
    }

    /* ----- CAPTURE WHISPER DATA FROM THE INITIATING CLIENT ----- */
    read(client->socket, &packets.from_client.to_username_length, sizeof(int));

    /* Ensure username is proper length. */
    packets.from_client.to_username_length = ntohl(packets.from_client.to_username_length);
    if (XNET_MAX_USERNAME_LEN < packets.from_client.to_username_length) {
        return_code = RC_FAILED_WHISPER;
        goto return_packet;
    }

    read(client->socket, &packets.from_client.to_username, packets.from_client.to_username_length);

    /* Ensure message length is within the limit. */
    read(client->socket, &packets.from_client.msg_length, sizeof(int));
    packets.from_client.msg_length = ntohl(packets.from_client.msg_length);
    if (MAX_MESSAGE_LENGTH < packets.from_client.msg_length) {
        return_code = RC_FAILED_WHISPER;
        goto return_packet;
    }

    read(client->socket, &packets.from_client.msg, packets.from_client.msg_length);
    /* ----------------------------------------------------------- */

    /* ----- TRY TO SEND MESSAGE TO DESIRED USER ----- */
    xnet_active_connection_t *desired_user = xnet_get_conn_by_user(xnet, packets.from_client.to_username);
    if (NULL == desired_user) {
        return_code = RC_FAILED_WHISPER;
        goto return_packet;
    }

    /* Create packet details */
    packets.to_target.opcode_relation = htons(CHAT_WHISPER_TARGET);
    strncpy(packets.to_target.from_username, client->account->username, XNET_MAX_USERNAME_LEN);
    packets.to_target.from_username_length = htonl(strnlen(packets.to_target.from_username, XNET_MAX_USERNAME_LEN));
    strncpy(packets.to_target.msg, packets.from_client.msg, MAX_MESSAGE_LENGTH);
    packets.to_target.msg_length = htonl(packets.from_client.msg_length);

    send(desired_user->socket, &packets.to_target, sizeof(packets.to_target), 0);

    /* ----------------------------------------------- */



    /* Send feedback to client. */
return_packet:
    packets.to_client.opcode_relation = htons(CHAT_WHISPER_OP);
    packets.to_client.return_code = htons(return_code);
    send(client->socket, &packets.to_client, sizeof(packets.to_client), 0);

    printf("Socket [%d] finished performing 'chat_perform_whisper()' with code [%d]\n", client->socket, return_code);
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

    /* Require client to be logged in. */
    if (false == client->account->is_logged_in) {
        err = E_GEN_OUT_RANGE;
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