'use strict';

var doSlide = function(layerId, layerTime) {
  //  DO REQUEST
  var timeFormat = moment(layerTime).format("YYYY-MM-DDThh:mm:ss") + "Z";

  TerraMA2WebComponents.MapDisplay.updateLayerTime(/**id */layerId, /** time */layerTime);
}

/**
 * Triggered when user clicks on TerraMA² Slider in Layers Menu
 * 
 * @param {Event}
 */
$("#terrama2-layerexplorer").on("click", "#terrama2-slider", function(event) {
  var self = $(this);
  var parentLi = $(self).parent();
  var parentId = $(parentLi).attr("data-layerid");
  var layerName = parentId.split(':')[1];
  var capability = sliderCapabilities.find(function(capability) { return capability.name === layerName; });

  if (!capability) {
    console.log("Capability not found...");
    return;
  }
  if (!capability.extent instanceof Array) {
    console.log("Capability has not extent array.");
    return;
  }

  setSlider(capability.extent, parentId);
});

$("#terrama2-layerexplorer").on("click", "button[class~='close-slider']", function(e) {
  $(this).parent().hide();
});

/**
 * Triggered when user clicks on TerraMA² Slider in Layers Menu - temporary
 * 
 * @param {Event}
 */

$("#terrama2-layerexplorer").on("click", "#terrama2-slider-mask", function(event) {
  var self = $(this);
  var parentLi = $(self).parent();
  var sliderContent = parentLi.children(".slider-content");
  if (sliderContent.is(':hidden'))
    sliderContent.show();
  else
    sliderContent.hide();

});


/**
 * Triggered when user clicks on TerraMA² Slider in Layers Menu - temporary
 * 
 * @param {Event}
 */

$("#terrama2-sortlayers").on("click", "#terrama2-opacity-slider", function(event) {
  
  var self = $(this);
  var parentLi = $(self).parent();
  var parentId = $(parentLi).attr("data-layerid");
  var actualOpacity = TerraMA2WebComponents.MapDisplay.getLayerOpacity(parentId) * 100;
  setOpacitySlider(parentId, actualOpacity);

});


$("#terrama2-sortlayers").on("click", "button[class~='close-slider']", function(e) {
  $(this).parent().hide();
});

var changeLayerOpacity = function(layerId, opacityValue){
  
  TerraMA2WebComponents.MapDisplay.updateLayerOpacity(layerId, opacityValue/100);
}