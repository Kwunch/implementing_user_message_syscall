#include <stdio.h>
#include <sys/syscall.h>
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
	//Send args -> osmsg[0] -s[1] to[2] msg[3]
	//Get args -> osmsg[0] -r[1]

	//Get current user
	char toUser[100];
	char msg[256];
	char from[100];

	struct passwd *pwd; //Used to check if recipient for -s exists on machine
	
	if (!(argc == 4 || argc == 2)) {
		printf("%d\n", argc);
		printf("Invalid Arg Length\n");
		return -1;
	}
	if (strcmp(argv[1], "-s") == 0) { // Check if argument is send arg

		strcpy(toUser, argv[2]); // Copy recipient into toUser

		pwd = getpwnam(toUser); // Check if recipient exists on local machine

		if (pwd == NULL) {
			// User does not exist. Exit out of program
			printf("User %s not found\n", toUser);
			return -1;
		}

		strcpy(msg, argv[3]); // Copy message into msg variable
		strcpy(from, getenv("USER")); // Get username of user sending message
		
		if (cs1550_send_msg(toUser, msg, from) == -2) {
			/*
				* Message failed to send.
				* Probably issue in 'kernel/sys.c'
			*/
			printf("Message Failed To Send\n");
			return -1;
		}

		printf("Message Sent\n");

	} else if (strcmp(argv[1], "-r") == 0) { // Check if arg is read msg arg
		// Copy current user into toUser
		// Check if messages send to 'toUser' exist
		strcpy(toUser, getenv("USER"));
		while (cs1550_get_msg(toUser) == 1) {/* While there are more messages, keep calling  */}
	} else {
		printf("Invalid Arg\n"); // Invalid arg passed
	}
	return 0;
}

int cs1550_send_msg(const char *to, const char *msg, const char *from) {
	/*
		* Call syscall 441 (sys_cs1550_send_msg)
		* Send message to the user in 'to'
	*/
	if (syscall(441, to, msg, from) == 0) {
		/*
			* Message send successfully
		*/
		return 0;
	}
	return -2;
}

int cs1550_get_msg(const char *to) {
	char *from = malloc(100 * sizeof(char)); // Allocate 100 bytes to from var [100 char max]
	char *msg = malloc(256 * sizeof(char)); // Allocate 256 bytes to msg var [256 char max]

	int val = syscall(442, to, msg, from); // Syscall 442 [sys_cs1550_get_msg] store return val
	
	if (val == -1) {
		// -1 returned inbox is empty
		printf("No new messages at this time\n");
		return -1;
	}
	
	/*
		* Inbox has at least 1 message.
		* Message and Sender copied to msg and from variables
		* Print the values of both
	*/
	printf("From %s - %s\n", from, msg);
	
	if (val == 0) { // Message was only message in inbox, return 0 so program can finish
		return 0;
	} else if (val == 1) { // More messages in inbox, return 1 for function recall
		return 1;
	}
	
	return -1; // Somehow
}
