#include "xnet_userbase.h"

static void free_all_entries(xnet_user_t *user_entry);

int xnet_create_user(xnet_userbase_group_t *base, char *user, char *pass, int new_perm)
{
    int err = 0;

    /* NULL Check */
    if (NULL == base) {
        err = E_GEN_NULL_PTR;
        goto handle_err;
    }

    if (NULL == user) {
        err = E_GEN_NULL_PTR;
        goto handle_err;
    }

    if (NULL == pass) {
        err = E_GEN_NULL_PTR;
        goto handle_err;
    }

    /* Loop through linked list until a spot is found. While also remembering previous node. */
    xnet_user_t *prev = NULL;
    xnet_user_t *current = base->head;

    while (NULL != current) {
        /* If an account exists with same username, don't create user. */
        if (0 == strncmp(user, current->username, XNET_MAX_USERNAME_LEN)) {
            err = E_SRV_USER_EXISTS;
            goto handle_err;
        }

        /* Make loop continue to progress. */
        prev = current;
        current = current->next;
    }

    /* Allocate space for new node. */
    current = calloc(1, sizeof(xnet_user_t));
    if (NULL == current) {
        err = E_GEN_FAIL_ALLOC;
        goto handle_err;
    }

    /* Configure node. */
    current->username = strndup(user, XNET_MAX_USERNAME_LEN);
    current->password = strndup(pass, XNET_MAX_PASSWD_LEN);
    current->perm_level = new_perm;
    current->prev = prev;
    current->next = NULL;

    /* If 'parent' is not NULL, we are inside the list.
       If 'prev' is NULL, it's an indication of looking at the head node as the head is the only node with a NULL previous node.
    */
    if (NULL != prev) {
        prev->next = current;
    } else {
        base->head = current;
    }

    /* Keep track of how many nodes there are. */
    base->count++;

	return err;

/* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "xnet_create_user()");
    return err;
}

int xnet_delete_user(xnet_userbase_group_t *base, char *user)
{
    int err = 0;

    /* NULL Check */
    if (NULL == base) {
        err = E_GEN_NULL_PTR;
        goto handle_err;
    }

    if (NULL == user) {
        err = E_GEN_NULL_PTR;
        goto handle_err;
    }

    /* Loop through linked list until a match is found. */
    xnet_user_t *current = base->head;
    bool user_found = false;
    while (NULL != current) {
        /* If this condition is met, 'current' is pointing to the argued user. */
        if (0 == strncmp(current->username, user, XNET_MAX_USERNAME_LEN)) {
            user_found = true;
            break;
        }
        current = current->next;
    }

    if (false == user_found) {
        err = E_SRV_USER_NOT_EXIST;
        goto handle_err;
    }

    /* If the head node is the one being argued, assign the head to the next node. 
       If not head node, tell the parent node to link to the next node. 
    */
    if (NULL == current->prev) {
        base->head = current->next;
    } else {
        current->prev = current->next;
    }

    /* Release the selected node's memory. */
    nfree((void **)&current->username);
    nfree((void **)&current->password);
    nfree((void **)&current);

    /* Keep track of how many nodes there are. */
    base->count--;

	return err;

/* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "xnet_delete_user()");
    return err;
}

void xnet_print_userbase(xnet_userbase_group_t *base)
{
    int err = 0;

    /* NULL Check */
    if (NULL == base) {
        err = E_GEN_NULL_PTR;
        goto handle_err;
    }

    /* Loop through linked list. */
    xnet_user_t *current = base->head;
    while (current != NULL) {
        printf("%s | %s | %d\n", current->username, current->password, current->perm_level);
        current = current->next;
    }

	return;

/* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "xnet_print_userbase()");
    return;
}

void xnet_destroy_userbase(xnet_userbase_group_t *base)
{
    int err = 0;

    /* NULL Check */
    if (NULL == base) {
        err = E_GEN_NULL_PTR;
        goto handle_err;
    }

    /* Release memory for every node before releasing main base. */
    free_all_entries(base->head);
    nfree((void **)&base);

	return;

/* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "xnet_destroy_userbase()");
    return;
}

static void free_all_entries(xnet_user_t *user_entry)
{
    if (NULL != user_entry) {
        free_all_entries(user_entry->next);
        nfree((void **)&user_entry->username);
        nfree((void **)&user_entry->password);
        nfree((void **)&user_entry);
    }
}