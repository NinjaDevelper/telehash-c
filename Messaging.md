# Abstract Messaging Layer for Storj Platform

## About Abstract Messaging Layer
Massaging Layer in Storj Platform is used for communication between two
parties,  e.g. sending/receiving broadcast messages, negotiating 
service conditions, etc. Messaging layer is abstracted because many 
communication implemenations would be usable in Storj Platform. 
For now there is [telehash-c](https://github.com/telehash/telehash-c)
implementation for the underlying layer.

All messages are formated in JSON.

## Messages in  Messaging Layer
Every communications in messaging layer works in event-driven manner. i.e.
recievers must register handlers for them to act to received messages because
they don't know when messages come in. 
So there is no functions to send() and recv() directly.

There are two types of message in Messaging layer. One is broadcast, another
is channel message.

### Broadcast
Everyone who use messaging layer must
specify a broadcast handler when initialing messaging class. 
First he would call ```add_broadcaster(receiver)``` function so that the ```receiver```
would register him to send messages that someone broadcasted.

When you 
broadcast a message, receiver of this message would analyze this message by 
the handler at first. Receiver would check this message, e.g. that sender 
didn't send messages too much, or  that this message is what receiver wants 
to know, etc. If the receiver thinks it should be broadcasted, the handler
would return a string , and messaging layer would broadcast this string to registered member.

### Channel Message

Channel message is used for general purpose. 
Recevier must register a handler as a channel handler with a channel name
if he want to act to a messages on the channel. Channel name is like a
_port number_ in TCP/IP, where receiver listen. At first the sender 
register a handler for responses and send a message 
with a channel name to a receiver. 
After sending messages, a reciever receives and read the message 
by a registered handler. When he wants to stop messaging, he would send
nothing.

### Handler for messages
Handler must implements ```Messaing.ChannelHander``` class. All methods that
starts with seq* would be run one by one when receiving messages.
The order of calling method is alphabeetic order.

## Example

The sample Python program  is below.

```
from storj.messaging import ChannelHandler
from storjtelehash.storjtelehash import StorjTelehash

class ChannelOpener(ChannelHandler):
    """
    Channel handler for channel opener.
    Methods below would be called in alphabetic order.
    """
    def seqA(self, packet):
        """"
        packet: recevied message 
        logging.debug('called by opener at 1st, i.e. when opening a  channel')
        return '{\"count\":0}'

    def seqB(self, packet):
        logging.debug('called by opener at 2nd')
        # packet would be '{\"count\":1}'
        #send back nothing.
        return None

    def seqC(self, packet):
    """
    never called
    """
        counter_opener = 9999
        return packet


class ChannelReceiver(ChannelHandler):
    """
    Channel handler for channel receiver.
    Methods below would be called in alphabetic order.
    """

    def seqA(self, packet):
        logging.debug('called by receiver at 1st')
        # packet would be '{\"count\":0}'
        return '{\"count\":1}'

    def seqB(self, packet):
        """
        never called
        """
        return packet

def broadcast_handler(self, packet):
    """
    handler called when receiving broadcasts.
    packet: received message
    """
    #do something 
    return packet

# initiate telehash as port=1234 and register broadcast handler
s = StorjTelehash(broadcast_handler, 1234)
# request locationA to broadcast to me.
s.add_broadcaster(locationA, 1)


# broadcast a messagt to locationC
s.broadcast(locationC, '{"service":"farming0"}')

# register channel handler with 'one_service' channel.
#register a Class, not an instance.
s.add_channel_handler('one_service', ChannelReceiver)

# open a channel with name 'another_service' with ChannelOpener handler.
#register an instnace, not a Class !
s.open_channel(locationB, 'another_service', ChannelOpener())
```
