var MapDisplay = function() {

  var getMap = function() {
    return olMap;
  }

  var createTileWMS = function(url, type, layerName, layerTitle) {
    return new ol.layer.Tile({
      source: new ol.source.TileWMS({
        preload: Infinity,
        url: url,
        serverType: type,
        params: {
          'LAYERS': layerName, 'TILED': true
        }
      }),
      name: layerName,
      title: layerTitle,
      visible: false
    });
  }

  /**right
  * @param {any} value
  * @returns {ol.layer.Base}
  */
  var findBy = function(layer, key, value) {

    if (layer.get(key) === value) {
      return layer;
    }

    // Find recursively if it is a group
    if (layer.getLayers) {
      var layers = layer.getLayers().getArray(),
      len = layers.length, result;
      for (var i = 0; i < len; i++) {
        result = findBy(layers[i], key, value);
        if (result) {
          return result;
        }
      }
    }

    return null;
  }

  this.getMap = getMap;
  this.createTileWMS = createTileWMS;
  this.findBy = findBy;

  var olMap = new ol.Map({
    renderer: 'canvas',
    layers: [
      new ol.layer.Group({
        layers: [
          new ol.layer.Tile({
            source: new ol.source.MapQuest({layer: 'sat'}),
            name: 'mapquest_sat',
            title: 'MapQuest Sat&eacute;lite',
            visible: false
          }),
          new ol.layer.Tile({
            source: new ol.source.MapQuest({layer: 'osm'}),
            name: 'mapquest_osm',
            title: 'MapQuest OSM',
            visible: false
          }),
          new ol.layer.Tile({
            source: new ol.source.OSM(),
            name: 'osm',
            title: 'Open Street Map',
            visible: true
          })
        ],
        name: 'bases',
        title: 'Camadas Base'
      })
    ],
    target: 'terrama2-map',
    view: new ol.View({
      projection: 'EPSG:4326',
      center: [-55, -15],
      zoom: 4
    })
  });

  olMap.getLayerGroup().set('name', 'root');
  olMap.getLayerGroup().set('title', 'Geoserver Local');
}
