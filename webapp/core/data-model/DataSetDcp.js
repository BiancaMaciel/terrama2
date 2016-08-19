var DataSet = require('./DataSet');


function DataSetDcp(params) {
  DataSet.call(this, params);

  this.positionWkt = params.positionWkt || null;
  this.position = params.position;
}

DataSetDcp.prototype = Object.create(DataSet.prototype);
DataSetDcp.prototype.constructor = DataSetDcp;

DataSetDcp.prototype.toObject = function() {
  return Object.assign(DataSet.prototype.toObject.call(this), {position: this.positionWkt || this.position});
};

DataSetDcp.prototype.rawObject = function() {
  return Object.assign(DataSet.prototype.toObject.call(this), {position: this.position});
};

module.exports = DataSetDcp;
