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

import Messaging
import TelehashBinder
import threading

import logging
log_fmt = '%(filename)s:%(lineno)d %(funcName)s() %(message)s'
logging.basicConfig(level=logging.DEBUG, format=log_fmt)

class MessagingTelehash(Messaging.Messaging):

    def __init__(self, broadcastHandler, port):
        Messaging.Messaging.__init__(self, broadcastHandler)
        self.cobj = TelehashBinder.init(port, self.getChannelHandler,
                                        broadcastHandler)
        self.startThread()

    def getMyLocation(self):
        return TelehashBinder.getMyLocation(self.cobj)

    def startThread(self):
        TelehashBinder.setStopFlag(self.cobj, 0)
        self.thread = threading.Thread(
            target=lambda: TelehashBinder.start(self.cobj))
        self.thread.setDaemon(True)
        self.thread.start()

    def openChannel(self, location, name, handler):
        if isinstance(handler, Messaging.ChannelHandler):
            TelehashBinder.setStopFlag(self.cobj, 1)
            self.thread.join()
            TelehashBinder.openChannel(self.cobj, location, name,
                                       handler.handle)
            self.startThread()
        else:
            logging.error("cannot add non ChannelHandler instance")

    def addBroadcaster(self, location, add):
        TelehashBinder.setStopFlag(self.cobj, 1)
        self.thread.join()
        TelehashBinder.addBroadcaster(self.cobj, location, add)
        self.startThread()

    def broadcast(self, location, message):
        TelehashBinder.setStopFlag(self.cobj, 1)
        self.thread.join()
        TelehashBinder.broadcast(self.cobj, location, message)
        self.startThread()

    def __del__(self):
        TelehashBinder.setStopFlag(self.cobj, 1)
        self.thread.join()
        TelehashBinder.finalize(self.cobj)
