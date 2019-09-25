# Copyright (c) 2014-2018 The Khronos Group Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

##########################################################################
# REQUIREMENTS TAG-ID GENERATOR FOR SPECIFICATIONS IN ASCIIDOCTOR
#
# Usage: python spec-tags.py [-help] [-digits 4|n] \
#          check|update|list|remove|reset spec.adoc [output.adoc]
#
# Use -help option for full detailed help.
#
# Author: Radhakrishna Giduthuri, Intel
##########################################################################

usage = """
REQUIREMENTS TAG-ID GENERATOR FOR SPECIFICATIONS IN ASCIIDOCTOR

Usage: python spec-tags.py [-help] \\
         [-digits 4|n] check|update|list|remove|reset spec.adoc [output.adoc]
"""

help = usage + """
  All the implementation requirements imposed by a specification can be
  tagged using unique identifiers of the form `[*REQ-#*]`, where # is a
  number. These can be used to help maintain requirement traceability
  throughout the implementation development process to assure that they are
  documented and verified.

  The requirements which persist across specification versions will be
  retained with the same requirement identifier. New specification versions
  may add or remove requirements, therefore the tags in a specification
  may not be in numerical order.

  To insert a requirement tag into the specification, insert below text marker
  into text without double-quotes (don't forget to add space before the tag):
      " `[*REQ*]`"
  This marks a tag without numeric ID. Use this script to automatically
  generate a unique requirement tag ID of the form `[*REQ-#*]` to look
  like [REQ-1234] anywhere in the document, where # is a number. The generated
  requirement tag IDs can be cross-referred using REQ-# name, where #
  is the unique number. For example, the requirement ID [REQ-1234] in a
  specification can be cross-referenced using "spec.html#REQ-1234" in the
  specification HTML.

  This scripts keeps track of all unique numbers already present in the document
  before generating unique numbers for new requirement tags. The unique
  requirement tag numbers will start from 1 and will be increment from the last
  unique number generated. The script keeps track of the last unique number by
  appending the document with "//[*CNT-#*]" marker at the end, where # is the
  last unique number.

  The script also takes number of digits in the generated requirement tag as
  an argument, with 4 digits as default when not specified.


OPTIONS:
  -digits n      Number of digits in generated requirement tags (default 4)

  -help          Print this message

COMMANDS:
  check          Check the consistency of requirement tags in the document.

  update         Replace `[*REQ*]` tags in the document with `[*REQ-#*]`
                 where # is a unique number.

  list           Show list of unique requirement IDs and corresponding line
                 number in the ASCIIDOCTOR specification document. Section
                 heading can be extracted from generated HTML output by
                 specifying HTML file as optional parameter after the input
                 ASCIIDOCTOR file.

  remove         Remove all requirement tags from the specification document to
                 support specification build without any requirement tags.

  reset          Replace all uniquely identified requirement tags with `[*REQ*]`
                 text. This is a feature to debug this script.

EXAMPLE:
  % python spec-tags.py update OpenVX_Specification.adoc
"""

import sys

def TagsAreValid(lines):
    return True

##
# get command-line arguments
#
tagIdDigits = 4
pos = 1
while len(sys.argv) >= (pos+1) and sys.argv[pos][0] == '-':
    if sys.argv[pos] == '-digits' and len(sys.argv) >= (pos+2):
        try:
            tagIdDigits = int(sys.argv[pos+1])
            pos = pos+2
        except:
            print('ERROR: %s is not a invalid number: ' % (sys.argv[pos+1]))
            print(usage)
            exit(1)
    elif sys.argv[pos] == '-help':
        print(help)
        exit(1)
    else:
        print('ERROR: %s is not a valid option' % (sys.argv[pos]))
        print(usage)
        exit(1)
if len(sys.argv) < (pos+2) or len(sys.argv) > (pos+3):
    print(usage)
    exit(1)
cmd = sys.argv[pos]
fileName = sys.argv[pos+1]
if cmd == 'list':
    fileNameSave = fileName
    fileNameHtml = sys.argv[pos+2] if len(sys.argv) > (pos+2) else None
else:
    fileNameSave = sys.argv[pos+2] if len(sys.argv) > (pos+2) else fileName
    fileNameHtml = None
if not cmd in ['check','update','list','remove','reset']:
    print('ERROR: %s is not a valid command' % (cmd))
    print(usage)
    exit(1)

##
# read lines
#
try:
    with open(fileName) as fp:
        lines = fp.readlines()
except:
    print('Unable to open', fileName)
    exit(1)

##
# check validity
#
newTagCount = 0
tagIds = []
tagIdsLine = []
tagIdsCount = 0
tagIdsUpdateNeeded = False
foundError = False
posInText = 0
for index, line in enumerate(lines):
    if line[:8] == '//[*CNT-':
        tagIdsCount = int(line[8:].split('*')[0])
        lines[index] = ''
        tagIdsUpdateNeeded = True
    taggaps = line.split(' `[*REQ')
    invgaps = ''.join(taggaps).split('[*REQ')
    itemList = line.split(' `[*REQ')
    posInLine = len(itemList[0])
    for item in itemList[1:]:
        posInLine = posInLine + len(item)
        idLen = len(item.split('*]`')[0])
        if item[:3] == '*]`':
            newTagCount = newTagCount + 1
        elif len(item) >= 5 and item[:5] == '-#*]`':
            pass
        elif item[0] == '-' and '*]`' in item:
            try:
                id = int(item[1:idLen])
                if id in tagIds:
                    print('ERROR: line %d: found duplicate tag: `[*REQ%s' % (index+1, item[:idLen]))
                    foundError = True
                else:
                    tagIds.append(id)
                    tagIdsLine.append((index, posInText + posInLine - len(item)))
            except:
                print('ERROR: line %d: found invalid tag syntax: `[*REQ%s' % (index+1, item[:idLen]))
                foundError = True
        else:
            print('ERROR: line %d: found invalid tag syntax: `[*REQ%s' % (index+1, item[:idLen]))
            foundError = True
    if len(invgaps) > 1:
        print('ERROR: line %d: found invalid tag syntax: missing space and/or `' % (index+1))
        foundError = True
    posInText = posInText + len(line)
if foundError:
    print('INFO:  valid tag syntax must be " `[*REQ*]`" or " `[*REQ-#*]`" without double-quotes')
    exit(1)
if newTagCount > 0:
    print('OK: found %d tags WITHOUT IDs' % (newTagCount))
else:
    print('OK: no tags found WITHOUT IDs')
lastID = 0
if len(tagIds) > 0:
    lastID = max(tagIds)
    print('OK: found %d tag IDs [%d..%d]' % (len(tagIds), min(tagIds), max(tagIds)))
    if len(tagIds) != max(tagIds):
        print('WARNING: found gaps in tag IDs list')
else:
    print('OK: no tag IDs found')
lastID = max(lastID,tagIdsCount)
if tagIdsCount != lastID + newTagCount:
    tagIdsCount = lastID + newTagCount
    tagIdsUpdateNeeded = True
if len(str(tagIdsCount)) > tagIdDigits:
    print('ERROR: number of digits(%d) is not sufficient to represent %d tags' % (tagIdDigits, tagIdsCount))
    exit(1)
print('OK: total %d tag IDs detected with %d tag IDs in all version' % (len(tagIds)+newTagCount, tagIdsCount))

##
# execute commands
#
updated = False
text = ''.join(lines)
if cmd == 'update':
    taggaps = text.split(' `[*REQ*]`')
    if len(taggaps) > 1:
        text = taggaps[0]
        for item in taggaps[1:]:
            lastID = lastID + 1
            id = (' `[*REQ-%%0%dd*]`[[REQ-%%0%dd]]' % (tagIdDigits, tagIdDigits)) % (lastID, lastID)
            text = text + id + item
        updated = True
        tagIdsUpdateNeeded = True
        print('OK: added IDs to %d tags' % (newTagCount))
    if tagIdsUpdateNeeded:
        updated = True
elif cmd == 'list':
    if len(tagIds) > 0:
        htmlReq = {}
        heading = ''
        if fileNameHtml:
            with open(fileNameHtml) as fp:
                html = fp.readlines()
                for line in html:
                    if '</h1>' in line or '</h2>' in line or '</h3>' in line:
                        heading = line.split('>')[-2][:-4]
                    elif '<code>[<strong>REQ-' in line:
                        reqList = line.split('<code>[<strong>REQ-')
                        for req in reqList[1:]:
                            try:
                                id = int(req.split('</strong>]</code>')[0])
                                htmlReq[id] = heading
                            except:
                                pass
        tagIds.sort()
        digitsLine = max(5, len(str(len(lines))))
        print(('  REQ-TAG# %%%ds %s' % (digitsLine, 'SECTION(html)' if fileNameHtml else '')) % ('LINE#'))
        for i in range(len(tagIds)):
            heading = ''
            if tagIds[i] in htmlReq:
                heading = htmlReq[tagIds[i]]
            print(('  REQ-%%0%dd %%%dd %%s' % (tagIdDigits, digitsLine)) % (tagIds[i], tagIdsLine[i][0]+1, heading))
elif cmd == 'reset':
    taggaps = text.split(' `[*REQ-')
    if len(taggaps) > 1:
        text = taggaps[0]
        for item in taggaps[1:]:
            itemEndSplit = item.split('*]`')
            if item[:4] == '#*]`':
                text = text + ' `[*REQ-#*]`' + item[4:]
            elif len(itemEndSplit) > 1:
                rest = ''
                for itemidx, itemref in enumerate(itemEndSplit):
                    if itemref[:6] == '[[REQ-':
                        itemEndSplit[itemidx] = ']]'.join(itemref.split(']]')[1:])
                text = text + ' `[*REQ*]`' + '*]`'.join(itemEndSplit[1:])
                updated = True
            else:
                print('ERROR: found invalid error tag: `[*REQ' + item[:1+tagIdDigits+3])
                exit(1)
        if updated:
            print('OK: removed IDs from %d tags' % (len(tagIds)))
    tagIdsUpdateNeeded = False
    lastID = 0
elif cmd == 'remove':
    taggaps = text.split(' `[*REQ')
    if len(taggaps) > 1:
        text = taggaps[0]
        for item in taggaps[1:]:
            itemEndSplit = item.split('*]`')
            if item[:4] == '#*]`':
                text = text + ' `[*REQ-#*]`' + item[4:]
            elif len(itemEndSplit) > 1:
                rest = ''
                for itemidx, itemref in enumerate(itemEndSplit):
                    if itemref[:6] == '[[REQ-':
                        itemEndSplit[itemidx] = ']]'.join(itemref.split(']]')[1:])
                text = text + '*]`'.join(itemEndSplit[1:])
                updated = True
            else:
                print('ERROR: found invalid error tag: `[*REQ' + item[:1+tagIdDigits+3])
                exit(1)
        print('OK: removed %d tags' % (len(tagIds) + newTagCount))
        updated = True
    tagIdsUpdateNeeded = False
    lastID = 0

##
# add tagId count at the end
#
if tagIdsUpdateNeeded:
    text = text + ('//[*CNT-%d*]' % (tagIdsCount)) + '\n'

##
# write the output
#
if updated or fileName != fileNameSave:
    with open(fileNameSave,'w') as fp:
        fp.write(text)
    print("OK: updated '%s'" % (fileNameSave))
