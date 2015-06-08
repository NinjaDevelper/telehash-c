#!/usr/bin/env python
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

from setuptools.command.test import test as TestCommand
import os
from distutils.core import setup, Extension

LONG_DESCRIPTION = open('README.md').read()
VERSION = '1.0'

install_requirements = []

test_requirements = [
    'pytest>=2.6.4',
    'pytest-pep8',
    'pytest-cache',
    'coveralls'
]

telehash_src = [
    'telehash-c/src/ext/stream.c',
    'telehash-c/src/ext/block.c',
    'telehash-c/src/pipe.c',
    'telehash-c/src/unix/util_sys.c',
    'telehash-c/src/unix/util.c',
    'telehash-c/src/net/serial.c',
    'telehash-c/src/net/udp4.c',
    'telehash-c/src/net/loopback.c',
    'telehash-c/src/net/tcp4.c',
    'telehash-c/src/lib/socketio.c',
    'telehash-c/src/lib/hashname.c',
    'telehash-c/src/lib/base32.c',
    'telehash-c/src/lib/js0n.c',
    'telehash-c/src/lib/chacha.c',
    'telehash-c/src/lib/base64.c',
    'telehash-c/src/lib/xht.c',
    'telehash-c/src/lib/murmur.c',
    'telehash-c/src/lib/lob.c',
    'telehash-c/src/link.c',
    'telehash-c/src/mesh.c',
    'telehash-c/src/e3x/channel.c',
    'telehash-c/src/e3x/cs1a/aes-internal.c',
    'telehash-c/src/e3x/cs1a/aes-internal-enc.c',
    'telehash-c/src/e3x/cs1a/cs1a.c',
    'telehash-c/src/e3x/cs1a/hmac.c',
    'telehash-c/src/e3x/cs1a/sha256.c',
    'telehash-c/src/e3x/cs1a/uECC.c',
    'telehash-c/src/e3x/cs1a/aes.c',
    'telehash-c/src/e3x/cs2a_disabled.c',
    'telehash-c/src/e3x/exchange.c',
    'telehash-c/src/e3x/cs3a_disabled.c',
    'telehash-c/src/e3x/e3x.c',
    'telehash-c/src/e3x/self.c',
    'telehash-c/src/e3x/event.c',
    'telehash-c/src/e3x/cipher.c',
    'telehash-c/src/util/uri.c',
    'telehash-c/src/util/util.c',
    'telehash-c/src/util/chunks.c',
]

module = Extension('storj.telehashbinder',
                   sources = ['cxx/telehashbinder_python.cpp',
                    'cxx/StorjTelehash.cpp'] + telehash_src,
                    extra_compile_args = ['-fpermissive'],
                    include_dirs=['telehash-c/unix', 'telehash-c/include',
                                 'telehash-c/lib', 'cxx'],
                   )


class PyTest(TestCommand):
    user_options = [('pytest-args=', 'a', "Arguments to pass to py.test")]

    def initialize_options(self):
        TestCommand.initialize_options(self)
        self.pytest_args = []

    def finalize_options(self):
        TestCommand.finalize_options(self)
        self.test_args = []
        self.test_suite = True

    def run_tests(self):
        # Import PyTest here because outside, the eggs are not loaded.
        import pytest
        import sys
        errno = pytest.main(self.pytest_args)
        sys.exit(errno)

setup(
    name='storjtelehash',
    version=VERSION,
    url='https://github.com/StorjPlatform/StorjTelehash',
    download_url='https://github.com/StorjPlatform/StorjTelehash/tarball/'
    + VERSION,
    license=open('LICENSE').read(),
    author='Shinya Yagyu',
    author_email='utamaro.sisho@gmail.com',
    description='Messaging Layer in Telehash.',
    long_description=LONG_DESCRIPTION,
    packages=['storj'],
    cmdclass={'test': PyTest},
    ext_modules=[module],
    install_requires=install_requirements,
    tests_require=test_requirements,
    namespace_packages=['storj'],
    keywords=['storj', 'storj platform', 'messaging layer', 'telehash']
)
