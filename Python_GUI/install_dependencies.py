import subprocess
import sys
def install(package):
    subprocess.check_call([sys.executable, '-m','pip','install', package])

with open('requirements.txt') as f:
    for line in f:
        install(line.strip())
