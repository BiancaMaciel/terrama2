var AbstractRequest = require('./AbstractRequest');
var Promise = require('bluebird');
var Exceptions = require("./Exceptions");
var Requester = require('request');
var NodeUtils = require('util');
var UriBuilder = require('./UriBuilder');

var WcsRequest = function(params) {
  AbstractRequest.apply(this, arguments);
};

WcsRequest.prototype = Object.create(AbstractRequest.prototype);
WcsRequest.prototype.constructor = WcsRequest;

WcsRequest.prototype.request = function() {
  var self = this;
  return  new Promise(function(resolve, reject) {
    var object = Object.assign(self.params, {});
    object.kind = "http";
    var uri = UriBuilder.buildUri(object) + "?service=WCS&version=2.0.1&request=GetCapabilities";

    Requester(uri, function(err, resp, body) {
      if (err)
        reject(new Exceptions.ConnectionError("Error in wcs request"));
      else {
        if (resp.statusCode === 200) {
          // todo: parse WCS GetCapabilities xml
          resolve();
        }
        else
          reject(new Exceptions.ConnectionError(NodeUtils.format("Error in http request: (%d) - %s", resp.statusCode, resp.statusMessage)));
      }
    });
  });
};


module.exports = WcsRequest;