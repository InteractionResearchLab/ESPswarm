import os
import sys


template = open("template.txt")
print template
count = 0

for line in template:
    print line,
    count += 1

print "read ",  count,  " lines" 