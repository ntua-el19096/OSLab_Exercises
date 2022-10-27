import os

fds = os.pipe()+os.pipe()

os.dup2(fds[0],33)
os.dup2(fds[1],34)
os.dup2(fds[2],53)
os.dup2(fds[3],54)

os.execv("./riddle",["riddle"])