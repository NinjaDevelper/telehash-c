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

from abc import ABCMeta, abstractmethod
import types
import pkgutil

import logging
log_fmt = '%(filename)s:%(lineno)d %(funcName)s() %(message)s'
logging.basicConfig(level=logging.DEBUG, format=log_fmt)


class NoSeqError(Exception):
    """
    Exception Class thrown when no seq is remaiend.
    This instance is called when receving broadcasts and channel packets.
    """
    def __init__(self, value):
        """
        init.
        :param str value: description about this exception
        """
        self.value = value

    def __str__(self):
        """
        return description about this exception when str()
        """
        return repr(self.value)


class ChannelHandler(object):
    """
    Class for handling  received packets in channels.
    This instance is called when receving broadcasts and channel packets.
    """

    def __init__(self):
        """init. store all seq* methods to list."""

        self.count = 0
        self.next = None
        self.mlist = []
        self.n = 0
        self.get_sequences()
        self.call = None

    def get_sequences(self):
        """store all seq* methods to list."""

        for name in dir(self):
            if name.startswith("seq"):
                seq = getattr(self, name)
                if isinstance(seq, types.MethodType):
                    self.mlist.append(seq)

    def _handle(self, packet):
        self.next = None
        r = self.mlist[self.n](packet)
        if self.next is None:
            self.n = self.n + 1
            if self.n >= len(self.mlist):
                self.n = -1
        else:
            self.n = self.mlist.index(self.next)
        return r

    def handle(self, packet):
        """
        call one seq* method incrementally per one packet.
        In seq* methods, you can use
        self.call to call another ClassHandler instance,
        self.next to jump to another seq* methods.

        :param string packet: a received packet to be handled.
        :return: packets to be send back. not sent if None
        """
        if self.n == -1:
            raise NoSeqError("no more methods for the handler.")

        if self.call is not None:
            try:
                r = self.call.handle(packet)
            except NoSeqError:
                self.call = None
                r = self._handle(packet)
        else:
                r = self._handle(packet)
        return r


class Messaging(object):
    """
    Abstract layer of Storj messaging.
    Every class handling Storj messaging must inherit it.
    """
    __metaclass__ = ABCMeta

    DESCRIPTION = ''
    """
    description about this messaging implementation which is
    ued in sublcass.
    """

    def __init__(self, broadcastHandler, **keywords):
        """
   　　　　　init.

       :param ChannelHandler broadcastHandler: a received packet to be handled.
       do nothing to this parameter.
        """
        self.channel_factories = {}

    def add_channel_handler(self, channel_name, factory):
        """
        add ChannelHandler class object and associate it with a channel name.

        :param Class handler_class: ChannelHandler class to be associated
                                    with
        :param method factory: factory method called  when creating
                               handler_class instnace.
        """
        self.channel_factories[channel_name] = factory

    def get_channel_handler(self, channel_name):
        """
        get ChannelHandler instance that is associate it with a channel_name.
        This method is called when receviing channel requests.

        :param str channel_name: channel name
        """
        if channel_name not in self.channel_factories.keys():
            raise TypeError(channel_name + ' is not registered')
        h = self.channel_factories[channel_name]()
        if not isinstance(h, ChannelHandler):
            raise TypeError(
                "create non ChannelHandler instance when call factory")

        return h.handle

    @abstractmethod
    def open_channel(self, location, name, handler):
        """
        open a channel with a handler.
        This method must be overwritten.

        :param str location: json str where you want to open a channel.
        :param str name: channel name that you want to open .
        :param ChannelHandler handler: channel handler.
        """
        pass

    @abstractmethod
    def add_broadcaster(self, location, add):
        """
        send a broadcast request to broadcaster.
        After calling this method, broadcast messages will be send continually.
        This method must be overwritten.

        :param str location: json str where you want to request a broadcast.
        :param  int add: if 0, request to not to broadcast. request to
                          broaadcast if others.
        """
        pass

    @abstractmethod
    def broadcast(self, location, message):
        """
        broadcast a message. This method must be overwritten.

        :param str location: json str where you want to send a broadcast.
        :param str message: broadcast message.
        """
        pass

    @abstractmethod
    def get_my_location(self):
        """
        get json str that represents my location.
        str depends on the underlaying layer.
        This method must be overwritten.

        :return: location str. format depends on underlaying layer.
                 e.g. telehash, etc.
        """
        pass

    @abstractmethod
    def get_my_id(self):
        """
        return my id information. format is:
        jlde3uibwflz4hqnk4zehvj5o5kd4goyqtrwqwhiotw6n4qtrf2a

        :param Object cobj: pointer of StorjTelehash instnace
                            returned by init()
        :return: id info.
        """
        pass
