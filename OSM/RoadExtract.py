import osm2gmns as og
net = og.getNetFromOSMFile("osmfile/上海.osm", network_types = ('auto'), POI=True)
og.outputNetToCSV(net, output_folder="output")
