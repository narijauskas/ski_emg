# using HidApi
using LibSerialPort
using DataStructures

using GLMakie
display(lines(rand(10)))



# open teensy
# while open
    # recv packet
    # if stream, unpack & print
    # if msg, print

using LibSerialPort

# Modify these as needed
# portname = "/dev/ttyS0"
portname = "COM4"



function echo(portname)
    baudrate = 115200 # teensy ignores baudrate
    LibSerialPort.open(portname, baudrate) do sp
        try
            while true
                sleep(0.01)
                if bytesavailable(sp) > 0
                    println(readline(sp))
                end
            end
        catch e
        end
    end
end