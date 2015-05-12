# Abstract Messaging Layer for Storj Platform

## About Abstract Messaging Layer
Massaging Layer in Storj Platform is used for communication between two
parties,  e.g. sending/receiving broadcast messages, negotiating 
service conditions, etc. Messaging layer is abstracted because many 
communication implemenations would be usable in Storj Platform. 
For now there is (telehash-c)[https://github.com/telehash/telehash-c] 
implementation for the underlying layer.

## Messages in  Messaging Layer
Every communications in messaging layer works in event-driven manner. i.e.
you must register handlers for them to act to received messages because
you don't know when messages come in. 
There are two types of message in Messaging layer. One is broadcast, another
is channel message.

### Broadcast
There are two types of message in Messaging layer. One is broadcast. You must
specify broadcast handler when you initiate messaging function. 
First he would call add_broadcaster(receiver) so that receiver would register
him to send broadcasted messages.

When you 
broadcast a message, receiver of this message would analyze this message by 
this handler atf irst. Receiver would check this message, e.g. cheking sender 
didn't send messages too much, or the message is what receiver wants 
to know or not, etc. If receiver thinks it should be broadcasted, the handler
would return string , and he would broadcast this string to registered member.

### Channel Message


