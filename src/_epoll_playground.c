// #define MAX_EVENTS 5
// #define READ_SIZE 10
// #include <stdio.h>
// #include <unistd.h>
// #include <sys/epoll.h>
// #include <string.h>
// #include <stdlib.h>
// #include <time.h>
// #include <stdbool.h>
// #include <pthread.h>


// struct server {
// 	bool is_running;
// };

// int epoll_playground(void);

// void *testy(void *data)
// {
// 	// for (size_t n=0; n<10; n++) {
// 	// 	printf("%d\n", n);
// 	// 	sleep(1);
// 	// }
// 	return NULL;
// }

// int epoll_playground(void)
// {
// 	struct server xnet;
// 	xnet.is_running = true;

//     int running = 1;
// 	int event_count;
// 	size_t bytes_read;
// 	char read_buffer[READ_SIZE + 1];
// 	struct epoll_event event = {0};
// 	struct epoll_event events[MAX_EVENTS] = {0};

// 	int epoll_fd = epoll_create1(0);
// 	if (-1 == epoll_fd) {
// 		fprintf(stderr, "Failed to create epoll file descriptor\n");
// 		return 1;
// 	}

// 	event.events = EPOLLIN;

// 	if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, 0, &event))
// 	{
// 		fprintf(stderr, "Failed to add file descriptor to epoll\n");
// 		close(epoll_fd);
// 		return 1;
// 	}

// 	while (xnet.is_running) {
// 		printf("\nPolling for input...\n");
// 		event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, 10000);
// 		printf("%d ready events\n", event_count);

// 		for (int i = 0; i < event_count; i++) {
// 			printf("Reading file descriptor '%d' -- ", events[i].data.fd);

// 			bytes_read = read(events[i].data.fd, read_buffer, READ_SIZE);
// 			printf("%zd bytes read.\n", bytes_read);

// 			read_buffer[bytes_read] = '\0';
// 			printf("Read '%s'\n", read_buffer);

// 			if (0 == strncmp(read_buffer, "yay\n", 4)) {
// 				pthread_t thread_id;
// 				pthread_create(&thread_id, NULL, testy, NULL);
// 			}

// 			if (0 == strncmp(read_buffer, "close\n", 6)) {
// 				xnet.is_running = false;
// 			}
// 		}
// 	}

// 	if (close(epoll_fd)) {
// 		fprintf(stderr, "Failed to close epoll file descriptor\n");
// 		return 1;
// 	}

// 	return 0;
// }
typedef int make_iso_compilers_happy;