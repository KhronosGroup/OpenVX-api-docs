#!/usr/bin/python3
# sortreg - sort a Registry XML file's <command> and <extension> tags on
# name, generating a new XML file on stdout. The new file will also
# be canonicalized,

import copy, sys, time, pdb, string, cProfile
from reg import *

# Return extension name attribute of an <extension> element
def extensionSortKey(elem):
    return elem.get('name')

# Return command name tag of a <command> element
def commandSortKey(elem):
    name = elem.find('proto/name')
    if (name == None):
        print('No key text for proto/name in element: ', elem.text, file=sys.stderr)
        return ''
    else:
        return name.text

# root - Element for root <registry> tag
# groupTag - name of group tag, '<commands>' or '<extensions>'
# sortKeyProc - procedure returning sort key for a child
#   <command> or <extension> element being sorted
def subsort(root, groupTag, sortKeyProc):
    # Find <extensions> element  to replace
    i = list(root).index(root.find(groupTag))
    # Can't sort in place because the element isn't actually a list,
    # so replace it with a new, empty <groupTag> element.
    oldexts = root[i]
    newexts = etree.Element(groupTag)
    # Copy attributes from old <groupTag> element
    for key in oldexts.attrib:
        newexts.attrib[key] = oldexts.attrib[key]
    # Copy whitespace from old <groupTag> element
    newexts.text = copy.deepcopy(oldexts.text)
    newexts.tail = copy.deepcopy(oldexts.tail)
    # Replace old tag with new
    root[i] = newexts

    # Build list of child elements from old element
    extlist = [elem for elem in oldexts]

    extlist.sort(key = sortKeyProc)

    # If the last element in the original list has moved, swap its
    # white space with the new last element
    if (oldexts[-1] != extlist[-1]):
        # Text
        swap = copy.deepcopy(oldexts[-1].text)
        oldexts[-1].text = copy.deepcopy(extlist[-1].text)
        extlist[-1].text = swap
        # Tail
        swap = copy.deepcopy(oldexts[-1].tail)
        oldexts[-1].tail = copy.deepcopy(extlist[-1].tail)
        extlist[-1].tail = swap

    # Now add sorted list elements into the new <groupTag> element
    for elem in extlist:
        newexts.append(elem)

# tree - lxml.etree Element
# fp - file handle
def printTree(tree, fp):
    print('<?xml version="1.0" encoding="UTF-8"?>', file=fp)
    print(etree.tostring(tree).decode('utf-8'), file=fp)

# Sort XML file, writing to sys.stdout
def sortXML(filename):
    # Load & parse registry
    tree = etree.parse(filename)
    #reg.loadElementTree(tree)
    root = tree.getroot()
    # Sort
    subsort(root,'extensions',extensionSortKey)
    subsort(root,'commands',commandSortKey)
    # Save
    printTree(root, sys.stdout)

if __name__ == '__main__':
    if (len(sys.argv) > 1):
        sortXML(sys.argv[1])
    else:
        print("No file to process", file=sys.stderr)


