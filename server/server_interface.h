
// Defines the operations that the client can call and that the server knows how to service.
// This is used by both the client and server.

#ifndef SERVER_INTERFACE_H
#define SERVER_INTERFACE_H

/**
 * @brief Gets the server's date
 * 
 * @return char* the servers date in string format
 */

char* getServerDate();
/**
 * @brief Adds two numbers up
 * 
 * @param one number one
 * @param two number two
 * @return int sum of the two numbers
 */
int add( int one, int two );
/**
 * @brief Returns the same output as the input
 * 
 * @param echo input
 * @return char* output
 */
char* echo(char* message);
/**
 * @brief Gets the name of the broker
 * 
 * @return char* the name of the broker
 */
char* getBrokerName();

char* sayHello(int age, char* name);

char* sayDog(char* one, char* two, char* three, char* four);

#endif
