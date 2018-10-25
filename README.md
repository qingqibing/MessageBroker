# MessageBroker Requirement

- Serve as a message router, when received a message, broker transfer it to every other client
- If NO connected client after 3 minutes, then broker kill itself
- Message format is json
- Make use of Overlapped I/O

