import os

def createIndexFile(startpath, indexFile):
    for root, dirs, files in os.walk(startpath):
        files = [f for f in files if not f[0] == '.']
        dirs[:] = [d for d in dirs if not d[0] == '.']
        level = root.replace(startpath, '').count(os.sep) - 1
        indent = ' ' * 2 * (level)
        directory = os.path.basename(root)
        if directory != '.':
            indexFile.write('{}- {}\n'.format(indent, os.path.basename(root)))
            subindent = ' ' * 2 * (level + 1)
            for f in files:
                indexFile.write('{}- [{}]({})\n'.format(subindent, f[:-3], f[:-3]))

indexFile = open('_Sidebar.md', 'w')
createIndexFile(".", indexFile)
indexFile.close();