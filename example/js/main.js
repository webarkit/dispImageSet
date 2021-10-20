var numIset;

window.addEventListener('dispImageSet-loaded', function() {
  var ariset = new ARiset(893, 1117);
  // We need to load the first Image to get numIset with the listener
  ariset.loadNFTMarker('data/pinball', 0);
  ariset.display();
    document.addEventListener('nftMarker', function (ev) {
      numIset = ev.detail.numIset;
      var imageSetWidth = ev.detail.widthNFT;
      var imageSetHeight = ev.detail.heightNFT;
      var dpi = ev.detail.dpi;
      var divNumIset = document.getElementById("numIset");
      var contentNI = document.createTextNode(numIset);
      divNumIset.appendChild(contentNI);
      infos(imageSetWidth, imageSetHeight, dpi);
  })
})

function infos(imageSetWidth, imageSetHeight, dpi){
  var parent = document.createElement('div');
  parent.id = "infos"
  var nftInfos = document.getElementById("nftInfos");
  nftInfos.appendChild(parent);
  var contentW = document.createTextNode('Width: ' + imageSetWidth + ' ');
  parent.appendChild(contentW);
  var contentH = document.createTextNode('Height: ' + imageSetHeight+ ' ');
  parent.appendChild(contentH);
  var contentDpi = document.createTextNode('Dpi: ' + dpi);
  parent.appendChild(contentDpi);
}

