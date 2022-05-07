#include <iostream>
#include <AR/ar.h>
#include <AR2/config.h>
#include <AR2/imageFormat.h>
#include <AR2/imageSet.h>
#include <AR2/tracking.h>
#include <AR2/util.h>
#include <GLES2/gl2.h>
#include <KPM/kpm.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/val.h>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <vector>

#define PAGES_MAX  10 // Maximum number of pages expected. You can change this down (to save
                      // memory) or up (to accomodate more pages.)

struct nftMarker {
  int numIset;
  int widthNFT;
  int heightNFT;
  int dpiNFT;
  int imgBWsize;
  int pointer;
};

struct arIset {
  int id = 0;
  int width = 0;
  int height = 0;
  ARUint8 *imgBW = NULL;
  int imgBWsize;
  AR2ImageSetT *imageSet = NULL;
  int numIset;
  int width_NFT;
  int height_NFT;
  int dpi_NFT;
  int surfaceSetCount = 0; // Running NFT marker id
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
    canvas.id = canvasId;
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
      canvas.width,
      canvas.height
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
  },
  id,
  width,
  height,
  texData
  );
}

int loadNFTMarker(arIset *arc, int surfaceSetCount, int numImage,
                  const char *filename) {
  ARLOG("Read ImageSet.\n");
  ARLOGi("numImage is: %d\n", numImage);
  ar2UtilRemoveExt((char *)filename);
  arc->imageSet = ar2ReadImageSet((char *)filename);
  if (arc->imageSet == NULL) {
    ARLOGe("file open error: %s.iset\n", filename);
    exit(0);
  }
  ARLOG("  end.\n");

  arc->numIset = arc->imageSet->num;
  arc->width_NFT = arc->imageSet->scale[numImage]->xsize;
  arc->height_NFT = arc->imageSet->scale[numImage]->ysize;
  arc->dpi_NFT = arc->imageSet->scale[numImage]->dpi;
  arc->imgBW = arc->imageSet->scale[numImage]->imgBW;

  ARLOGi("printing pointer imgBW: %d\n", arc->imgBW);

  ARLOGi("NFT number of ImageSet: %i\n", arc->numIset);
  ARLOGi("NFT marker width: %i\n", arc->width_NFT);
  ARLOGi("NFT marker height: %i\n", arc->height_NFT);
  ARLOGi("NFT marker dpi: %i\n", arc->dpi_NFT);

  ARLOGi("imgBW filled\n");

  ARLOGi("  Done.\n");

  ARLOGi("imgsizePointer: %d\n", arc->imgBWsize);

  ARLOGi("Loading of NFT data complete.\n");
  return (TRUE);
}

nftMarker readNFTMarker(int id, int numImage, std::string datasetPathname) {
  nftMarker nft;
  if (arIsets.find(id) == arIsets.end()) {
    return nft;
  }
  arIset *arc = &(arIsets[id]);

  // Load marker(s).
  int patt_id = arc->surfaceSetCount;
  if (!loadNFTMarker(arc, patt_id, numImage, datasetPathname.c_str())) {
    ARLOGe("ARimageFsetDisplay(): Unable to read NFT marker.\n");
    return nft;
  } else {
    ARLOGi("Passing the imgBW pointer: %d\n", (int)arc->imgBW);
  }

  arc->surfaceSetCount++;
  int numIset = arc->numIset;
  for(int i = 0; i < numIset; i++){
    create_emscripten_canvas(i, arc->imageSet->scale[i]->xsize, arc->imageSet->scale[i]->ysize, arc->imageSet->scale[i]->imgBW);
  }

  EM_ASM_(
      {
        if (!ariset["frameMalloc"]) {
          ariset["frameMalloc"] = ({});
        }
        var frameMalloc = ariset["frameMalloc"];
        frameMalloc["frameIbwpointer"] = $1;
        frameMalloc["frameimgBWsize"] = $2;
      },
      arc->id, arc->imgBW, arc->imgBWsize);

  nft.numIset = arc->numIset;
  nft.widthNFT = arc->width_NFT;
  nft.heightNFT = arc->height_NFT;
  nft.dpiNFT = arc->dpi_NFT;
  nft.imgBWsize = arc->imgBWsize;
  nft.pointer = (int)arc->imgBW;

  return nft;
}

int setup(int width, int height) {
  int id = gARIsetID++;
  arIset *arc = &(arIsets[id]);
  arc->id = id;
  arc->imgBWsize = width * height * 4 * sizeof(ARUint8);
  arc->imgBW = (ARUint8 *)malloc(arc->imgBWsize);

  ARLOGi("Allocated imgBWsize %d\n", arc->imgBWsize);

  return arc->id;
}
}

#include "bindings.cpp"
