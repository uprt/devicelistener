# libasio example
This was a trial task for one job application.
The task was "to compute the number of messages received from some devices".
I decided to use libasio (boost::asio), let my app receive messages in async way, support variable-width packets and let the protocol use TCP as a transport level.

# Why boost::asio?
Well, it is a mature and reliable library widely used in production.
It has good API and architecture, so it fits to our task perfectly.
Also, using asio I can later easily extend my program, for example, to receive data not only from TCP sockets, but from RS-232/RS-485 too, or make it not single-threaded, but with multiple parallel workers (actually, to do this some code changes are needed, like not forget to add some mutexes and guard_locks to MsgCounter class and logging calls).

# The protocol
The protocol itself is inspired by RFC 1006 a bit: https://tools.ietf.org/html/rfc1006
It has a simple preamble:
```c
{
  uint8_t version;
  uint8_t reserved;
  uint16_t length;
}
```
That helps us to split messages that can come together or vice versa not completely due to TCP's "streaming" nature'.
After that, the payload comes. It consists of the payload itself prepended with metadata header.
I don't care about byte order in this case, let's consider that devices have the same endianness as our machine's architecture.
Also I reserved a number of fields in it to keep different metadata of the measurement:
```c
{
  uint16_t deviceId;
  uint16_t measurementTag;
  uint32_t timestamp;
  uint16_t measurementType;
  uint16_t dataLength;
}
```
Actually, I'm only gathering deviceId from this payload header and perform no validation for other fields, because our job is only to count messages.

# Devices description
As we can see from the protocol headers, devices are identified using the unique integer ID.
To be able pretty-print them with human-readable names, there can be a file with "deviceId" - "deviceName" mapping, splitted by ":" symbol.
Example:
```c
1:SomeDevice
2:SomeThermometer
3:SomePowerSupply
4:SomeNetworkSwitch
```

# Building
I've tested it on Ubuntu 18.04 and Debian Buster (Sid), so the manual will be for them. 
As I use Boost and CMake, we need to fetch them:
```apt-get install cmake libasio-dev libboost-system1.67.0 libboost-system1.67-dev```

And, of course, we need a compiler. 
I tested with gcc version 7.3.0 and clang 6.0.0

How to build:
```bash
https://github.com/uprt/libasio_example.git
cd libasio_example
cmake ./
make
```

After that you will have binaries in bin/ directory.

# Usage
By default it starts on 5555 TCP port, trying to read device descriptions from ./devices.conf file and prints statistics to stdout every 5 seconds.
This can be overriden using command line arguments. Starting with "-?" will print small help info about it:
```
$ bin/libasio_example -?
Available command line arguments:
-f <filename> - path to the file with devices descriptions
-p <port> - TCP port to run server on
-i <interval> - interval (in seconds) to print statistics to stdout
```
Example:
```$ bin/libasio_example -p 5555 -f ./devices.conf -i 10 ```

After that the server will start printing currents statistics to stdout:

```
Added device 'SomeDevice' with id = 1
Added device 'SomeThermometer' with id = 2
Added device 'SomePowerSupply' with id = 3
Added device 'SomeNetworkSwitch' with id = 4
Server is listening on port 5555...
-----------------------------------------------------
Current statistics of received messages from devices:
[device id] - [number of valid messages]
SomeNetworkSwitch - 114
SomePowerSupply - 6
SomeDevice - 78
SomeThermometer - 0
-----------------------------------------------------
```
# The simulator
I also made a small quick-and-dirty device simulator for debugging and demonstration purposes. It is written in pure C without any 3rd-party dependencies. We can start it like this:
```./dsimulator 'server_ip' 'server_port' 'device_id' 'intensity_multiplier'```
'intesity_multipplier' is [1..100] coefficient that sets how intensive our simulator will send packets to the server.

We can start a group of simulators in parallel:
```bash
./dsimulator 127.0.0.1 5555 1 10 &
./dsimulator 127.0.0.1 5555 2 50 &
./dsimulator 127.0.0.1 5555 3 99 &
./dsimulator 127.0.0.1 5555 4 15 &
```
They all will exit when the connection with the server is lost.


# Test
Yes, I have some tests for the protocol parser and message counter. They are using GTest framework and not built by default.

```
cd devicelistener/tests
cmake ./
make
../bin/device_listener_test
```
