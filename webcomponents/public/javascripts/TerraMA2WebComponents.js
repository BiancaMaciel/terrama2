// Makes sure that jQuery has been loaded before terrama2.js
if(typeof jQuery === "undefined") {
  throw new Error("TerraMA² WebComponents requires jQuery");
}

// Makes sure that OpenLayers 3 has been loaded before terrama2.js
if(typeof ol === "undefined") {
  throw new Error("TerraMA² WebComponents requires Openlayers 3");
}

"use strict";

window.TerraMA2WebComponents = {
  webcomponents: {}
};

/**
 * Main class of the API.
 * @class TerraMA2WebComponents
 *
 * @author Jean Souza [jean.souza@funcate.org.br]
 *
 * @property {array} memberComponents - Array of components names received on the initialization.
 * @property {int} memberComponentsLength - Length of the components names array.
 * @property {boolean} memberComponentsLoaded - Flag that indicates if all the components have been loaded.
 * @property {string} memberTerrama2Url - TerraMA² WebComponents URL.
 * @property {json} memberConfJsonComponents - Configuration JSON containing the paths of the components files.
 * @property {function} memberCallbackFunction - Callback function to be executed when all the components are loaded.
 */
TerraMA2WebComponents.obj = (function() {

  // Array of components names received on the initialization
  var memberComponents = null;
  // Length of the components names array
  var memberComponentsLength = null;
  // Flag that indicates if all the components have been loaded
  var memberComponentsLoaded = false;
  // TerraMA² WebComponents URL
  var memberTerrama2Url = null;
  // Configuration JSON containing the paths of the components files
  var memberConfJsonComponents = null;
  // Callback function to be executed when all the components are loaded
  var memberCallbackFunction = null;

  /**
   * Returns the length of the components names array.
   * @returns {int} memberComponentsLength - Length of the components names array
   *
   * @function getComponentsLength
   * @memberof TerraMA2WebComponents
   * @inner
   */
  var getComponentsLength = function() {
    return memberComponentsLength;
  };

  /**
   * Returns the URL of the TerraMA² WebComponents.
   * @returns {string} memberTerrama2Url - URL of the TerraMA² WebComponents
   *
   * @function getComponentsLength
   * @memberof TerraMA2WebComponents
   * @inner
   */
  var getTerrama2Url = function() {
    return memberTerrama2Url;
  };

  /**
   * Verifies if a given file exists.
   * @param {string} url - URL of the file
   * @returns {boolean} boolean - Flag that indicates if the file exists
   *
   * @function fileExists
   * @memberof TerraMA2WebComponents
   * @inner
   */
  var fileExists = function(url) {
    $.ajax({
      url: url,
      async: false,
      error: function() {
        return false;
      },
      success: function() {
        return true;
      }
    });
  };

  /**
   * Loads the components present in the components array.
   * @param {int} i - Current array index
   *
   * @private
   * @function loadComponents
   * @memberof TerraMA2WebComponents
   * @inner
   */
  var loadComponents = function(i) {
    if(i < memberComponentsLength) {
      $.ajax({
        url: memberTerrama2Url + "/javascripts/components/" + memberConfJsonComponents[memberComponents[i]],
        dataType: "script",
        success: function() {
          loadComponents(++i);
        }
      });
    } else {
      memberComponentsLoaded = true;

      $.each(memberComponents, function(i, componentItem) {
        TerraMA2WebComponents.webcomponents[componentItem].init();
      });

      memberCallbackFunction();
    }
  };

  /**
   * Returns the flag that indicates if all the components have been loaded.
   * @returns {boolean} memberComponentsLoaded - Flag that indicates if all the components have been loaded
   *
   * @function isComponentsLoaded
   * @memberof TerraMA2WebComponents
   * @inner
   */
  var isComponentsLoaded = function() {
    return memberComponentsLoaded;
  };

  /**
   * Loads the configuration files.
   *
   * @private
   * @function loadConfigurations
   * @memberof TerraMA2WebComponents
   * @inner
   */
  var loadConfigurations = function() {
    var url = memberTerrama2Url + "/config/";

    $.getJSON(url + "Components.TerraMA2WebComponents.json", function(data) {
      memberConfJsonComponents = data;
      loadComponents(0);
    });
  };

  /**
   * Initializes the necessary features.
   * @param {string} terrama2Url - TerraMA² WebComponents URL
   * @param {array} components - Array of components names
   * @param {function} callbackFunction - Callback function to be executed when all the components are loaded
   *
   * @function init
   * @memberof TerraMA2WebComponents
   * @inner
   */
  var init = function(terrama2Url, components, callbackFunction) {
    memberCallbackFunction = callbackFunction;
    memberComponents = components;
    memberComponentsLength = components.length;
    memberTerrama2Url = terrama2Url;
    loadConfigurations();
  };

  return {
  	getComponentsLength: getComponentsLength,
  	getTerrama2Url: getTerrama2Url,
  	fileExists: fileExists,
    isComponentsLoaded: isComponentsLoaded,
  	init: init
  };
})();
