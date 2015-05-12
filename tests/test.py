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
from storjtelehash.storjtelehash import StorjTelehash
from storjtelehash import telehashbinder
import json
import pytest
import time

import logging
log_fmt = '%(filename)s:%(lineno)d %(funcName)s() %(message)s'
logging.basicConfig(level=logging.DEBUG, format=log_fmt)

counter_opener = 0
counter_receiver = 0


class ChannelOpener(ChannelHandler):

    def seqA(self, packet):
        logging.debug('called openerA')
        global counter_opener
        counter_opener = counter_opener + 1
        return '{\"count\":0}'

    def seqB(self, packet):
        logging.debug('called openerB')
        global counter_opener
        counter_opener = counter_opener + 1
        j = json.loads(packet)
        assert j['count'] == 1
        return None

    def seqC(self, packet):
        counter_opener = 9999
        return packet

class ChannelReceiver(ChannelHandler):

    def seqA(self, packet):
        logging.debug('called receiverA')
        global counter_receiver
        counter_receiver = counter_receiver + 1
        j = json.loads(packet)
        assert j['count'] == 0
        return '{\"count\":1}'

    def seqB(self, packet):
        logging.debug('called receiverB message='+packet)
        global counter_receiver
        counter_receiver = counter_receiver + 1
        return packet

    def seqC(self, packet):
        counter_receiver = 9999
        return packet

class TestStorjTelehash(object):

    def setup(self):
        telehashbinder.set_gc(0)
        self.m2 = StorjTelehash(self.broadcast_handler, 1234)
        self.m3 = StorjTelehash(self.broadcast_handler, -9999)
        self.m4 = StorjTelehash(self.broadcast_handler, -9999)

        loc = self.m2.get_my_location()
        self.location =  \
            '{"keys":{"1a\":"' + json.loads(loc)['keys']['1a']  + '"},' \
            '"paths\":[{"type":"udp4","ip":"127.0.0.1","port":1234}]}'
        logging.debug("location=" + self.location)

    def broadcast_handler(self, packet):
        j = json.loads(packet)
        assert j['service'] == 'farming%d' % (self.status)
        self.status = self.status + 1
        logging.debug("packjet=" + packet)
        if self.status == 2:
            return None
        j['service'] = "farming%d" % (self.status)
        return json.dumps(j)

    def test_storjtelehash(self):
        self.status = 0
        self.m4.add_broadcaster(self.location, 1)
        self.m3.broadcast(self.location, '{"service":"farming0"}')
        time.sleep(2)
        assert self.status == 2

        self.status = 0
        self.m2.add_channel_handler('counter_test', ChannelReceiver)
        self.m3.open_channel(self.location, 'counter_test', ChannelOpener())
        time.sleep(2)
        assert counter_opener == 2
        assert counter_receiver == 1
