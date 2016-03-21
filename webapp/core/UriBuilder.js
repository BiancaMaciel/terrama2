var Util = require('util');
var URL = require("url");


function buildUri(uriObjectGiven) {
  var uriObject = {
    hostname: uriObjectGiven.host || uriObjectGiven.address,
    port: uriObjectGiven.port,
    protocol: uriObjectGiven.kind.toLowerCase() || uriObjectGiven.scheme.toLowerCase(),
    pathname: uriObjectGiven.path || uriObjectGiven.pathname,
    slashes: true // It defines that URI protocol require colon-slash-slash
  };

  if (uriObjectGiven.auth)
    uriObject.auth = uriObjectGiven.auth;
  else if (uriObjectGiven.user || uriObjectGiven.username || uriObjectGiven.pass || uriObjectGiven.password) {
    var user = uriObjectGiven.user || uriObjectGiven.username;
    var password = uriObjectGiven.pass || uriObjectGiven.password;
    uriObject.auth = Util.format("%s:%s", user ? user : "", password ? password : "");
  }

  return URL.format(uriObject);
}

function buildObject(uriString) {
  // todo: parse
  var uriObject = URL.parse(uriString);

  var auth;
  if (uriObject.auth)
    auth = uriObject.auth.split(':');

  return {
    kind: uriObject.protocol.substr(0, uriObject.protocol.length-1).toUpperCase(),
    address: uriObject.hostname,
    user: auth ? auth[0] : '',
    password: auth ? auth[1] : '',
    path: uriObject.pathname,
    port: uriObject.port
  };
}

module.exports = {
  buildUri: buildUri,
  buildObject: buildObject
};