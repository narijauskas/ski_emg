# using HidApi
using LibSerialPort
using DataStructures

using GLMakie
display(lines(rand(10)))

## -------------------------- -------------------------- ##



function echo(f, portname)
    baudrate = 115200 # teensy ignores baudrate
    LibSerialPort.open(portname, baudrate) do sp
        read(sp) # clear buffer
        try
            while true
                sleep(0.01)
                if bytesavailable(sp) > 0
                    f(readline(sp))
                end
            end
        catch e
            # stop via interrupt... also supresses errors
        end
    end
end

echo(portname) = echo(println, portname)

echo(println, "COM4")




# open teensy
# while open
    # recv packet
    # if stream, unpack & print
    # if msg, print

## -------------------------- -------------------------- ##

function trace(portname, n; npoints = 100)

    # build figure
    fig = Figure(); ax = Axis(fig[1,1])

    # create empty buffers & plot them
    buffs = map(2:n+1) do i
        cb = Observable(CircularBuffer{Float64}(npoints))
        fill!(cb[], 0.0)
        obvec = lift(collect, cb)
        lines!(ax, obvec)
        return cb
    end
    ylims!(ax, 0, 4096) 
    display(fig)

    echo(portname) do pkt
        for (v, cb) in zip(split(String(pkt), ",")[2:n+1], buffs)
            push!(cb[], parse(Float64, v))
            notify(cb) # update plot
        end
    end

end