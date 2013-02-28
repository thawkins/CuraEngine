#ifndef LAYERPART_H
#define LAYERPART_H

/*
The layer-part creation step is the first step in creating actual useful data for 3D printing.
It takes the result of the Slice step, which is an unordered list of polygons, and makes groups of polygons,
each of these groups is called a "part", which sometimes are also known as "islands". These parts represent
isolated areas in the 2D layer with possible holes.

Creating "parts" is an important step, as all elements in a single part should be printed before going to another part.
And all every bit inside a single part can be printed without the nozzle leaving the boundery of this part.

It's also the first step that stores the result in the "data storage" so all other steps can access it.
*/


void createLayerWithParts(SliceLayer& storageLayer, SlicerLayer* layer)
{
    ClipperLib::Polygons polyList;
    for(unsigned int i=0; i<layer->polygonList.size(); i++)
    {
        ClipperLib::Polygon p;
        p.push_back(layer->polygonList[i].points[0]);
        unsigned int prev = 0;
        for(unsigned int j=1; j<layer->polygonList[i].points.size(); j++)
        {
            if (shorterThen(layer->polygonList[i].points[j] - layer->polygonList[i].points[prev], 200))
                continue;
            p.push_back(layer->polygonList[i].points[j]);
            prev = j;
        }
        polyList.push_back(p);
    }
    
    ClipperLib::ExPolygons resultPolys;
    ClipperLib::Clipper clipper;
    clipper.AddPolygons(polyList, ClipperLib::ptSubject);
    clipper.Execute(ClipperLib::ctUnion, resultPolys);
    
    for(unsigned int i=0; i<resultPolys.size(); i++)
    {
        storageLayer.parts.push_back(SliceLayerPart());
        
        storageLayer.parts[i].outline.push_back(resultPolys[i].outer);
        for(unsigned int j=0; j<resultPolys[i].holes.size(); j++)
        {
            storageLayer.parts[i].outline.push_back(resultPolys[i].holes[j]);
        }
    }
}

void createLayerParts(SliceDataStorage& storage, Slicer* slicer)
{
    for(unsigned int layerNr = 0; layerNr < slicer->layers.size(); layerNr++)
    {
        storage.layers.push_back(SliceLayer());
        createLayerWithParts(storage.layers[layerNr], &slicer->layers[layerNr]);
        //LayerPartsLayer(&slicer->layers[layerNr])
    }
}

void dumpLayerparts(SliceDataStorage& storage, const char* filename)
{
    FILE* out = fopen(filename, "w");
    fprintf(out, "<!DOCTYPE html><html><body>");
    Point3 modelSize = storage.modelSize;
    
    for(unsigned int layerNr=0;layerNr<storage.layers.size(); layerNr++)
    {
        fprintf(out, "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" style=\"width: 150px; height:150px\">\n");
        SliceLayer* layer = &storage.layers[layerNr];
        for(unsigned int i=0;i<layer->parts.size();i++)
        {
            SliceLayerPart* part = &layer->parts[i];
            for(unsigned int j=0;j<part->outline.size();j++)
            {
                fprintf(out, "<polygon points=\"");
                for(unsigned int k=0;k<part->outline[j].size();k++)
                {
                    fprintf(out, "%f,%f ", float(part->outline[j][k].X + modelSize.x/2)/modelSize.x*150, float(part->outline[j][k].Y + modelSize.y/2)/modelSize.y*150);
                }
                if (j == 0)
                    fprintf(out, "\" style=\"fill:gray; stroke:black;stroke-width:1\" />\n");
                else
                    fprintf(out, "\" style=\"fill:red; stroke:black;stroke-width:1\" />\n");
            }
        }
        fprintf(out, "</svg>\n");
    }
    fprintf(out, "</body></html>");
    fclose(out);
}

void dumpLayer(FILE* out, Point3 modelSize)
{
}

#endif//LAYERPART_H