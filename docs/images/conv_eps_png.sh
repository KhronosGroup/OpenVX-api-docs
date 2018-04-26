#!/bin/sh
# Convert pipelining extension images to SVG

for eps in \
    pipe_nopipelining \
    pipe_pipelining \
    pipe_soc \
    pipe_sourcesink \
    pipe_updown \
; do
    input=old/${eps}.eps
    output=${eps}.svg
    inkscape --export-plain-svg $output $input
done

# Files only present in PNG form
for png in graph_batch_processing \
; do
    convert old/${png}.png ${png}.pnm
    potrace ${png}.pnm -s -o ${png}.svg
    rm ${png}.pnm
done
