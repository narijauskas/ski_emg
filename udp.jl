using Sockets
using DataStructures
# using Observables
using GLMakie
display(plot(rand(10)))



host = ip"192.168.0.36"
port = 54545
n = 2

function udp_listener(host, port, n)
    sock = UDPSocket()
    if bind(sock, host, port)
        # Threads.@spawn begin
            try
                while true
                    from, pck = recvfrom(sock)
                    println("values received:")
                    for str in split(String(pck), ",")[1:n]
                        println("    $str")
                    end
                end
            finally
                close(sock)
            end
        # end
    end
end


function emg_monitor(host, port, n, flag=Ref(true); npoints = 100)
    sock = UDPSocket()
    fig = Figure()
    ax = Axis(fig[1,1])

    # make & plot buffers
    buffs = map(1:n) do i
        cb = Observable(CircularBuffer{Float64}(npoints))
        fill!(cb[], 0.0)
        obvec = lift(collect, cb)
        lines!(ax, obvec)
        return cb
    end
    ylims!(ax, 0, 4000)
    display(fig)
    

    if bind(sock, host, port)
        try
            while true == flag[]
                from, pck = recvfrom(sock)
                # parse string (break at commas)        
                for (v, cb) in zip(split(String(pck), ",")[1:n], buffs)
                    push!(cb[], parse(Float64, v))
                    notify(cb) # update plot
                end
            end
        finally
            close(sock)
        end
    end
end

flag = Ref(true)
emg_monitor(host, port, n, flag)










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