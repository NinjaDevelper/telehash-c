[![Build Status](https://travis-ci.org/StorjPlatform/StorjTelehash.svg?branch=master)](https://travis-ci.org/StorjPlatform/StorjTelehash)
[![Coverage Status](https://coveralls.io/repos/StorjPlatform/StorjTelehash/badge.svg?branch=master)](https://coveralls.io/r/StorjPlatform/StorjTelehash?branch=master)

# Messaging Layer on Storj Platform by Telehash

## Requirements
This requires 
* `g++` (v4.8 or higher for test)
* `python` (2.x or 3.x)

## Installation

To compile 

    $ make python
    
To run the associated tests:

    $ make test
    $ LD_LIBRARY_PATH=libtap ./test

To run the associated tests for python:

    $ PYTHONPATH=. py.test -q tests/test.py -s


## Usage

Explanation about messaging layer is [here](Messaging.md).

API Document for abstract messaging layer is [here](https://rawgit.com/StorjPlatform/StorjTelehash/master/docs/html/messaging.html)

API Document for storjtelehash , which is concorete implementation of abstract messaging layer by telehash-c ,
is [here](https://rawgit.com/StorjPlatform/StorjTelehash/master/docs/html/storjtelehash.html)

API Document for telehashbinder , which is classes/functions used by storjtelehash,
is [here](https://rawgit.com/StorjPlatform/StorjTelehash/master/docs/html/telehashbinder.html)

# Contribution
Improvements to the codebase and pull requests are encouraged.


