#include "Element.h"
#include "ImageDraw.h"
// test OSM nodes rendering unit
int main(int argc,char *argv[])
{
    nodes Nodes;
    vector<vector<string>> data;

    // test if nodes object function correctly, test correctly
    Nodes.ReadFromCSV("/home/joey/Projects/OSM_Analysis/output/node.csv", data);
    Nodes.Extract(data);

    // test if links object function correctly
    data.clear();
    links Links;
    Links.ReadFromCSV("/home/joey/Projects/OSM_Analysis/output/link.csv", data);
    Links.Extract(data);
    
    Nodes.LongLatHeight2ENU();
    cout << Nodes.nodes[1].E_coord << endl << Nodes.nodes[1].N_coord << endl << Nodes.nodes[1].U_coord << endl;

    
    return 0;
}