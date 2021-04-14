We handed in Part A and Part B in seperate files.

For Part we we got B1) operational and were able to have proper communication between
the DB_editor and the DB_server (main.c).

For B2) we were unable to complete full communication between the ATM and the full system but are
able to run the three processes concurrently and have minimal execution between them.

We replaced the message types with integers, as defined below:
	PIN = 1
	BALANCE = 2
	WITHDRAW = 3
	UPDATE_DB = 4
	PIN_WRONG = 5
	OK = 6
