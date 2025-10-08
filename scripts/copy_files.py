import os
import sys
import shutil

sourcePath = sys.argv[1] + "\\"
destPath = sys.argv[2] + "\\"

def copyFile(sourceSubpath, filename):
    if os.path.isfile(destPath + filename):
        os.remove(destPath + filename)

    if not os.path.isfile(sourcePath + sourceSubpath + filename):
        print("File " + filename + " not found in " + sourcePath + sourceSubpath)
        return

    shutil.copyfile(sourcePath + sourceSubpath + filename, destPath + filename)

copyFile("src\\gpilot\\", "GPilot.exe")
copyFile("src\\vendor\\uCNC\\", "uCNC.dll")
copyFile("src\\vendor\\PropertyEditor\\", "PropertyEditor.dll")
