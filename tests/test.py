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
from MessagingTelehash import MessagingTelehash
import TelehashBinder
import json
import pytest
import time

import logging
log_fmt = '%(filename)s:%(lineno)d %(funcName)s() %(message)s'
logging.basicConfig(level=logging.DEBUG, format=log_fmt)

counter_opener = 0
counter_receiver = 0


class ChannelOpener(Messaging.ChannelHandler):

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


class ChannelReceiver(Messaging.ChannelHandler):

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
        counter = counter + 1
        return packet


class TestMessagingTelehash(object):

    def setup(self):
        TelehashBinder.setGC(0)
        self.m3 = MessagingTelehash(self.broadcastHandler, -9999)
        self.m4 = MessagingTelehash(self.broadcastHandler, -9999)
        # a channel handler will be overwritten. only last one is valid.
        self.m2 = MessagingTelehash(self.broadcastHandler, 1234)

        loc = self.m2.getMyLocation()
        self.location =  \
            '{"keys":{"1a\":"' + json.loads(loc)['keys']['1a']  + '"},' \
            '"paths\":[{"type":"udp4","ip":"127.0.0.1","port":1234}]}'
        logging.debug("location=" + self.location)

    def broadcastHandler(self, packet):
        j = json.loads(packet)
        assert j['service'] == 'farming%d' % (self.status)
        self.status = self.status + 1
        logging.debug("packjet=" + packet)
        if self.status == 2:
            return None
        j['service'] = "farming%d" % (self.status)
        return json.dumps(j)

    def test_MessagingTelehash(self):
        self.status = 0
        self.m4.addBroadcaster(self.location, 1)
        self.m3.broadcast(self.location, '{"service":"farming0"}')
        time.sleep(2)
        assert self.status == 2

        self.status = 0
        self.m2.addChannelHandler('counter_test', ChannelReceiver)
        self.m3.openChannel(self.location, 'counter_test', ChannelOpener())
        time.sleep(2)
        assert counter_opener == 2
        assert counter_receiver == 1
