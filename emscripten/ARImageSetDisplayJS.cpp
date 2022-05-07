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

GLuint program;

#define PIX_C(x, y) ((x)/256.0f + (y)/256.0f)
#define CLAMP(c) ((c) < 0.f ? 0.f : ((c) > 1.f ? 1.f : (c)))
#define PIX(x, y) CLAMP(PIX_C(x, y))

extern "C" {
using emscripten::val;
void draw()
{
  int w, h;
  emscripten_get_canvas_element_size("#canvas", &w, &h);
  printf("canvas width: %d\n", w);
  printf("canvas height: %d\n", h);
  float xs = (float)h / w;
  float ys = 1.0f;
  float mat[] = { xs, 0, 0, 0, 0, ys, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
  glUniformMatrix4fv(glGetUniformLocation(program, "mat"), 1, 0, mat);
  glClearColor(0,0,1,1);
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  unsigned char* imageData = (unsigned char*)malloc(256*256*4*sizeof(unsigned char));
  glReadPixels(0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
  for(int y = 0; y < 256; ++y)
    for(int x = 0; x < 256; ++x)
    {
      unsigned char red = imageData[(y*256+x)*4];
      float expectedRed = PIX(x, y);
      unsigned char eRed = (unsigned char)(expectedRed * 255.0f);
    }
  emscripten_cancel_main_loop();
}

//void emscripten_canvas(int width, int height, const char* name, float* texData) {
void emscripten_canvas(int width, int height, const char* name, unsigned char* texData) {
  emscripten_set_canvas_element_size(name, width, height);
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.alpha = attr.depth = attr.stencil = attr.antialias =
      attr.preserveDrawingBuffer = attr.failIfMajorPerformanceCaveat = 0;
  attr.enableExtensionsByDefault = 1;
  attr.premultipliedAlpha = 0;
  attr.majorVersion = 1;
  attr.minorVersion = 0;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx =
      emscripten_webgl_create_context(name, &attr);
  emscripten_webgl_make_context_current(ctx);
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  const char *vss = "attribute vec4 vPosition; uniform mat4 mat; varying vec2 "
                    "texCoord; void main() { gl_Position = vPosition; texCoord "
                    "= (vPosition.xy + vec2(1.0)) * vec2(0.5); }";
  glShaderSource(vs, 1, &vss, 0);
  glCompileShader(vs);
  GLint isCompiled = 0;
  glGetShaderiv(vs, GL_COMPILE_STATUS, &isCompiled);
  if (!isCompiled) {
    GLint maxLength = 0;
    glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &maxLength);
    char *buf = new char[maxLength];
    glGetShaderInfoLog(vs, maxLength, &maxLength, buf);
    printf("%s\n", buf);
  }

  GLuint ps = glCreateShader(GL_FRAGMENT_SHADER);
  const char *pss = "precision lowp float; varying vec2 texCoord; uniform vec3 "
                    "colors[3]; uniform sampler2D tex; void main() { "
                    "gl_FragColor = texture2D(tex, texCoord); }";
  glShaderSource(ps, 1, &pss, 0);
  glCompileShader(ps);
  glGetShaderiv(ps, GL_COMPILE_STATUS, &isCompiled);
  if (!isCompiled) {
    GLint maxLength = 0;
    glGetShaderiv(ps, GL_INFO_LOG_LENGTH, &maxLength);
    char *buf = new char[maxLength];
    glGetShaderInfoLog(ps, maxLength, &maxLength, buf);
    printf("%s\n", buf);
  }

  program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, ps);
  glBindAttribLocation(program, 0, "vPosition");
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &isCompiled);
  if (!isCompiled) {
    GLint maxLength = 0;
    glGetShaderiv(program, GL_INFO_LOG_LENGTH, &maxLength);
    char *buf = new char[maxLength];
    glGetProgramInfoLog(program, maxLength, &maxLength, buf);
    printf("%s\n", buf);
  }

  glUseProgram(program);

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  float verts[] = {-1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, 1};
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, 0, sizeof(float) * 2, 0);
  glEnableVertexAttribArray(0);
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  /*glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE,
               GL_FLOAT, texData);*/
  glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE,
               GL_UNSIGNED_BYTE, texData);
  emscripten_set_main_loop(draw, 0, 0);
}

void emscripten_canvas2(int width, int height, std::string name, unsigned char* texData)
{
  EM_ASM_({
    var canvas = document.getElementById("canvas");
    var ctx = canvas.getContext("2d");
    var width = $0;
    var height = $1;
    ctx.canvas.width = width;
    ctx.canvas.height = height;
    var Buffer = new Uint8ClampedArray(
      Module.HEAPU8.buffer,
      $2,
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
  width,
  height,
  texData
  );
}

void emscripten_canvas3()
{
  val canvas = val::global("document").call<val>("getElementById", val("canvas"));
  val ctx = canvas.call<val>("getContext", val("2d"));
  ctx.set("fillStyle", "green");
  ctx.call<void>("fillRect", 10, 10, 150, 100);
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
  int width = 256;
  int height = 256;
  float *floatTexData = (float *)malloc(width * height * sizeof(float));
  for (int y = 0; y < height; ++y)
    for (int x = 0; x < width; ++x) {
      floatTexData[y * width + x] = PIX(x, y);
    }
  //emscripten_canvas(256, 256, "#canvas", (unsigned char*)floatTexData);
  emscripten_canvas2(arc->width_NFT, arc->height_NFT, "canvas", arc->imgBW);
  //emscripten_canvas3();
  //emscripten_canvas(256, 256, "#canvas", arc->imgBW);

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
