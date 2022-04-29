using CSV
using GLMakie

file = CSV.File("data/data_5.txt")
lines(file.time, file.EMG1)

