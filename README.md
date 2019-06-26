# OSP2P

OSP2P is an operating system interface for a distributed computing peer-to-peer network.

## Overview

OSP2P network consists of two kinds of nodes: trackers and peers. Trackers keep track of which peers are connected to the network; peers actually download files from each other. Peer-to-peer communication in the OSP2P system is built from a series of remote procedure calls (RPCs). These RPCs are formatted as normal text, formatted somewhat like other Internet protocols (HTTP, FTP, SMTP, and so forth).

### The OSP2P Protocol

A peer logs in to a tracker as follows.

1. The peer connects to the tracker machine.
2. The tracker responds with a greeting message.
3. The peer registers its IP address and port with an "ADDR" RPC.
4. The tracker responds with a message indicating success or failure. Now the tracker will report this peer to other peers who wan to download files. 

The peer then informs the tracker of the files it is willing to upload to others.

1. The peer registers each file with a "HAVE filename" RPC.
2. The tracker responds with a message indicating success or failure.

When a peer wants to download a file from the network, it communicates with both the tracker and other peers. 

1. The downloading peer asks the tracker which peers have the file with a "WANT filename" RPC.
2. The tracker responds with a message listing the available peers willing to serve that file.
3. The downloading peer picks an available peer from this set, and connects to that peer, sending it a "GET filename OSP2P" RPC.
4. The uploading peer responds to this RPC by sending the entire file.

Once the downloading peer has downloaded the entire file, it informs the tracker that it, too, has the file.

1. The downloading peer registers its newly downloaded file with a "HAVE filename" RPC.
2. The tracker responds with a message indicating success or failure.

## Features

- Implements parallel upload/download between peers
- Robust communication using defensive programming
- Configurable option for evil peer that aims to crash any peers that attempt to download files from it

## Installation

Extract into ubuntu distribution and run in QEMU emulator with `./run-good`.
