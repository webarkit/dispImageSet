var interval = setTimeout(function() {
  var ariset = new ARiset(893, 1117);
  console.log(ariset);
  ariset.loadNFTMarker('data/pinball');
  ariset.display();
}, 200);
