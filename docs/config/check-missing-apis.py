# Copyright (c) 2019 The Khronos Group Inc.
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
# MATCH APIs IN ASCIIDOC FILES AGAINST ITS DECLARATION IN HEADER FILES
#
# Usage: python config/check-missing-apis.py
#
# Author: Radhakrishna Giduthuri, Intel
##########################################################################

import sys
import glob

verbose = False
if (len(sys.argv) > 1) and (sys.argv[1] == '-v'):
    verbose = True

for adoc_file in glob.glob('*.adoc'):
    if verbose:
        print('')
        print('=========== ADOC: {}'.format(adoc_file))
    
    adoc_apis = []
    adoc_api_decl = {}
    for line in open(adoc_file,'r').readlines():
        line = line.strip()
        if line[:20] == 'include::api/protos/':
            adoc_api = line[20:-6]
            adoc_apis.append(adoc_api)
            decl = []
            state = 0
            for dline in open(line[9:-2],'r').readlines():
                dline = ''.join(dline.split('NOAPI'))
                if (state == 0) and (dline[:4] == '----'):
                    state = 1
                elif (state == 1):
                    state = 2
                    items = dline.strip().split('(')[0].split()
                    ret_type = items[0]
                    api_name = items[1]
                    decl.append(ret_type)
                    decl.append(api_name)
                elif (state == 2) and (dline[:4] == '----'):
                    state = 3
                elif (state == 2):
                    dline = dline.strip()
                    if dline[-1] == ',':
                        dline = dline[:-1]
                    elif dline[-2:] == ');':
                        dline = dline[:-2]
                    ditems = dline.split()
                    if len(ditems) >= 2:
                        pname = ditems[-1]
                        ptype = ' '.join(ditems[:-1])
                        decl.append([ptype,pname])
            adoc_api_decl[adoc_api] = decl
    
    h_apis_total = []
    for hfile in glob.glob('../include/VX/*.h'):
        h_apis = []
        hstate = 0
        pline = ''
        for line in open(hfile,'r').readlines():
            line = pline + line.strip()
            if line[:12] == 'VX_API_ENTRY':
                if not (');' in line):
                    pline = line.strip() + ' '
                    hstate = 1
                    continue
                line = line[:-2]
                hstate = 0
                pline = ''
                if not ('VX_API_CALL' in line):
                    print('ERROR: missing VX_API_CALL in {} of {}'.format(line, hfile))
                h_api = ''
                h_api_count = 0
                for api in adoc_apis:
                    if api+'(' in line:
                        h_api = api
                        h_api_count += 1
                if h_api_count > 1:
                    if verbose:
                        print('***> {}'.format(h_api, hfile))
                if h_api_count >= 1:
                    h_apis.append(h_api)
                    line = line[12:].strip()
                    items = line.split('VX_API_CALL')
                    ret_type = items[0].strip()
                    decl = [ret_type, h_api]
                    line = ' '.join(items[1:]).strip()
                    items = line.split('(')
                    func = items[0]
                    if func != h_api:
                        print('********* {}|{}|{}'.format(func, h_api, hfile), adoc_api_decl[h_api], line)
                    items = [v.strip() for v in ' '.join(items[1:]).split(',')]
                    for item in items:
                        item = '* '.join(item.split(' *'))
                        ditems = item.split()
                        pname = ditems[-1]
                        ptype = ' '.join(ditems[:-1])
                        decl.append([ptype,pname])
                    adecl = adoc_api_decl[h_api]
                    adecl = '{} {}({})'.format(adecl[0], adecl[1], ', '.join([' '.join(v) for v in adecl[2:]]))
                    hdecl = '{} {}({})'.format(decl[0], decl[1], ', '.join([' '.join(v) for v in decl[2:]]))
                    hdecl = '*'.join(hdecl.split(' *'))
                    adecl = '*'.join(adecl.split(' *'))
                    hdecl = '*'.join(hdecl.split('* '))
                    adecl = '*'.join(adecl.split('* '))
                    hdecl = '**'.join(hdecl.split('** '))
                    adecl = '**'.join(adecl.split('** '))
                    #hdecl = ''.join(hdecl.split('enum '))
                    if adecl != hdecl:
                        print('')
                        print('== {} in {}'.format(h_api, hfile))
                        print('<adoc< ' + adecl)
                        print('>.h>>> ' + hdecl)
        if len(h_apis) > 0:
            h_apis_total = h_apis_total + h_apis
            if verbose:
                print('%4d %s' % (len(h_apis), hfile))
    
    if verbose:
        print('%4d == %d == count' % (len(h_apis_total), len(adoc_apis)))
        print('%4d == %d == unique' % (len(set(h_apis_total)), len(set(adoc_apis))))
    
    missing_apis = []
    for api in adoc_apis:
        if not (api in sorted(h_apis_total)):
            missing_apis.append(api)
    if len(missing_apis) > 0:
        print('')
        print('{}: MISSING APIs in header files'.format(adoc_file))
        for api in missing_apis:
            print('    ' + api)
            print(adoc_api_decl[api])
print('')
