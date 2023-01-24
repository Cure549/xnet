# **XNET**

A multi-purpose, multi-threaded, epoll server framework that uses an addon approach to expand it's modularity.

# **Table Of Contents**
[Overview](#overview)

[How-To-Use](#how-to-use)

[The Loading Process](#the-loading-process)

[Server Categories](#server-categories)

[The Addon System](#the-addon-system)

[Creating An Addon](#creating-an-addon)

# **OVERVIEW**

Although XNet was made to **make server development easier**. It's still crucial to understand the underlying structure. XNet is a server framework and does have support for **out-of-the-box** use cases. However, to squeeze all of the possibilities out of XNet, this documentation will explain everything such as the event structuring, session handling, addon systems, threading, packet subscription, etc. This is your guide to all things XNet.

# **HOW-TO-USE**

### Sample
```c
#include  "xnet_base.h" // XNet's core library.

int main(void)
{
	const char *ip   = "127.0.0.1";
	size_t port      = 47007;
	size_t backlog   = 5;
	size_t timeout   = 300;
	xnet_box_t *xnet = xnet_create(ip, port, backlog, timeout);
	xnet_start(xnet);
	xnet_destroy(xnet);
}
```

### Result
```
user@my_computer:~$ ./a.out
[XNet]
IP: 127.0.0.1
Port: 47007
```
> 📝 **Note:** If there are any issues in regards to getting this output when attempting to execute the above code. Ensure you have no linker errors, and that the header file is being properly recognized.

### Explanation
By including the **header file**, you will be exposed to three very important functions.
```c
xnet_create();
xnet_start();
xnet_destroy();
```
These functions consist of the **bare minimum** when it comes to a running an instance of XNet. A more comprehensive explanation of what these functions do will be explained further down. The fundamental aspect to understand here is that XNet has a **very specific loading process**. See below.

# **THE LOADING PROCESS**

XNet consists of a five-part loading process. Understanding this will make understanding everything else much easier.

## 1. Create Server
Creating the server consists of calling the following function signature.
```c
xnet_box_t *xnet_create(const char *ip, size_t port, size_t backlog, size_t timeout);
```
This accomplishes a few things. Ensures the values given are valid and assigns them to the server. As well as it initializes necessary memory for all the internal server categories.
> 📝 **Note:** These internal server categories consist of General, Network, Thread, Connections, and Userbase. They are all covered more deeply, further in the guide.

There are two static functions responsible for the majority of this heavy lifting.
```c
// Attempts to initialize memory for internal server categories.
static xnet_box_t *initialize_xnet_box(void);
```
```c
// Primarily handles socket configuration
static int xnet_configure(xnet_box_t *xnet);
```

## 2. Configure Server
This step is where the end-user will be doing **essentially all of the configuration for the server**. If you want to introduce any custom addons, and/or blacklist features from said addons. It should occur at this point. You can also manipulate the standard event calls.
> 📝 **Note:** There are four standard event calls. OnConnectionAttempt, OnTerminateSignal, OnClientSend, and OnSessionExpire. These are fairly self explanatory, however a deeper explanation will be made regarding these events further in the guide.

## 3. Start Server



## 4. Handle  Events



## 5. Shutdown and Cleanup


# **SERVER CATEGORIES**

When using XNet, the end-user has access to a lot of information about the server. It's split up into five main categories that allow the end-user to have readable, quick access to various data.

## General

The general category consists of data that is primarily behavioral focused.
```c
bool is_running;
const char *ip;
size_t port;
size_t backlog;
size_t connection_timeout;
size_t max_connections;
void (*on_connection_attempt)(xnet_box_t *xnet);
void (*on_terminate_signal)(xnet_box_t *xnet);
void (*on_client_send)(xnet_box_t *xnet, xnet_active_connection_t *me);
// It's not suggested to manually change 'perform', unless you know what you are doing.
int  (*perform[XNET_MAX_FEATURES])(xnet_box_t *xnet, xnet_active_connection_t *client);
```

Examples
```c
xnet->general->is_running = false;
xnet->general->on_client_send = my_custom_func;
```

## Network

## Connections

## Userbase

## Thread

# **THE ADDON SYSTEM**
One of the key aspects of XNet is that it incorporates an addon system. A way to **compartmentalize** the server's core functionality, from the additive and unique functionality that is requested from a client. This not only allows there to be minimal reliance in terms of code, but allows the end-user to mentally abstract the two compartments.

Addons are able to communicate with everything related to the server's core, however it's a **one-way relationship**. This is to ensure there is not accidental reliance embedded into the server. An addon can access data from the server, but the server can not access data from an addon. If you run into a situation where this poses an issue, reconsider your design decisions.

When creating an addon, it is up to the creator if a reliance should be established upon other addons. A forceful way to ensure this reliance is not a part of XNet at this time.

## Limitations
 - XNet limits the **total number of features** to 4096. There is no limitations on number of addons.
- Feature opcodes 0-50 are reserved for core server functionality.


# **CREATING AN ADDON**
Creating addons for XNet is where things really start to get fun. However, there's a standard that should be known before releasing an addon into the wild. This is to ensure as much consistency as possible when using or building addons for XNet.

## Folder Structure Example
This exact structure is not required. This is more-so an example of a structure that an addon **should** work in.
```
📦my_server
 ┣ 📂build
 ┣ 📂include
 ┃ ┣ 📜xnet_addon_my_custom_addon.h   <== Header file
 ┃ ┗ 📜all_other_header_files.h
 ┣ 📂src
 ┃ ┣ 📜gerr.c
 ┃ ┣ 📜server.c
 ┃ ┣ 📜xnet_addon_chat.c
 ┃ ┣ 📜xnet_addon_core.c
 ┃ ┣ 📜xnet_addon_ftp.c
 ┃ ┣ 📜xnet_addon_my_custom_addon.c   <== Source file
 ┃ ┣ 📜xnet_base.c
 ┃ ┗ 📜xnet_utils.c
 ┣ 📜.gitignore
 ┗ 📜Makefile
```
> **Note:** Notice the naming convention for addons. 'xnet_addon_xyz'.

## Header File Example
The header file should show packet subscriptions, what opcodes are used, and function signatures. Here's an example:
```c
/**
* @file xnet_addon_my_custom_addon.h
* @author RandomPerson1
* @brief My custom addon for a XNet server that does stuff.
* @version 1.0
* @date 2023-01-11
*
* @copyright Copyright (c) 2022 RandomPerson1
* License MIT
*/

/* [XNET 135-155] */   <== This is an easy way to show other users what opcodes are reserved.

#ifndef XNET_ADDON_MY_CUSTOM_ADDON_H
#define XNET_ADDON_MY_CUSTOM_ADDON_H

#include  "xnet_base.h"
#include  "xnet_utils.h"

struct __attribute__((__packed__)) my_custom_addon_action1_tc {
	int length;
	char msg[2048];
};

struct __attribute__((__packed__)) my_custom_addon_action1_fc {
	int length;
	char msg[2048];
};

typedef struct __attribute__((__packed__)) my_custom_addon_action1_root {
	struct my_custom_addon_action1_tc to_client;
	struct my_custom_addon_action1_fc from_client;
} my_custom_addon_action1_t ;

int xnet_integrate_my_custom_addon(xnet_box_t *xnet);

// Echo's what the client sent.
int my_custom_addon_perform_action1(xnet_box_t *xnet, xnet_active_connection_t *client);

#endif // RandomPerson1
```
There are a few standards to know in regards to this header file:

 - Every addon should display what opcodes are in use at the top of their respective header file.
 - Each feature should have a ```to_client``` and ```from_client``` struct. Both suffixed with 'tc', and 'fc' respectively. They should then be packed into a root struct that contains both.
 - The naming convention for said structs are ```<AddonName>_<FeatureName>_<tc/fc>```.
 - Your addon will need a integrate function that informs the server of it's existence. The naming convention is ```xnet_integrate_<AddonName>```.
 - Every feature will need a function to call when it's opcode is identified. This function signature must conform to the signature below:
```c
int <AddonName>_perform_<FeatureName>(xnet_box_t *xnet, xnet_active_connection_t *client);
```

## Source File Example
The source file should be fairly straightforward at this point. As all there is left to do is to complete implementation.
```c
#include "xnet_addon_chat.h"

int xnet_integrate_my_custom_addon(xnet_box_t *xnet)
{
	xnet_insert_feature(xnet, 135, my_custom_addon_perform_action1);
	return 0;
}

int my_custom_addon_perform_action1(xnet_box_t *xnet, xnet_active_connection_t *client)
{
	(void)xnet; // Hide unused argument warnings.
	puts("action1 was triggered!");
	my_custom_addon_action1_t packet = {0};
	read(client->client_event.data.fd, &packet.from_client, sizeof(packet.from_client));
	printf("%d (%s)\n", ntohl(packet.from_client.length), packet.from_client.msg);
	return 0;
}
```
> **Note:** Notice how we use `xnet_insert_feature()` to inform XNet of the association between opcode and function.

## Main Server File
Now that the addon is completed. We need to inform XNet to integrate it. By remembering the load process, it should be fairly straightforward to know at what point the integrate call should be made.
```c
#include "xnet_base.h"
#include "xnet_addon_my_custom_addon.h"

int main(void)
{
	const char *ip   = "127.0.0.1";
	size_t port      = 47007;
	size_t backlog   = 5;
	size_t timeout   = 300;
	xnet_box_t *xnet = xnet_create(ip, port, backlog, timeout);
	xnet_integrate_my_custom_addon(xnet);   ⇐ Integrate the addon before starting server.
	xet_start(xnet);
	xnet_destroy(xnet);
}
```
# **FUNCTION AND TYPES OVERVIEW**
| Types                          | Description     |
|--------------------------------|-----------------|
| ```xnet_box_t```               | The most important data type in XNet. Controls everthing in regards to server behaviour. |
| ```xnet_user_t```              | Hold's account data such as username, password, permissions, etc. This type is solely responsible for handling accounts.            |
| ```xnet_user_session_t```      | Each user upon connection must be given a timed session. This type does just that.            |
| ```xnet_active_connection_t``` | Each time a client successfully connects. They are assigned one of these. This type is what XNet uses to represent a client's connection. |
| ```xnet_general_group_t```     |  |
| ```xnet_network_group_t```     |  |
| ```xnet_thread_group_t```      |  |
| ```xnet_connection_group_t```  |  |
| ```xnet_userbase_group_t```    |  |