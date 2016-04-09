#include "server_interface.h"
#include <string.h>
#include <stulibc.h>

char* services[] = { "getBrokerName", "echo", "add", "sayHello",
		"getServerDate", "sayDog", "diffirence", NULL };
char* getServerDate() {
	return "20 jan 2012";
}

char* getBrokerName() {
	return "broker v1";
}

char* echo(char* data) {
	STR_Reverse(data);
	return data;
}

int add(int one, int two) {
	return (one + two);
}

char* sayHello(int age, char* name) {

	return "sayHello()";
}

char* sayDog(char* one, char* two, char* three, char* four) {
	char* first_part = STR_Join(one, two);
	char* second_part = STR_Join(three, four);
	char* final_part = STR_Join(first_part, second_part);

	return final_part;
}

int diffirence(int one, int two) {
	return two - one;
}
