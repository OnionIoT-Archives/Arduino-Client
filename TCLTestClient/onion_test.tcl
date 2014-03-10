#/usr/bin/wish85

# This is an attempt to define the protocol for Onion.IO

# Author: Patrick Shields
# Date: 2/24/2014


set ::HOST "mqtt.onion.io"
set ::PORT 1883
set ::KEEPALIVE 15

set ::DEVICE_ID  "RsyI3k9O"
set ::DEVICE_KEY "NgbaClBcKcxcYVtn"

set ::MQTTPROTOCOLVERSION 	3
set ::MQTTCONNECT     		[expr 1 << 4 ] ;# // Client request to connect to Server
set ::MQTTCONNACK     		[expr 2 << 4 ] ;# // Connect Acknowledgment
set ::MQTTPUBLISH     		[expr 3 << 4 ] ;# // Publish message
set ::MQTTPUBACK      		[expr 4 << 4 ] ;# // Publish Acknowledgment
set ::MQTTPUBREC      		[expr 5 << 4 ] ;# // Publish Received (assured delivery part 1)
set ::MQTTPUBREL      		[expr 6 << 4 ] ;# // Publish Release (assured delivery part 2)
set ::MQTTPUBCOMP     		[expr 7 << 4 ] ;# // Publish Complete (assured delivery part 3)
set ::MQTTSUBSCRIBE   		[expr 8 << 4 ] ;# // Client Subscribe request
set ::MQTTSUBACK      		[expr 9 << 4 ] ;# // Subscribe Acknowledgment
set ::MQTTUNSUBSCRIBE 		[expr 10 << 4] ;# // Client Unsubscribe request
set ::MQTTUNSUBACK    		[expr 11 << 4] ;# // Unsubscribe Acknowledgment
set ::MQTTPINGREQ     		[expr 12 << 4] ;# // PING Request
set ::MQTTPINGRESP    		[expr 13 << 4] ;# // PING Response
set ::MQTTDISCONNECT  		[expr 14 << 4] ;# // Client is Disconnecting
set ::MQTTReserved    		[expr 15 << 4] ;# // Reserved

set ::MQTTQOS0        		[expr (0 << 1)]
set ::MQTTQOS1        		[expr (1 << 1)]
set ::MQTTQOS2        		[expr (2 << 1)]

set ::NextMsgId 0
set ::Sock ""
set ::EveryAfter ""
set ::LastInActivity 0
set ::LastOutActivity 0
set ::PingOutstanding 0

proc begin {} {
    set topic "/$::DEVICE_ID"
    set init "$::DEVICE_ID;CONNECTED"
    if {[connect $::DEVICE_ID $::DEVICE_KEY]} {
        publish "/register" $init
        subscribe $topic
    }
    puts "Registering command"
    get "/on" "1"
    get "/off" "2"
    post "/set" "3" "value"
    puts "Starting background loop"
    every 100 loop
}

proc connect {device_id device_key} {
    # 0x00, 0x06, 'M', 'Q', 'I', 's', 'd', 'p', MQTTPROTOCOLVERSION
    set buffer "[binary format c* {0x00 0x06}]MQIsdp[binary format c $::MQTTPROTOCOLVERSION]"
    
    set v [binary format c [expr 0x02 | 0x80 | 0x40]]
    set buffer "$buffer$v"
    set buffer "$buffer[binary format c [expr $::KEEPALIVE/256]][binary format c [expr $::KEEPALIVE%256]]"
    set buffer "$buffer[convert_string $device_id][convert_string $device_id][convert_string $device_key]"
    
    # Open socket
    open_socket
    set ::NextMsgId 1
    #puts "About to write to socket: Connect - $buffer\n\n"
    write_socket $::MQTTCONNECT $buffer
    # Try to read back response
    set resp ""
    set start [clock milliseconds]
    set ::LastInActivity $start
    set ::LastOutActivity $start
    while {(![eof $::Sock]) && ($resp eq "")} {
        set resp [read $::Sock]
        set now [clock milliseconds]
        set delta [expr ($now-$start)/1000]
        if {$delta > $::KEEPALIVE} {
            puts "Timed out"
            return 0
        }
    }
    if {[string length $resp]>3} {
        binary scan $resp cccc one two three four
        #puts "Got response of: $one $two $three $four"
        if {$four == 0} {
            set ::LastInActivity [clock milliseconds]
            set ::PingOutstanding 0
            return 1
        }
    } else {
        puts "Err: I got $resp"
    }
    
    return 0
    
}

proc convert_string {str} {
    set len [string length $str]
    return "[binary format c [expr $len/256]][binary format c [expr $len%256]]$str"
}

proc open_socket {} {
    if {$::Sock ne ""} {
        catch {close $::Sock}
    }
    set ::Sock [socket $::HOST $::PORT]
    fconfigure $::Sock -blocking 0 -encoding binary -translation binary
}

proc write_socket {header buffer} {
    set len [string length $buffer]
    set lenbuf [getLenString $len]
    binary scan $lenbuf H* hex
    #puts "Length of $len was converted to $hex"
    set buf "[binary format c $header]$lenbuf$buffer"
    binary scan $buf H* hex
    #puts "Sending buffer = 0x$hex"
    if {$::Sock ne ""} {
        puts -nonewline $::Sock $buf
        flush $::Sock
    }
}

proc getLenString {length} {
    if {$length == 0} {
	    return [binary format c 0]
	}
	set buffer ""
	while {$length > 0} {
	    set digit [expr $length % 128]
	    set length [expr $length / 128]
	    if {$length > 0} {
	        set digit [expr $digit | 0x80]
	    }
	    set buffer "$buffer[binary format c $digit]"
	}
	return $buffer
}

proc publish {topic payload} {
    # Check for connected
    if {![checkSocketOpen]} {
        return ""
    }
    # Send data
    set buffer "[convert_string $topic]$payload"
    write_socket $::MQTTPUBLISH $buffer
}

proc subscribe {topic} {
    # Check to make sure we're connected
    if {![checkSocketOpen]} {
        return ""
    }
    
    incr ::NextMsgId
    if {($::NextMsgId == 0) || ($::::NextMsgId > 65535)} {
        set ::NextMsgId 1
    }
    set buffer [binary format c [expr $::NextMsgId / 256]][binary format c [expr $::NextMsgId % 256]]
    set buffer "$buffer[convert_string $topic]"
    set buffer "$buffer[binary format c 0]"
    write_socket [expr $::MQTTSUBSCRIBE | $::MQTTQOS1] $buffer
}

proc callback {topic payload} {
    set sections [split $payload ";"]
    set id [lindex $sections 0]
    set params [lrange $sections 1 end]
    puts "I got a callback for Id=$id with Params=$params Topic=$topic"
}

proc loop {} {
    # Check to make sure we're connected
    if {[checkSocketOpen]} {
        set now [clock milliseconds]
        set inDelta [expr $now - $::LastInActivity]
        set outDelta [expr $now - $::LastOutActivity]
        if {($inDelta > ($::KEEPALIVE*1000)) || ($outDelta > ($::KEEPALIVE*1000))} {
            if {$::PingOutstanding} {
                # Close socket
                
                return 0
            } else {
                write_socket $::MQTTPINGREQ ""
                puts "Sent timeout Ping"
                set ::LastInActivity $now
                set ::LastOutActivity $now
                set ::PingOutstanding 1
            }
        }
        set recv [read $::Sock]
        if {[string length $recv] > 0} {
            puts "Got some data!"
            set ::LastInActivity $now
            binary scan [string range $recv 0 0] H* typeHex
            set type [expr 0x$typeHex & 0xF0]
            if {$type == $::MQTTPUBLISH} {
                # Read packet length data
                binary scan [string range $recv 1 1] H* byte
                set digit [expr 0x$byte]
                set length [expr $digit & 0x7F]
                set mult 1
                set index 2
                while {$digit & 128} {
                    set mult [expr $mult * 128]
                    binary scan [string range $recv $index $index] H* byte
                    set digit [expr 0x$byte]
                    set length [expr $length + ($digit & 127)*$mult]
                    incr index
                }
                # Read topic string length
                puts "Extracting Topic Length: Raw Data = [string range $recv $index [expr $index+1]]"
                binary scan [string range $recv $index [expr $index+1]] Su topicLength
                incr index 2
                set topic [string range $recv $index [expr $index+$topicLength-1]]
                incr index $topicLength
                set payload [string range $recv $index end]
                callback $topic $payload
            } elseif {$type == $::MQTTPINGREQ} {
                write_socket $::MQTTPINGRESP ""
                puts "Sent a Ping (got a request)"
            } elseif {$type == $::MQTTPINGRESP} {
                set ::PingOutstanding 0
                puts "Got a pong"
            }
        }
    } else {
        connect $::DEVICE_ID $::DEVICE_KEY 
    }
}

proc get {endPoint functionId} {
    set payload "$::DEVICE_ID;GET;$endPoint;$functionId"
    publish "/register" $payload
}

proc post {endPoint functionId arguments} {
    set payload "$::DEVICE_ID;POST;$endPoint;$functionId;$arguments"
    publish "/register" $payload
}

proc checkSocketOpen {} {
    if {$::Sock eq ""} {return 0}
    if {[catch {eof $::Sock} closed]} {
        return 0
    }
    if {$closed} {
        return 0
    }
    return 1
}


proc every {ms body} {
        global EveryAfter
        eval $body; set EveryAfter [after $ms [info level 0]]
#       eval $body; after idle [info level 0]

}

proc stopLoop {} {
    after cancel $::EveryAfter
}