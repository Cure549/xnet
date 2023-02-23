#include "xnet_addon_chat.h"

chat_main_t chat_base;

static int assign_user_to_room(xnet_active_connection_t *client, int room_number, int seat_number);
static int remove_user_from_room(xnet_active_connection_t *client);
static bool is_room_name_taken(char *room_name);
static int find_room_with_name(char *room_name);
static int check_for_available_slot(int room_number);
static int get_my_room_number(xnet_active_connection_t *client);

int test_connect(xnet_box_t *xnet, xnet_active_connection_t *client)
{
    (void)xnet;
    (void)client;
    puts("CONNECT MSGGGG!!!!");
    return 0;
}

int test_disconnect(xnet_box_t *xnet, xnet_active_connection_t *client)
{
    (void)xnet;
    remove_user_from_room(client);
    return 0;
}

int xnet_integrate_chat_addon(xnet_box_t *xnet)
{
    xnet_insert_feature(xnet, CHAT_LOGIN_OP, chat_perform_login);
    xnet_insert_feature(xnet, CHAT_WHISPER_OP, chat_perform_whisper);
    xnet_insert_feature(xnet, CHAT_JOIN_OP, chat_perform_join_room);
    xnet_insert_feature(xnet, CHAT_SHOUT_OP, chat_perform_shout);
    xnet_addon_callback(xnet, ON_CLIENT_CONNECT, test_connect);
    xnet_addon_callback(xnet, ON_CLIENT_DISCONNECT, test_disconnect);
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

    /* Make sure message is not being sent to self. */
    if (client->account->username == desired_user->account->username) {
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
    int return_code = 0;

    printf("Socket [%d] is performing 'chat_perform_join_room()'\n", client->socket);

    if (NULL == xnet) {
        return_code = RC_FAILED_JOIN_ROOM;
        goto return_packet;
    }

    if (NULL == client) {
        return_code = RC_FAILED_JOIN_ROOM;
        goto return_packet;
    }

    chat_join_room_packet_t packets = {0};

    read(client->socket, &packets.from_client.room_name_length, sizeof(int));
    packets.from_client.room_name_length = ntohl(packets.from_client.room_name_length);

    read(client->socket, &packets.from_client.room_name, MAX_ROOM_NAME_LEN);

    int room_number = find_room_with_name((char *)packets.from_client.room_name);
    if (-1 == room_number) {
        return_code = RC_FAILED_JOIN_ROOM;
        goto return_packet;
    }

    int seat_number = check_for_available_slot(room_number);
    if (-1 == seat_number) {
        return_code = RC_FAILED_JOIN_ROOM;
        goto return_packet;
    }

    /* Is user in room? */
    int in_room = get_my_room_number(client);
    if (-1 != in_room) {
        /* Attempts to remove user from their current room. */
        int try_remove = remove_user_from_room(client);
        if (0 != try_remove) {
            return_code = RC_FAILED_JOIN_ROOM;
            goto return_packet;
        }
    }

    int try_join = assign_user_to_room(client, room_number, seat_number);
    if (0 != try_join) {
        return_code = RC_FAILED_JOIN_ROOM;
        goto return_packet;
    }

    printf("%s got assigned to room %s\n", client->account->username, packets.from_client.room_name);

    /* Send feedback to client. */
return_packet:
    packets.to_client.opcode_relation = htons(CHAT_JOIN_OP);
    packets.to_client.return_code = htons(return_code);
    send(client->socket, &packets.to_client, sizeof(packets.to_client), 0);

    printf("Socket [%d] finished performing 'chat_perform_join_room()' with code [%d]\n", client->socket, return_code);
    return 0;
}

int chat_perform_shout(xnet_box_t *xnet, xnet_active_connection_t *client)
{
    int return_code = 0;

    printf("Socket [%d] is performing 'chat_perform_shout()'\n", client->socket);

    if (NULL == xnet) {
        return_code = RC_FAILED_SHOUT;
        goto return_packet;
    }

    if (NULL == client) {
        return_code = RC_FAILED_SHOUT;
        goto return_packet;
    }

    chat_shout_packet_t packets = {0};

    // parse_packet(
    //     .in = client->socket;
    //     .out = &packets;
    //     .formula = ""
    // );

    read(client->socket, &packets.from_client.msg_length, sizeof(int));
    packets.from_client.msg_length = ntohl(packets.from_client.msg_length);

    read(client->socket, &packets.from_client.msg, packets.from_client.msg_length);

    int room_number = get_my_room_number(client);
    if (-1 == room_number) {
        return_code = RC_FAILED_SHOUT;
        goto return_packet;
    }

    /* Create whisper packet details */
    chat_whisper_packet_t dupe_whisper = {0};
    dupe_whisper.to_target.opcode_relation = htons(CHAT_WHISPER_TARGET);
    strncpy(dupe_whisper.to_target.from_username, client->account->username, XNET_MAX_USERNAME_LEN);
    dupe_whisper.to_target.from_username_length = htonl(strnlen(dupe_whisper.to_target.from_username, XNET_MAX_USERNAME_LEN));
    strncpy(dupe_whisper.to_target.msg, packets.from_client.msg, packets.from_client.msg_length);
    dupe_whisper.to_target.msg_length = htonl(packets.from_client.msg_length);

    /* Send message packet to every user in room. */
    for (size_t n = 0; n < MAX_USERS_IN_ROOM; n++) {
        xnet_active_connection_t *current_user = chat_base.rooms[room_number].users[n];
        /* Don't shout at yourself!! */
        if (client == current_user) {
            continue;
        }

        /* Shout at everyone else!! */
        if (NULL != current_user) {
            send(current_user->socket, &dupe_whisper.to_target, sizeof(dupe_whisper.to_target), 0);
        }
    }

    printf("%s shouted %s in room %s\n", client->account->username, packets.from_client.msg, chat_base.rooms[room_number].name);

    /* Send feedback to client. */
return_packet:
    packets.to_client.opcode_relation = htons(CHAT_SHOUT_OP);
    packets.to_client.return_code = htons(return_code);
    send(client->socket, &packets.to_client, sizeof(packets.to_client), 0);

    printf("Socket [%d] finished performing 'chat_perform_shout()' with code [%d]\n", client->socket, return_code);
    return 0;
}

int chat_create_room(char *room_name)
{
    int err = 0;

    /* NULL Check */
    if (NULL == room_name) {
        err = E_GEN_NULL_PTR;
        goto handle_err;
    }

    /* Ensure room name isn't already used. */
    bool room_taken = is_room_name_taken(room_name);
    if (room_taken) {
        err = E_GEN_OUT_RANGE;
        goto handle_err;
    }

    /* Assume room is not available. */
    int avail_room = -1;

    /* Find a room index. */
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

static int get_my_room_number(xnet_active_connection_t *client)
{
    int err = 0;

    /* NULL Check */
    if (NULL == client) {
        err = E_GEN_NULL_PTR;
        goto handle_err;
    }

    int found_rm_index = -1;

    for (size_t rm_idx = 0; (rm_idx < MAX_ROOM_COUNT && -1 == found_rm_index); rm_idx++) {
        for (size_t usr_idx = 0; (usr_idx < MAX_USERS_IN_ROOM && -1 == found_rm_index); usr_idx++) {
            if (client == chat_base.rooms[rm_idx].users[usr_idx]) {
                // This assignment makes the above conditionals no longer valid, causing the parent loop to break.
                found_rm_index = rm_idx;
            }
        }
    }

    return found_rm_index;

    /* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "get_my_room_number()");
    return -1; 
}

static bool is_room_name_taken(char *room_name)
{
    int err = 0;
    bool taken = false;

    if (NULL == room_name) {
        err = E_GEN_NULL_PTR;
        goto handle_err;
    }

    /* Loop through every room, if room name matches, break out and notify return. */
    for (size_t n = 0; n < MAX_ROOM_COUNT; n++) {
        if (NULL != chat_base.rooms[n].name && 0 == strncmp(chat_base.rooms[n].name, room_name, MAX_ROOM_NAME_LEN)) {
           taken = true;
           break;
        }
    }

    return taken;

    /* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "is_room_name_taken()");
    return true;
}

static int find_room_with_name(char *room_name)
{
    int err = 0;

    if (NULL == room_name) {
        err = E_GEN_NULL_PTR;
        goto handle_err;
    }

    /* Start off assuming room is not found. */
    int room_number = -1;

    /* Get index of room with match. */
    for (size_t n = 0; n < MAX_ROOM_COUNT; n++) {
        if (NULL == chat_base.rooms[n].name) {
            continue;
        }
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

    return room_number;

    /* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "find_room_with_name()");
    return -1;
}

static int check_for_available_slot(int room_number)
{
    int err = 0;
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

    return avail_slot;

    /* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "check_for_available_slot()");
    return -1;
}

static int remove_user_from_room(xnet_active_connection_t *client)
{
    int err = 0;

    /* NULL Check */
    if (NULL == client) {
        err = E_GEN_NULL_PTR;
        goto handle_err;
    }

    if (NULL == client->account) {
        err = E_GEN_NULL_PTR;
        goto handle_err;
    }

    /* Require client to be logged in. */
    if (false == client->account->is_logged_in) {
        err = E_GEN_OUT_RANGE;
        goto handle_err;
    }

    /* Get the current room that user resides in. */
    int room_number = get_my_room_number(client);
    if (-1 == room_number) {
        err = E_GEN_NEGATIVE_NUM;
        goto handle_err;
    }

    /* Search for them in the room, and remove them. */
    for (size_t n = 0; n < MAX_USERS_IN_ROOM; n++) {
        if (client == chat_base.rooms[room_number].users[n]) {
            // Removes user from room
            chat_base.rooms[room_number].users[n] = NULL;
            printf("%s got removed from room %s\n", client->account->username, chat_base.rooms[room_number].name);
            break;
        }
    }

    return 0;

    /* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "remove_user_from_room()");
    return err;
}

static int assign_user_to_room(xnet_active_connection_t *client, int room_number, int seat_number)
{
    int err = 0;

    /* NULL Check */
    if (NULL == client) {
        err = E_GEN_NULL_PTR;
        goto handle_err;
    }

    /* Require client to be logged in. */
    if (false == client->account->is_logged_in) {
        err = E_GEN_OUT_RANGE;
        goto handle_err;
    }

    /* Assign user. */
    chat_base.rooms[room_number].users[seat_number] = client;

    return 0;

    /* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "assign_user_to_room()");
    return err;
}