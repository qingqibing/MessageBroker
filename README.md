# MessageBroker Requirement

- Serve as a message router, when received a message, broker transfer it to every other client
- If NO connected client after 3 minutes, then broker kill itself
- Message format is json
- Make use of Overlapped I/O

### Branch 

master branch use APC and Event kernel object to get aysnc I/O notification, while IOCP branch use I/O completion port .

