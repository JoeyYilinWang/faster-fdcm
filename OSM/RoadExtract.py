# Copyright 2022, Yi-Lin Wang
# All Rights Reserved 
# Permission to use, copy, modify, and distribute this software and 
# its documentation for any non-commercial purpose is hereby granted 
# without fee, provided that the above copyright notice appear in 
# all copies and that both that copyright notice and this permission 
# notice appear in supporting documentation, and that the name of 
# the author not be used in advertising or publicity pertaining to 
# distribution of the software without specific, written prior 
# permission. 
# THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
# ANY PARTICULAR PURPOSE. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR 
# ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN 
# AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING 
# OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 

import osm2gmns as og
# OSM 子图区域，min_lat, min_longti, max_lat, max_longti within tuple
region = (31.22585, 121.46745, 31.22937, 121.47374) 
gps_center = ((31.22585+31.22937)/2, (121.46745+121.47374)/2)
# 然而为保证子图元素连续性，肯定会增加一些或删除一些元素，所以上述提供的区域也只是一个大概的范围
net = og.getNetFromOSMFile("/home/joey/Projects/OSM_Analysis/osmfile/上海.osm", network_types = ('auto'), POI=True, bbox=region)
og.outputNetToCSV(net, output_folder="output")
