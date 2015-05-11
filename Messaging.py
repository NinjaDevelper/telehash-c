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

import logging
log_fmt = '%(filename)s:%(lineno)d %(funcName)s() %(message)s'
logging.basicConfig(level=logging.DEBUG, format=log_fmt)


class ChannelHandler(object):

    def __init__(self):
        self.count = 0
        self.next = None
        self.mlist = []
        self.n = 0
        self.getSequences()

    def getSequences(self):
        for name in dir(self):
            if name.startswith("seq"):
                seq = getattr(self, name)
                if isinstance(seq, types.MethodType):
                    self.mlist.append(seq)

    def handle(self, packet):
        if self.n == -1:
            return None

        self.next = None
        r = self.mlist[self.n](packet)
        if self.next == None:
            self.n = self.n + 1
            if self.n >= len(self.mlist):
                self.n = -1
        else:
            self.n = self.mlist.index(self.next)
        return r


class Messaging(object):
    __metaclass__ = ABCMeta

    def __init__(self, broadcastHandler):
        self.channels = {}

    def addChannelHandler(self, channelName, handlerClass):
        if issubclass(handlerClass, ChannelHandler):
            self.channels[channelName] = handlerClass
        else:
            logging.error("cannot add non ChannelHandler subclass")

    def getChannelHandler(self, channelName):
        if channelName not in self.channels.keys():
            logging.info(channelName + ' not found')
            return None

        return self.channels[channelName]().handle

    @abstractmethod
    def openChannel(self, location, name, handler):
        pass

    @abstractmethod
    def addBroadcaster(self, location, add):
        pass

    @abstractmethod
    def broadcast(serl, location, message):
        pass

    @abstractmethod
    def getMyLocation(serl):
        pass
