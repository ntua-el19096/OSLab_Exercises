import os
os.dup2(1,99)
os.execv("./riddle",["riddle"])
