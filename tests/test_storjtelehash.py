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


import json
import pytest
import time
import logging

import storj.messaging
from storj.messaging.messaging import Messaging
from storj.messaging.messaging import ChannelHandler
import storj.messaging.storjtelehash

log_fmt = '%(filename)s:%(lineno)d %(funcName)s() %(message)s'
logging.basicConfig(level=logging.DEBUG, format=log_fmt)

counter_opener = 0
counter_receiver = 0

cls = storj.messaging.storjtelehash.StorjTelehash


class CalledChannel(ChannelHandler):

    def seqA(self, packet):
        logging.debug('called channel packet=' + packet)
        global counter_opener
        j = json.loads(packet)
        assert j['count'] == 3
        counter_opener = counter_opener + 1
        return '{\"count\":4}'


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
        self.call = CalledChannel()
        return '{\"count\":2}'

    def seqC(self, packet):
        logging.debug('called openerC')
        global counter_opener
        counter_opener = counter_opener + 1
        j = json.loads(packet)
        assert j['count'] == 5
        self.call = CalledChannel()
        return None

    def seqZ(self, packet):
        counter_opener = 9999
        return None


class ChannelReceiver(ChannelHandler):

    def seqA(self, packet):
        logging.debug('called receiverA')
        global counter_receiver
        counter_receiver = counter_receiver + 1
        j = json.loads(packet)
        self.next = self.seqC
        assert j['count'] == 0
        return '{\"count\":1}'

    def seqB(self, packet):
        counter_receiver = 9999
        return None

    def seqC(self, packet):
        logging.debug('called receiverB message='+packet)
        global counter_receiver
        j = json.loads(packet)
        assert j['count'] == 2
        counter_receiver = counter_receiver + 1
        return '{\"count\":3}'

    def seqD(self, packet):
        logging.debug('called receiverB message='+packet)
        global counter_receiver
        j = json.loads(packet)
        assert j['count'] == 4
        counter_receiver = counter_receiver + 1
        return '{\"count\":5}'

    def seqZ(self, packet):
        counter_receiver = 9999
        return None


class ChannelReceiverNG(ChannelHandler):

    def seqA(self, packet):
        logging.debug('called openerNG')
        return '{\"count\":1}'


class TestStorjTelehash(object):
    def setup(self):
        self.m2 = cls(self.broadcast_handler2)
        self.m3 = cls(self.broadcast_handler3, port=-9999)
        self.m4 = cls(self.broadcast_handler4, port=-9999)

        id = self.m2.get_my_id()
        assert len(id) == 52
        logging.debug("id=" + id)
        self.location = self.m2.get_my_location()
        logging.debug("location=" + self.location)
        self.status2 = 0
        self.status4 = 0

    def broadcast_handler2(self, packet):
        logging.debug("broadcast packet2=" + packet)
        j = json.loads(packet)
        assert j['service'] == 'farming0'
        self.status2 += 1
        j['service'] = "farming1"
        return json.dumps(j)

    def broadcast_handler3(self, packet):
        logging.debug("broadcast packet3=" + packet)
        assert 1 == 0
        return None

    def broadcast_handler4(self, packet):
        logging.debug("broadcast packjet4=" + packet)
        j = json.loads(packet)
        assert j['service'] == 'farming1'
        self.status4 += 1
        return None

    def test_storjtelehash(self):
        with pytest.raises(TypeError):
            m = cls("aaa", port=1234)

        with pytest.raises(TypeError):
            self.m3.open_channel(self.location, 'counter_test',
                                 ChannelOpener)

        self.status = 0
        self.m4.add_broadcaster(self.location, 1)
        self.m3.broadcast(self.location, '{"service":"farming0"}')
        time.sleep(2)
        assert self.status2 == 1
        assert self.status4 == 1

        self.status = 0
        self.m2.add_channel_handler('counter_test',
                                    (lambda: ChannelReceiver()))
        self.m3.open_channel(self.location, 'counter_test', ChannelOpener())
        time.sleep(2)
        assert counter_opener == 4
        assert counter_receiver == 3

#       with pytest.raises(IOError):
        self.m2.add_channel_handler('counter_testNG', ChannelReceiverNG)
        self.m3.open_channel(self.location, 'counter_testNG',
                             ChannelOpener())
        time.sleep(2)
        logging.debug('should to raise exception in another thread,\
         but cannot catch')

        with pytest.raises(TypeError):
            self.m2.get_channel_handler("nothing")

#        with pytest.raises(TypeError):
        self.m2.add_channel_handler('testtest', (lambda: None))
        self.m3.open_channel(self.location, 'testtest', ChannelOpener())
        logging.debug('should to raise exception in another thread,\
         but cannot catch')

        self.m2.finalize()
        self.m3.finalize()
        self.m4.finalize()
