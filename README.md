# intel-mcs-example
The .MCS file format is also know as the Intel HEX format (or I32HEX). For
more info see
(Wikipedia's Intel HEX page)[https://en.wikipedia.org/wiki/Intel_HEX]. The
protocol is all in ASCII, so there is a bunch of string scanning and parsing
out hex data that way. I put that into packed structures because it makes my
life easier when it comes to the checksum calculation. Not all of them needed
to be packed, but I like consistency.
