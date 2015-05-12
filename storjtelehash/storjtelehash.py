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
from storjtelehash import telehashbinder
import threading

import logging
log_fmt = '%(filename)s:%(lineno)d %(funcName)s() %(message)s'
logging.basicConfig(level=logging.DEBUG, format=log_fmt)

class StorjTelehash(Messaging):

    def __init__(self, broadcast_handler, port):
        Messaging.__init__(self, broadcast_handler)
        self.cobj = telehashbinder.init(port, self.get_channel_handler,
                                        broadcast_handler)
        self.start_thread()

    def get_my_location(self):
        return telehashbinder.get_my_location(self.cobj)

    def start_thread(self):
        telehashbinder.set_stopflag(self.cobj, 0)
        self.thread = threading.Thread(
            target=lambda: telehashbinder.start(self.cobj))
        self.thread.setDaemon(True)
        self.thread.start()

    def open_channel(self, location, name, handler):
        if isinstance(handler, ChannelHandler):
            telehashbinder.set_stopflag(self.cobj, 1)
            self.thread.join()
            telehashbinder.open_channel(self.cobj, location, name,
                                       handler.handle)
            self.start_thread()
        else:
            logging.error("cannot add non ChannelHandler instance")

    def add_broadcaster(self, location, add):
        telehashbinder.set_stopflag(self.cobj, 1)
        self.thread.join()
        telehashbinder.add_broadcaster(self.cobj, location, add)
        self.start_thread()

    def broadcast(self, location, message):
        telehashbinder.set_stopflag(self.cobj, 1)
        self.thread.join()
        telehashbinder.broadcast(self.cobj, location, message)
        self.start_thread()

    def __del__(self):
        telehashbinder.set_stopflag(self.cobj, 1)
        self.thread.join()
        telehashbinder.finalize(self.cobj)
