#include "gerr.h"

static const char *const _gerr_desc[] = {
    [E_GEN_POSITIVE_NUM]    = "Unexpected positive value",
    [E_GEN_NEGATIVE_NUM]    = "Unexpected negative value",
    [E_GEN_OUT_RANGE]       = "Value is out of range",
    [E_GEN_NULL_PTR]        = "NULL Pointer detected",
    [E_GEN_FAIL_ALLOC]      = "Heap allocation failure",
    [E_GEN_FAIL_STRNDUP]    = "Strndup() failure",
    [E_GEN_FAIL_STR_LENGTH] = "Invalid string length",
    [E_GEN_NON_ZERO]        = "Expected Non-Zero value",
    [E_GEN_SINGLE_CHAR]     = "Must contain only one character",
    [E_GEN_FAIL_MEMSET]     = "Failure found during memset()",

    [E_SRV_BAD_ADDR_INFO] = "Failed to obtain address information",
    [E_SRV_FAIL_SOCK]     = "Failed to create socket",
    [E_SRV_FAIL_SOCK_OPT] = "Failed to set socket option",
    [E_SRV_FAIL_BIND]     = "Failed to bind",
    [E_SRV_FAIL_LISTEN]   = "Failed to listen on socket",
    [E_SRV_FAIL_ACCEPT]   = "Failed to accept connection",
    [E_SRV_REACHED_CLT_LIMIT] = "Failed client request due to max client limit reached",
    [E_SRV_INVALID_PORT] = "Failed port assignment due to it not falling within the set valid range",
    [E_SRV_FAIL_CREATE] = "Failed to create server during the server creation process",
    [E_SRV_INVALID_IP] = "Failed to assign ip",
    [E_SRV_INVALID_BACKLOG] = "Failed to assign backlog",
    [E_SRV_INVALID_TIMEOUT] = "Failed to assign timeout",
};

static const char *
_get_category(int err_val)
{
    if (2500 >= err_val) {
        return "GENERAL";
    }

    if (3500 >= err_val) {
        return "SERVER";
    }

    return "UNKNOWN";
}

void
g_show_err(int err_val, const char *msg)
{
    /* NULL check */
    if (NULL == msg)
    {
        return;
    }

    const char *title    = "GERROR";
    const char *category = _get_category(err_val);

    // If an additional message was passed in, print the message along with
    // error statement.
    if (0 == strnlen(msg, MAX_ERR_MSG_LENGTH))
    {
        fprintf(
            stderr, "[%s] <%s> : %s\n", title, category, _gerr_desc[err_val]);
    }
    else
    {
        fprintf(stderr,
                "[%s] <%s> : %s (%s)\n",
                title,
                category,
                _gerr_desc[err_val],
                msg);
    }
}