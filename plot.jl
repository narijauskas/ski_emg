using CSV
using GLMakie

# data_8:
# shift weight, 3x calf raise, 3x squat, 3x bend leg, 3x jump

file = CSV.File("data/data_8.txt")
lines(file.time, file.EMG1)
lines!(file.time, file.EMG2)
lines!(file.time, file.EMG3)
lines!(file.time, file.EMG4)
lines!(file.time, file.EMG5)
lines!(file.time, file.EMG6)
lines!(file.time, file.EMG7)
display(current_figure())


## save high-quality
using CairoMakie
CairoMakie.activate!()
save("data_8.svg", current_figure())

