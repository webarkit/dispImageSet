#include <iostream>
#include <AR/ar.h>
#include <AR2/config.h>
#include <AR2/imageFormat.h>
#include <AR2/imageSet.h>
#include <AR2/tracking.h>
#include <AR2/util.h>
#include <KPM/kpm.h>
#include <emscripten.h>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <vector>

struct arIset {
  int id = 0;
  AR2ImageSetT *imageSet = NULL;
  int numIset;
};

std::unordered_map<int, arIset> arIsets;

static int ARISET_NOT_FOUND = -1;
static int gARIsetID = 0;

extern "C" {

void create_emscripten_canvas(int id, int width, int height, unsigned char* texData)
{
  EM_ASM_({
    var canvas = document.createElement("canvas");
    var idx = $0;
    var canvasId = "canvas" + idx;
    canvas.setAttribute("id", canvasId);
    console.log("canvas id: ", canvasId);
    var ctx = canvas.getContext("2d");
    var width = $1;
    var height = $2;
    console.log("canvas width: ", width);
    console.log("canvas height: ", height);
    ctx.canvas.width = width;
    ctx.canvas.height = height;
    var Buffer = new Uint8ClampedArray(
      Module.HEAPU8.buffer,
      $3,
      width*height*4
    );
    var id = new ImageData(
      new Uint8ClampedArray(canvas.width * canvas.height * 4),
      ctx.canvas.width,
      ctx.canvas.height
    );
    console.log("Data: ", Buffer);
    console.log("Data length: ", Buffer.length);
    for (var i = 0, j = 0; i < Buffer.length; i++, j += 4) {
      var v = Buffer[i];
      id.data[j + 0] = v;
      id.data[j + 1] = v;
      id.data[j + 2] = v;
      id.data[j + 3] = 255;
    };
    ctx.putImageData(id, 0, 0);
    document.body.appendChild(canvas);
    Module._free(Buffer);
  },
  id,
  width,
  height,
  texData
  );
}

int loadNFTMarker(arIset *arc, const char *filename) {
  ARLOGi("Read ImageSet.\n");
  ar2UtilRemoveExt((char *)filename);
  arc->imageSet = ar2ReadImageSet((char *)filename);
  if (arc->imageSet == NULL) {
    ARLOGe("file open error: %s.iset\n", filename);
    exit(0);
  }
  ARLOGi("  end.\n");

  arc->numIset = arc->imageSet->num;

  ARLOGi("Loading of NFT data complete.\n");
  return (TRUE);
}

bool readNFTMarker(int id, std::string datasetPathname) {
  if (arIsets.find(id) == arIsets.end()) {
    return false;
  }
  arIset *arc = &(arIsets[id]);

  // Load marker(s).
  if (!loadNFTMarker(arc, datasetPathname.c_str())) {
    ARLOGe("ARimageFsetDisplay(): Unable to read NFT marker.\n");
    return false;
  } else {
    ARLOGi("Creating all the canvases...\n");
  }

  int numIset = arc->numIset;
  for(int i = 0; i < numIset; i++){
    create_emscripten_canvas(i, arc->imageSet->scale[i]->xsize, arc->imageSet->scale[i]->ysize, arc->imageSet->scale[i]->imgBW);
  }

  ARLOGi("  Done.\n");

  return true;
}

int setup() {
  int id = gARIsetID++;
  arIset *arc = &(arIsets[id]);
  arc->id = id;
  return arc->id;
}
}

#include "bindings.cpp"
