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
import subprocess

verbose = False
if (len(sys.argv) > 1) and (sys.argv[1] == '-v'):
    verbose = True

adoc_files = [
    'OpenVX_Specification.adoc',
    'vx_khr_buffer_aliasing.adoc',
    'vx_khr_class.adoc',
    'vx_khr_export_and_import.adoc',
    'vx_khr_feature_sets.adoc',
    'vx_khr_icd.adoc',
    'vx_khr_import_kernel.adoc',
    'vx_khr_nn.adoc',
    'vx_khr_opencl_interop.adoc',
    'vx_khr_pipelining.adoc',
    'vx_khr_s16.adoc',
    'vx_khr_tiling.adoc',
    'vx_khr_user_data_object.adoc',
    'vx_khr_xml.adoc',
]

h_files = [
    '../include/VX/vx.h',
    '../include/VX/vx_api.h',
    '../include/VX/vx_import.h',
    '../include/VX/vx_kernels.h',
    '../include/VX/vx_khr_buffer_aliasing.h',
    '../include/VX/vx_khr_class.h',
    '../include/VX/vx_khr_icd.h',
    '../include/VX/vx_khr_import_kernel.h',
    '../include/VX/vx_khr_ix.h',
    '../include/VX/vx_khr_nn.h',
    '../include/VX/vx_khr_opencl_interop.h',
    '../include/VX/vx_khr_pipelining.h',
    '../include/VX/vx_khr_tiling.h',
    '../include/VX/vx_khr_user_data_object.h',
    '../include/VX/vx_khr_xml.h',
    '../include/VX/vx_nodes.h',
    '../include/VX/vx_types.h',
    '../include/VX/vx_vendors.h',
    '../include/VX/vxu.h',
]

final_adecl = {}
final_hdecl = {}
origin_adecl = {}
origin_hdecl = {}

for adoc_file in adoc_files:
    if verbose:
        print('')
        print('=========== ADOC: {}'.format(adoc_file))

    for line in open(adoc_file,'r').readlines():
        line = line.strip()
        if line[:20] == 'include::api/protos/':
            adoc_api = line[20:-6]
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
                    elif dline == '...':
                        decl.append(['','...'])
            adecl = '{} {}({})'.format(decl[0], decl[1], ', '.join([' '.join(v) for v in decl[2:]]))
            adecl = '*'.join(adecl.split(' *'))
            adecl = '*'.join(adecl.split('* '))
            adecl = '**'.join(adecl.split('** '))
            if adoc_api in final_adecl:
                if final_adecl[adoc_api] != adecl:
                    print('ERROR: found decl(adco) mismatch for {}: {}'.format(adoc_api, adecl))
                if not (adoc_file in origin_adecl[adoc_api]):
                    origin_adecl[adoc_api].append(adoc_file)
            else:
                final_adecl[adoc_api] = adecl
                origin_adecl[adoc_api] = [adoc_file]

for hfile in h_files:
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
            else:
                line = line[12:].strip()
                items = line.split('VX_API_CALL')
                ret_type = items[0].strip()
                line = ' '.join(items[1:]).strip()
                items = line.split('(')
                h_api = items[0]
                decl = [ret_type, h_api]
                items = [v.strip() for v in ' '.join(items[1:]).split(',')]
                for item in items:
                    item = '* '.join(item.split(' *'))
                    ditems = item.split()
                    pname = ditems[-1]
                    ptype = ' '.join(ditems[:-1])
                    decl.append([ptype,pname])
                hdecl = '{} {}({})'.format(decl[0], decl[1], ', '.join([' '.join(v) for v in decl[2:]]))
                hdecl = '*'.join(hdecl.split(' *'))
                hdecl = '*'.join(hdecl.split('* '))
                hdecl = '**'.join(hdecl.split('** '))
                hdecl = '()'.join(hdecl.split('( void)'))
                if h_api in final_hdecl:
                    if final_hdecl[h_api] != hdecl:
                        print('ERROR: found decl(h) mismatch for {}: {}'.format(h_api, hdecl))
                    if not (hfile in origin_hdecl[h_api]):
                        origin_hdecl[h_api].append(hfile)
                else:
                    final_hdecl[h_api] = hdecl
                    origin_hdecl[h_api] = [hfile]

for api in sorted(final_adecl):
    if (api in final_hdecl) and (final_adecl[api] != final_hdecl[api]):
        print('')
        print('MISMATCH of {} between .adoc and .h'.format(api))
        print('<adoc< ' + final_adecl[api])
        print('>.h>>> ' + final_hdecl[api])
print('')
for api in sorted(origin_adecl):
    if not (api in origin_hdecl):
        print('MISSING from .h: "{}": {}'.format(api, ' '.join(origin_adecl[api])))
for api in sorted(origin_hdecl):
    if not (api in origin_adecl):
        print('MISSING from .adoc: "{}": {}'.format(api, ' '.join(origin_hdecl[api])))
print('')
for api in sorted(origin_adecl):
    if len(origin_adecl[api]) > 1:
        print('DUPLICATE: .adoc: "{}" in {}'.format(api, ' '.join(origin_adecl[api])))
for api in sorted(origin_hdecl):
    if len(origin_hdecl[api]) > 1:
        print('DUPLICATE: .h: "{}" in {}'.format(api, ' '.join(origin_hdecl[api])))
print('')

def inCts(a):
    cmd = "git --git-dir ../../cts/.git grep -q " + a + ""
    ret = subprocess.call(cmd, shell=True)
    if (ret) > 0:
        return False
    else:
        return True

for api in sorted(origin_adecl):
    if not (inCts(api)):
        print('MISSING from CTS: "{}": {}'.format(api, ' '.join(origin_adecl[api])))
