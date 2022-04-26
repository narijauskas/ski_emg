# using HidApi
using LibSerialPort
using DataStructures

using GLMakie
display(lines(rand(10)))

## -------------------------- -------------------------- ##



function echo(f, portname)
    baudrate = 115200 # teensy ignores baudrate
    LibSerialPort.open(portname, baudrate) do sp
        # clear buffer
        read(sp)

        try
            while true
                sleep(0.01)
                if bytesavailable(sp) > 0
                    # pkt = readline(sp)
                    # first(pkt)
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

function trace(portname, n)

    # build figure
    fig = Figure(); ax = Axis(fig[1,1])

    # create empty buffers & plot them
    buffs = map(1:n) do i
        cb = Observable(CircularBuffer{Float64}(npoints))
        fill!(cb[], 0.0)
        obvec = lift(collect, cb)
        lines!(ax, obvec)
        return cb
    end
    ylims!(ax, 0, 4000)
    display(fig)



end