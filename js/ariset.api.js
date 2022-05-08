(function () {
  "use strict";
  var scope;
  if (typeof window !== "undefined") {
    scope = window;
  } else {
    scope = global;
  }

  var ARiset = function (width, height) {
    this.id = 0;
    this.nftMarkerCount = 0;
    this.numIset = 0;
    this.imageSetWidth = 0;
    this.imageSetHeight = 0;
    this.dpi = 0;
    this._init(width, height);
  };

  ARiset.prototype.display = function () {
    var self = this;
    document.addEventListener('nftMarker', function(ev) {
      self.numIset = ev.detail.numIset;
      self.imageSetWidth = ev.detail.widthNFT;
      self.imageSetHeight = ev.detail.heightNFT;
      self.dpi = ev.detail.dpi;
    })
  };

  ARiset.prototype.loadNFTMarker = function (markerURL, numImage, onSuccess, onError) {
    var self = this;
    if (markerURL) {
      return ariset.readNFTMarker(
        this.id,
        numImage,
        markerURL,
        function (nftMarker) {
          console.log(nftMarker);
          var nftEvent = new CustomEvent('nftMarker', {
            detail: {
              numImage: numImage,
              numIset: nftMarker.numIset,
              widthNFT: nftMarker.width,
              heightNFT: nftMarker.height,
              dpi: nftMarker.dpi
            }
          });
          document.dispatchEvent(nftEvent);
        },
        onError
      );
    } else {
      if (onError) {
        onError("Marker URL needs to be defined and not equal empty string!");
      } else {
        console.error(
          "Marker URL needs to be defined and not equal empty string!"
        );
      }
    }
  };

  ARiset.prototype.getImageSet = function () {
    return this.imageSet;
  };

  ARiset.prototype._init = function (width, height) {
    this.id = ariset.setup(width, height);

  };

  var markerCount = 0;

  function readNFTMarker(arId, numImage, url, callback, onError) {
    var mId = markerCount++;
    var prefix = "/markerNFT_" + mId;
    var filename1 = prefix + ".fset";
    var filename2 = prefix + ".iset";
    var filename3 = prefix + ".fset3";
    ajax(
      url + ".fset",
      filename1,
      function () {
        ajax(
          url + ".iset",
          filename2,
          function () {
            ajax(
              url + ".fset3",
              filename3,
              function () {
                var id = Module._readNFTMarker(arId, numImage, prefix);
                if (callback) callback(id);
              },
              function(errorNumber) {
                if (onError) onError(errorNumber);
              }
            );
          },
          function(errorNumber) {
            if (onError) onError(errorNumber);
          }
        );
      },
      function(errorNumber) {
        if (onError) onError(errorNumber);
      }
    );
  }

  function writeStringToFS(target, string, callback) {
    var byteArray = new Uint8Array(string.length);
    for (var i = 0; i < byteArray.length; i++) {
      byteArray[i] = string.charCodeAt(i) & 0xff;
    }
    writeByteArrayToFS(target, byteArray, callback);
  }

  function writeByteArrayToFS(target, byteArray, callback) {
    FS.writeFile(target, byteArray, { encoding: "binary" });
    // console.log('FS written', target);

    callback(byteArray);
  }

  function ajax(url, target, callback, errorCallback) {
    var oReq = new XMLHttpRequest();
    oReq.open("GET", url, true);
    oReq.responseType = "arraybuffer"; // blob arraybuffer

    oReq.onload = function () {
      if (this.status == 200) {
        // console.log('ajax done for ', url);
        var arrayBuffer = oReq.response;
        var byteArray = new Uint8Array(arrayBuffer);
        writeByteArrayToFS(target, byteArray, callback);
      } else {
        errorCallback(this.status);
      }
    };

    oReq.send();
  }

  var ariset = {
    readNFTMarker: readNFTMarker
  };

  var FUNCTIONS = ["setup", "display", "getImageSet"];

  function runWhenLoaded() {
    FUNCTIONS.forEach(function (n) {
      ariset[n] = Module[n];
    });

    for (var m in Module) {
      if (m.match(/^AR/)) ariset[m] = Module[m];
    }
  }

  scope.ariset = ariset;
  scope.ARiset = ARiset;

  if (scope.Module) {
    scope.Module.onRuntimeInitialized = function () {
      runWhenLoaded();
      var event = new Event("dispImageSet-loaded");
      scope.dispatchEvent(event);
    };
  } else {
    scope.Module = {
      onRuntimeInitialized: function () {
        runWhenLoaded();
      }
    };
  }
})();
