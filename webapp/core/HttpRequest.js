var AbstractRequest = require('./AbstractRequest');
var Promise = require('bluebird');
var Exceptions = require("./Exceptions");
var NodeUtils = require('util');
var Requester = require('request');

var HttpRequest = function(params) {
  AbstractRequest.apply(this, arguments);
};

HttpRequest.prototype = Object.create(AbstractRequest.prototype, {
  'constructor': HttpRequest
});

HttpRequest.prototype.request = function() {
  var self = this;
  return  new Promise(function(resolve, reject) {

  //   todo: implement it correctly
    var uri = NodeUtils.format("%s://%s:%s@%s:%s%s", self.params.kind.toLowerCase(),
                                                     self.params.user,
                                                     self.params.password,
                                                     self.params.address,
                                                     self.params.port,
                                                     self.params.path);
    Requester(uri, function(err, resp, body) {
      if (err)
        reject(new Exceptions.ConnectionError("Error in http request"));
      else {
        if (resp.statusCode === 200)
          resolve();
        else
          reject(new Exceptions.ConnectionError(NodeUtils.format("Error in http request: (%d) - %s", resp.statusCode, resp.statusMessage)));
      }
    });
  });
};


module.exports = HttpRequest;