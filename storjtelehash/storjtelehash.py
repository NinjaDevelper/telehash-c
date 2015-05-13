# -*- coding: utf-8 -*-

# Copyright (c) 2015, Shinya Yagyu
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

from storj.messaging import Messaging
from storj.messaging import ChannelHandler
from .  import telehashbinder
# import telehashbinder #for creating document
import threading
import logging
log_fmt = '%(filename)s:%(lineno)d %(funcName)s() %(message)s'
logging.basicConfig(level=logging.DEBUG, format=log_fmt)


class StorjTelehash(Messaging):
    """
    Concrete Messaging layer for Storj Platform in Telehash.
    Everything in telehash-C is not thread safe. So run function after
    stop a thread, and run a thread again in all functions.
    """
    def __init__(self, broadcast_handler, port):
        """
        init

        :param ChannelHandler broadcast_handler: broadcast handler.
        :param int port: port number to be listened packets. if 0, port number
        is seletcted randomly.
        """
        Messaging.__init__(self, broadcast_handler)
        self.cobj = telehashbinder.init(port, self.get_channel_handler,
                                        broadcast_handler)
        self.start_thread()

    def get_my_location(self):
        """
        return my location information. format is:
         {"keys":{"1a":"al45izsjxe2sikv7mc6jpnwywybbkqvsou"},
        paths":[{"type":"udp4","ip":"127.0.0.1","port":1234}]

         :return: location info.
        """
        return telehashbinder.get_my_location(self.cobj)

    def start_thread(self):
        """star to receive netowrk packets in a thread. """

        telehashbinder.set_stopflag(self.cobj, 0)
        self.thread = threading.Thread(
            target=lambda: telehashbinder.start(self.cobj))
        self.thread.setDaemon(True)
        self.thread.start()

    def open_channel(self, location, name, handler):
        """
        open a channel with a handler.

        :param str location: json str where you want to open a channel.
        :param str name: channel name that you want to open .
        :param ChannelHandler handler: channel handler.
        """
        if isinstance(handler, ChannelHandler):
            telehashbinder.set_stopflag(self.cobj, 1)
            self.thread.join()
            telehashbinder.open_channel(self.cobj, location, name,
                                        handler.handle)
            self.start_thread()
        else:
            logging.error("cannot add non ChannelHandler instance")

    def add_broadcaster(self, location, add):
        """
        send a broadcast request to broadcaster.
        After calling this method, broadcast messages will be send continually.

        :param str location: json str where you want to request a broadcast.
        :param  int add: if 0, request to not to  broadcast. request to
                          broaadcast if others.
        """
        telehashbinder.set_stopflag(self.cobj, 1)
        self.thread.join()
        telehashbinder.add_broadcaster(self.cobj, location, add)
        self.start_thread()

    def broadcast(self, location, message):
        """
        broadcast a message.

        :param str location: json str where you want to send a broadcast.
        :param str message: broadcast message.
        """
        telehashbinder.set_stopflag(self.cobj, 1)
        self.thread.join()
        telehashbinder.broadcast(self.cobj, location, message)
        self.start_thread()

    def __del__(self):
        """
         destructor. stop a thread and call telehashbinder's finalization.
        """

        telehashbinder.set_stopflag(self.cobj, 1)
        self.thread.join()
        telehashbinder.finalize(self.cobj)
