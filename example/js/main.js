var numIset;

window.addEventListener('dispImageSet-loaded', function() {
  var ariset = new ARiset(893, 1117);
  // We need to load the first Image to get numIset with the listener
  ariset.loadNFTMarker('data/pinball', 0);
    document.addEventListener('nftMarker', function (ev) {
      numIset = ev.detail.numIset;
      // Now we can load the other images
      var i = 1;
          while(i<numIset) { 
              ariset.loadNFTMarker('data/pinball', i);
              i++;
            if (i===numIset){
              break;
            }
          }
      var imageSetWidth = ev.detail.widthNFT;
      var imageSetHeight = ev.detail.heightNFT;
      var dpi = ev.detail.dpi;
      var divNumIset = document.getElementById("numIset");
      var contentNI = document.createTextNode(numIset);
      divNumIset.appendChild(contentNI);
      ariset.display();
      infos(imageSetWidth, imageSetHeight, dpi);
  })
})

function infos(imageSetWidth, imageSetHeight, dpi){
  var parent = document.createElement('div');
  document.body.appendChild(parent);
  var contentW = document.createTextNode('Width: ' + imageSetWidth + ' ');
  parent.appendChild(contentW);
  var contentH = document.createTextNode('Height: ' + imageSetHeight+ ' ');
  parent.appendChild(contentH);
  var contentDpi = document.createTextNode('Dpi: ' + dpi);
  parent.appendChild(contentDpi);
}

