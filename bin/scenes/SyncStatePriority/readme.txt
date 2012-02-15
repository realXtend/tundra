This is a example script how to manipulate the 'how and when' a clients scene sync state gets filled.
It's done on the server and start when a client connects we have to decide what entities we release immediately
and what do we keep as pending. Once you add a entity to the pending list it will never go automatically to the
clients sync state, you have to explicitly add it there when your logic determines it is time.
There are multiple ways of accomplishing prioritization and it depends largely on the scene you are running on the server.
This scripts aim is to show a few generic ways how to do it.

Please also read SyncStatePriority.js comments on the top and the code itself.
