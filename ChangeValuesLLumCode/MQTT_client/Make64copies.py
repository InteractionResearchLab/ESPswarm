import sys
import os
def FindAndReplaceLines():

    for line in code:
        #print"itterating"
    
        if line.strip() == 'const char* assigned_id = "01";':
                outputFile.write('%s' %"".join(modfiliedChangeID))
                print "Modified ChangeID"
        
        elif line.strip() == 'const char* id = "ESP01";':
            outputFile.write('%s' % "".join(modfiliedresetID))
            print "Modified resetID"
        else:
            outputFile.write(line)
        # print "Not got it"


def editLines(val):
    modfiliedChangeID[27:29] = '%s' %val
    #print modfiliedChangeID
    print "".join(modfiliedChangeID),
    
    modfiliedresetID[21:23] = '%s' %val
    #print modfiliedChangeID
    print "".join(modfiliedresetID)



print "This is the name of the script: ", sys.argv[0]
print "Number of arguments: ", len(sys.argv)
print "The arguments are: " , str(sys.argv)


code = open(sys.argv[1],"r")

print code
print "Script change touch lines"
count = 0 

for line in code:
    count += 1
    if count == 11:
        #print line
        changeID = line
    if count == 14:
        #print line
        resetID = line

print ""
print "Read ", count , "  lines."
print "changeID = %s" % changeID,
print "resetID = %s"% resetID

print "Values To edit:"
print changeID[27:29] 
print resetID[21:23]

print ""
print "OriginalStrings:"
print changeID,
print resetID
print ""
print ""


print "EditedStrings:"
modfiliedChangeID = list(changeID)
modfiliedresetID = list(resetID)
editLines(15)

code.close()
#close and open file again to start from the beginning of file

code = open(sys.argv[1],"r")
outputFile = open('_PreviewFile.txt','w')

if not os.path.exists("OutputFiles"):
    os.makedirs("OutputFiles")

FindAndReplaceLines()

code.close()
outputFile.close()


for i in range(1,65):
    fileNumber = "%02d" % (i)
    outputFile = open('OutputFiles/Output%s.ino'% fileNumber,'w')
    code = open(sys.argv[1],"r")
    editLines(fileNumber)
    FindAndReplaceLines()
    code.close()
    outputFile.close()



