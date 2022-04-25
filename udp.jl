using Sockets

host = ip"192.168.0.36"
port = 54545

function udp_listener(host::IPAddr, port)
    sock = UDPSocket()
    if bind(sock, host, port)
        Threads.@spawn begin
            try
                while true
                    from, pck = recvfrom(sock)
                    println(String(pck))
                end
            finally
                close(sock)
            end
        end
    end
end










function rec_data(port::Integer, to::Sockets.InetAddr)
    sock = UDPSocket()
    if bind(sock, Sockets.localhost, port)
        Threads.@spawn begin
            while true
                from, pck = recvfrom(sock)
                msg = deserialize(pck)
                if msg == :exit
                    break
                else
                    Threads.@spawn par_serve(from, to, msg)
                end
            end
            close(sock)
        end
    else
        println(stderr, "port $port not available")
    end
end

function par_serve(from, to, msg)
    # do something with msg and from
    output = some_op_taking_time(from, msg)
    outpck = serialize(output)
    sock = UDPSocket()
    send(sock, to.host, to.port, outpck)
    close(sock)
end