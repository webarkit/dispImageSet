#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <emscripten.h>
#include <AR/ar.h>
#include <AR2/config.h>
#include <AR2/imageFormat.h>
#include <AR2/imageSet.h>
#include <AR2/tracking.h>
#include <AR2/util.h>
#include <KPM/kpm.h>

#define PAGES_MAX               10          // Maximum number of pages expected. You can change this down (to save memory) or up (to accomodate more pages.)

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

  int loadNFTMarker(arIset *arc, int surfaceSetCount, int numImage, const char* filename) {
    ARLOG("Read ImageSet.\n");
    ARLOGi("numImage is: %d\n", numImage);
    ar2UtilRemoveExt( (char*)filename );
    arc->imageSet = ar2ReadImageSet( (char*)filename );
    if( arc->imageSet == NULL ) {
        ARLOGe("file open error: %s.iset\n", filename );
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
		if (arIsets.find(id) == arIsets.end()) { return nft; }
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

    EM_ASM_({
			if (!ariset["frameMalloc"]) {
				ariset["frameMalloc"] = ({});
			}
			var frameMalloc = ariset["frameMalloc"];
      frameMalloc["frameIbwpointer"] = $1;
      frameMalloc["frameimgBWsize"] = $2;
		},
			arc->id,
      arc->imgBW,
      arc->imgBWsize
		);

    nft.numIset = arc->numIset;
    nft.widthNFT = arc->width_NFT;
    nft.heightNFT = arc->height_NFT;
    nft.dpiNFT = arc->dpi_NFT;
    nft.imgBWsize = arc->imgBWsize;
    nft.pointer = (int)arc->imgBW;

    return nft;
	}

  int setup(int width, int height){
    int id = gARIsetID++;
		arIset *arc = &(arIsets[id]);
		arc->id = id;
    arc->imgBWsize = width * height * 4 * sizeof(ARUint8);
    arc->imgBW = (ARUint8*) malloc(arc->imgBWsize);

    ARLOGi("Allocated imgBWsize %d\n", arc->imgBWsize);

		return arc->id;
  }

}

#include "bindings.cpp"
