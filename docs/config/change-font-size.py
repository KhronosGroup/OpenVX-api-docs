import re

fontsize = re.compile('.*font-size: ([0-9.]*)em;')
with open('config/archive/khronos.css') as fp:
    lines = fp.readlines()
text = ''
for line in lines:
    result = fontsize.match(line)
    if result:
        oldsize = result.group(1)
        newsize = str(float(oldsize)*10.0/10.5)
        line = ('font-size: ' + newsize + 'em;').join(line.split('font-size: ' + oldsize + 'em;'))
    text = text + line
with open('config/khronos.css','w') as fp:
    fp.write(text)
