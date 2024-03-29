(function () {
  "use strict";
  var scope;
  if (typeof window !== "undefined") {
    scope = window;
  } else {
    scope = global;
  }

  var ARiset = function () {
    this.id = 0;
    this._init();
    this.version = "0.1.0";
    console.log("dispImageSet version: ", this.version);
  };

  ARiset.prototype.loadNFTMarker = function (markerURL, onSuccess, onError) {
    var self = this;
    if (markerURL) {
      return ariset.readNFTMarker(
        this.id,
        markerURL,
        function (nftMarker) {
          if(nftMarker) { 
            var nftEvent = new CustomEvent('nftNumIset', {
              detail: {
                numIset: ariset.getNumIset(self.id),             
              }
            });
            document.dispatchEvent(nftEvent);
          }
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

  ARiset.prototype._init = function () {
    this.id = ariset.setup();
  };

  var markerCount = 0;

  function readNFTMarker(arId, url, callback, onError) {
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
                var id = Module._readNFTMarker(arId, prefix);
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

  var FUNCTIONS = ["setup", "getNumIset"];

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
