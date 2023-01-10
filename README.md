# XNET

A multi-purpose, multi-threaded, epoll server framework that uses an addon approach to expand it's modularity.


# OVERVIEW

Although XNet was made to **make server development easier**. It's still crucial to understand the underlying structure. XNet is a server framework and does have support for **out-of-the-box** use cases. However, to squeeze all of the possibilities out of XNet, this documentation will explain everything such as the event structuring, session handling, addon systems, threading, packet subscription, etc. This is your guide to all things XNet.

# HOW-TO-USE

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
> **Note:** If there are any issues in regards to getting this output when attempting to execute the above code. Ensure you have no linker errors, and that the header file is being properly recognized.

### Explanation
By including the **header file**, you will be exposed to three very important functions.
```c
xnet_create();
xnet_start();
xnet_destroy();
```
These functions consist of the **bare minimum** when it comes to a running an instance of XNet. A more comprehensive explanation of what these functions do will be explained further down. The fundamental aspect to understand here is that XNet has a **very specific loading process**. See below.

# THE LOADING PROCESS

XNet consists of a five-part loading process. Understanding this will make understanding everything else much easier.

## 1. Create Server
Creating the server consists of calling the following function signature.
```c
xnet_box_t *xnet_create(const char *ip, size_t port, size_t backlog, size_t timeout);
```
This accomplishes a few things. Ensures the values given are valid and assigns them to the server. As well as it initializes necessary memory for all the internal server categories.
> **Note:** These internal server categories consist of General, Network, Thread, Connections, and Userbase. They are all covered more deeply, further in the guide.

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
> **Note:** There are four standard event calls. OnConnectionAttempt, OnTerminateSignal, OnClientSend, and OnSessionExpire. These are fairly self explanatory, however a deeper explanation will be made regarding these events further in the guide.

## 3. Start Server



## 4. Handle  Events



## 5. Shutdown and Cleanup


# SERVER CATEGORIES

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
int (*perform[XNET_MAX_FEATURES])(xnet_box_t *xnet, xnet_active_connection_t *client);

// Examples
xnet->general->is_running = false;
xnet->general->on_client_send = my_custom_func(xnet, client);
```

## Network

## Connections

## Userbase

## Thread

# THE ADDON SYSTEM
One of the key aspects of XNet is that it incorporates an addon system. A way to **compartmentalize** the server's core functionality, from the additive and unique functionality that is requested from a client. This not only allows there to be minimal reliance in terms of code, but allows the end-user to mentally abstract the two compartments.

Addons are able to communicate with everything related to the server's core, however it's a **one-way relationship**. This is to ensure there is not accidental reliance coded into the server. An addon can access data from the server, but the server can not access data from an addon. If you run into a situation where this poses an issue, reconsider your design decisions.

When creating an addon, it is up to the creator if a reliance should be established upon other addons. A forceful way to ensure this reliance is not a part of XNet at this time.

## Limitations

 - XNet limits the **total number of features** to 4096. There is no limitations on number of addons.
- Opcode conflicts will be common, ensure you reserve as small of a block is necessary for your addon. 



how to incorporate an addon into xnet
limitations placed onto addons
blacklisting features of an addon
Creating addons/examples
explain chat/ftp/core addon
the core addon is primarily for basic server to client capabilities(tell client it successfully connected, etc)
explain packet subscription