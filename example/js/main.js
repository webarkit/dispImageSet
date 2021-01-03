var numIset;

window.addEventListener('dispImageSet-loaded', function() {
  var ariset = new ARiset(893, 1117);
    for (var i = 0; i < 9; i++) {
    ariset.loadNFTMarker('data/pinball', i);
    }
    document.addEventListener('nftMarker', function (ev) {
      numIset = ev.detail.numIset;
      var imageSetWidth = ev.detail.widthNFT;
      var imageSetHeight = ev.detail.heightNFT;
      var dpi = ev.detail.dpi;
  
      var divNumIset = document.getElementById("numIset");
      var contentNI = document.createTextNode(numIset);
      divNumIset.appendChild(contentNI);
  
      var divW = document.getElementById("widthNFT");
      var contentW = document.createTextNode(imageSetWidth);
      divW.appendChild(contentW);
  
      var divH = document.getElementById("heightNFT");
      var contentH = document.createTextNode(imageSetHeight);
      divH.appendChild(contentH);
  
      var divDpi = document.getElementById("dpiNFT");
      var contentDpi = document.createTextNode(dpi);
      divDpi.appendChild(contentDpi);
    ariset.display();
  })
})

