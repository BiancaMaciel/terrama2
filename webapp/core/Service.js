var net = require('net');
var Promise = require('bluebird');
var Utils = require('./Utils');
var EventEmitter = require('events').EventEmitter;
var NodeUtils = require('util');
var Signals = require('./Signals')


/**
 This method parses the bytearray received.
 @param {Buffer} byteArray - a nodejs buffer with bytearray received
 @return {Object} object - a javascript object with signal, message and size

 */
function parseByteArray(byteArray) {
  var messageSizeReceived = byteArray.readUInt32BE(0);
  var signalReceived = byteArray.readUInt32BE(4);
  var rawData = byteArray.slice(8, byteArray.length);

  // validate signal
  var signal = Utils.getTcpSignal(signalReceived);

  var jsonMessage;

  if (rawData.length === 0)
    jsonMessage = {};
  else
    jsonMessage = JSON.parse(rawData);

  return {
    size: messageSizeReceived,
    signal: signal,
    message: jsonMessage
  }
}


var Service = module.exports = function(serviceInstance) {
  EventEmitter.call(this);
  this.service = serviceInstance;

  var self = this;

  self.socket = new net.Socket();

  var callbackSuccess = null;
  var callbackError = null;

  self.socket.on('data', function(byteArray) {
    console.log("client received: ", byteArray);
    console.log("client received: ", byteArray.toString());

    try  {
      var parsed = parseByteArray(byteArray);

      switch(parsed.signal) {
        case Signals.LOG_SIGNAL:

          self.logs.push(parsed.message);
          self.emit("log", parsed.message);
          break;
        case Signals.STATUS_SIGNAL:
          self.emit("status", parsed.message);
          break;
        case Signals.TERMINATE_SERVICE_SIGNAL:
          self.emit("stop", parsed);
          break;
      }

      if (callbackSuccess)
        callbackSuccess(parsed);
    } catch (e) {
      console.log("Error parsing bytearray: ", e);
      self.emit('error', e);
      if (callbackError)
        callbackError(e);
    }

  });

  self.socket.on('close', function(byteArray) {
    self.emit('close', byteArray);
    console.log("client closed: ", byteArray);
  });

  self.socket.on('error', function(err) {
    callbackError(err);
    console.log("client error: ", err);
  });

  self.isOpen = function() {
    return self.socket.readyState == "open";
  };

  self.connect = function() {
    return new Promise(function(resolve, reject) {
      if (!self.isOpen()) {
        callbackError = reject;
        self.socket.connect(self.service.port, self.service.host, function() {
          resolve();
        })
      } else
        reject(new Error("Could not connect. There is a opened connection"));
    })
  };

  self.status = function(buffer) {
    return new Promise(function(resolve, reject) {
      if (!self.isOpen()) {
        self.emit('error', new Error("Could not retrieve status from closed connection"));
        return;
      }

      self.socket.write(buffer);

      socket.setTimeout(2000, function() {
        self.emit("error", new Error("Status Timeout exceeded."));
      })
    });
  };

  self.update = function(buffer) {
    return new Promise(function(resolve, reject) {
      if (!self.isOpen()) {
        self.emit('error', new Error("Could not update service from closed connection"));
        return;
      }

      callbackError = reject;
      self.socket.write(buffer);

      resolve();
    });
  };

  self.send = function(buffer) {
    if (!self.isOpen()) {
      self.emit('error', new Error("Could not send data from closed connection"));
      return;
    }

    self.socket.write(buffer);
  };

  self.stop = function(buffer) {
    return new Promise(function(resolve, reject) {
      if (!self.isOpen()) {
        self.emit('error', new Error("Could not close a no existent connection"));
        return;
      }

      callbackSuccess = resolve;
      callbackError = reject;

      self.socket.write(buffer);

      self.socket.setTimeout(5000, function() {
        self.emit("error", new Error("Stop Timeout exceeded."));
      })
    });
  };

  self.log = function(buffer) {
    return new Promise(function(resolve, reject) {
      if (!self.isOpen()) {
        return reject(new Error("Could not apply log request from a no existent connection"));
      }

      callbackSuccess = resolve;
      callbackError = reject;

      self.socket.write(buffer);
    })
  }
};

NodeUtils.inherits(Service, EventEmitter);
