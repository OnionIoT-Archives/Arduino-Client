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
    puts "Recv From Sock: $sock"
    set ::s $sock
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

proc publishData {sock funcId {params ""}} {
    # this will pack the list of params into publish packet and send it
    set paramLength [llength $params]
    set payload [msgPackArray [expr $paramLength + 1]]
    append payload [msgPackInt $funcId]
    foreach param $params {
        append payload [msgPackString $param]
    }
    send $sock 30 $payload
    
}

proc parseMsgPack {hex} {
        
}

proc msgPackInt int {
    if {$int >= 0} {
        if {$int < 128} {
            return [format "%02X" $int]
        } elseif {$int < 256} {
            return CC[format "%02X" $int]
        } elseif {$int < 65536} {
            return CD[format "%04X" $int]
        } else {
            return CE[format "%04X" $int]
        }
    } else {
        if {$int > -32} {
            return [string range [format "%02hX" $int] end-1 end]
        } elseif {$int > -129} {
            return D0[string range [format "%02hX" $int] end-1 end]
        } elseif {$int > -32768} {
            return D1[format "%04hX" $int]
        } else {
            return D2[format "%08X" $int]
        }
    }
}

proc msgPackArray {length} {
    if {$length < 16} {
        return [format "%02X" [expr 0x90 + $length]]
    } elseif {$length < 65536} {
        return DC[format "%04X" $length]
    } else {
        return DD[format "%08X" $length]
    }
}


proc msgPackString {str} {
    set length [string length $str]
    binary scan $str H* strHex
    if {$length < 32} {
        return [format "%02X" [expr 0xA0 + $length]]$strHex
    } elseif {$length < 256} {
        return D9[format "%02X" $length]$strHex
    } else {
        return DA[format "%04X" $length]$strHex
    }
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
set ::BeatingAfter ""

proc stopLoop {} {
    after cancel $::EveryAfter
}

proc startBeating {} {
    if {$::BeatingAfter == ""} {
        set ::BeatCount 0
        beatevery
    }
}

proc setBeating {ms body} {
    set ::SmackTime $ms
    set ::SmackCode $body
}

proc beatevery {} {
    global BeatingAfter
    eval $::SmackCode
    incr ::BeatCount
    set BeatingAfter [after $::SmackTime [info level 0]]
}

proc stopBeating {} {
    after cancel $::BeatingAfter
    set ::BeatingAfter ""
    puts "Code was beat $::BeatCount times"
    set ::BeatCount 0
}
set ::LED 0
proc toggleLED {} {
    if {$::LED} {
        publishData $::s 2
        set ::LED 0
    } else {
        publishData $::s 1
        set ::LED 1
    }
}