window.addEventListener('dispImageSet-loaded', function() {
  var ariset = new ARiset();
  // We need to load the first Image to get numIset with the listener
  ariset.loadNFTMarker('data/pinball');
    document.addEventListener('nftNumIset', function (ev) {
      var numIset = ev.detail.numIset;
      var divNumIset = document.getElementById("numIset");
      var contentNI = document.createTextNode(numIset);
      divNumIset.appendChild(contentNI);
  })
})