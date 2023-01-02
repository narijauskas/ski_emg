using CSV
using GLMakie, CairoMakie

## ------------- build plot ------------- ##
GLMakie.activate!() # switch plotting backend

# auto-name header, see https://csv.juliadata.org/stable/reading.html#header for more info
file = CSV.File("leo_data/data_7.txt", header = false) 

# isolate time data
time = file.Column1./1e6

# explicitly build figure/axis objects
fig = Figure(); ax = Axis(fig[1,1])

# plot everything except time as a funtion of time into the axis
for name in filter(!isequal(:Column1), file.names)
    lines!(ax, time, file[name])
end
# ylims!()

display(current_figure())


## ------------- save high-quality ------------- ##
CairoMakie.activate!() # switch plotting backend
save("leo_data_5.png", current_figure())

