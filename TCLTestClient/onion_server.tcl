#/usr/bin/wish85

# This is a simple Onion server interface for testing

# Author: Patrick Shields
# Date: 3/11/2014


set ::HOST ""
set ::PORT 2721
set ::KEEPALIVE 15

set ::DEVICE_ID  "RsyI3k9O"
set ::DEVICE_KEY "NgbaClBcKcxcYVtn"

set ::ONIONPROTOCOLVERSION 	1
set ::ONIONCONNECT     		[expr 1 << 4 ] ;# // Client request to connect to Server
set ::ONIONCONNACK     		[expr 2 << 4 ] ;# // Connect Acknowledgment
set ::ONIONPUBLISH     		[expr 3 << 4 ] ;# // Publish message
set ::ONIONPUBACK      		[expr 4 << 4 ] ;# // Publish Acknowledgment
set ::ONIONPUBREC      		[expr 5 << 4 ] ;# // Publish Received (assured delivery part 1)
set ::ONIONPUBREL      		[expr 6 << 4 ] ;# // Publish Release (assured delivery part 2)
set ::ONIONPUBCOMP     		[expr 7 << 4 ] ;# // Publish Complete (assured delivery part 3)
set ::ONIONSUBSCRIBE   		[expr 8 << 4 ] ;# // Client Subscribe request
set ::ONIONSUBACK      		[expr 9 << 4 ] ;# // Subscribe Acknowledgment
set ::ONIONUNSUBSCRIBE 		[expr 10 << 4] ;# // Client Unsubscribe request
set ::ONIONUNSUBACK    		[expr 11 << 4] ;# // Unsubscribe Acknowledgment
set ::ONIONPINGREQ     		[expr 12 << 4] ;# // PING Request
set ::ONIONPINGRESP    		[expr 13 << 4] ;# // PING Response
set ::ONIONDISCONNECT  		[expr 14 << 4] ;# // Client is Disconnecting
set ::ONIONReserved    		[expr 15 << 4] ;# // Reserved

set ::ONIONQOS0        		[expr (0 << 1)]
set ::ONIONQOS1        		[expr (1 << 1)]
set ::ONIONQOS2        		[expr (2 << 1)]

set ::NextMsgId 0
set ::Sock ""
set ::ServerSock ""
set ::EveryAfter ""
set ::LastInActivity 0
set ::LastOutActivity 0
set ::PingOutstanding 0


proc start {} {
    set ::ServerSock [socket -server handleConnection $::PORT]
    every 100 cleanup
}

proc stop {} {
    # Close all open connections
    foreach conn [array names ::Connections] {
        catch {close $conn}
    }
    close $::ServerSock
    after cancel $::EveryAfter
}

proc handleConnection {s addr port} {
    set ::Connections($s) [list $addr $port]
    fconfigure $s -blocking 0
    fconfigure $s -encoding binary -translation binary
    fileevent $s readable [list handleConnectionRead $s]
    puts "New Connection from $addr:$port"
}

proc handleConnectionRead {s} {
     set data [read $s]
     # parse packet
     while {[string length $data] > 2} {
        binary scan $data H2SH* type length payload
        binary scan [string range $data 3 [expr 2+$length]] H* payload
        puts "Raw data: type=$type length=$length payload=$payload"
        parsePacket $s $type $length $payload 
        set data [string range $data [expr 4+($length)] end]
     }
}

proc parsePacket {sock type length payload} {
    switch $type {
        "10" {
            puts "Got Connection Packet: Payload = $payload"
            send $sock 20 00
        }
        "30" {
            puts "Got Publish Packet: Payload = $payload"
            send $sock 40 00
        }
        "80" {
            puts "Got Subscribe Packet: Payload = $payload"
            send $sock 90 00
        }
        "c0" {
            puts "Got Ping Packet: Payload = $payload"
            send $sock D0 ""
        }
    }
}

proc send {sock command payload} {
    set length [expr [string length $payload]/2]
    puts -nonewline $sock [binary format H2SH* $command $length $payload]
    flush $sock 
}

proc parseMsgPack {hex} {
        
}

proc every {ms body} {
        global EveryAfter
        eval $body; set EveryAfter [after $ms [info level 0]]
#       eval $body; after idle [info level 0]

}

proc cleanup {} {
    foreach conn [array names ::Connections] {
        set eof 0
        if {[catch {set eof [eof $conn]}]} {
            catch {close $conn}
            array unset ::Connections $conn
        }
        if {$eof} {
            catch {close $conn}
            array unset ::Connections $conn
        }
    }
}

proc stopLoop {} {
    after cancel $::EveryAfter
}