For the term project we completed all components specified except generating a livestock/starvation case.

For running a.2 concurrent ATM processes, b.5 concurrent ATM processes, c.Interest Calculator, d.Generating a
deadlock case, and the bonus functionality (Money Transfer) please refer to Test_file.txt.

The two reports for the deadlock and the bonus functionality are included in the submission.

FUNCTIONALITY OF EACH PROCESS:

ATM.C:
The ATM asks as the interface between the user and the system and takes input from the user to complete various
operations.

DB Server (main.c):
The DB server does the main functionality of the system and performs the requested operation from the ATM user and
uses it to update the DB.

DB_editor.c:
The DB editor works with the DB server by sending it an account to be created by taking input from the user and
the once sent to the DB server the account gets added to the DB for use in the system.

Interest_Calculator.c:
The interest calculator updates the database by adding 1% funds to a user if they have positive funds and decrementing
2% if the user has negative funds. It does this every 60s, we achieved this by sleeping the process for 60 before
every time we run the functionality.



