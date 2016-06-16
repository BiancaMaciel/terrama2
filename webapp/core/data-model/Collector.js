var BaseClass = require("./AbstractData");
var Schedule = require("./Schedule");
var Filter = require('./Filter');
var Intersection = require('./Intersection');

var Collector = module.exports = function(params) {
  BaseClass.call(this, {'class': 'Collector'});
  this.id = params.id;
  this.project_id = params.project_id;
  this.service_instance_id = params.service_instance_id;
  this.input_data_series = params.data_series_input;
  this.output_data_series = params.data_series_output;

  if (params.CollectorInputOutputs)
    this.setInputOutputMap(params.CollectorInputOutputs);
  else
    this.input_output_map = params.input_output_map || [];

  if (params.Schedule)
    this.schedule = new Schedule(params.Schedule.get() || {});
  else
    this.schedule = params.schedule || {};

  if (params.Filter)
    this.filter = new Filter(params.Filter.get() || {});
  else
    this.filter = params.filter || {};

  if (params.Intersections)
    this.setIntersection(params.Intersections);
  else
    this.intersection = params.intersection || [];

  this.active = params.active;
};

Collector.prototype = Object.create(BaseClass.prototype);
Collector.prototype.constructor = Collector;

Collector.prototype.setIntersection = function (intersection) {
  if (intersection instanceof Array) {
    var output = [];
    intersection.forEach(function(instance) {
      if (instance instanceof BaseClass)
        output.push(instance);
      else // sequelize
        output.push(new Intersection(instance.get()));
    });
    this.intersection = output;
  } else
    this.intersection = intersection || [];
};

Collector.prototype.setInputOutputMap = function (inputOutputModel) {
  var output = [];
  inputOutputModel.forEach(function(element) {
    // TODO: fix it
    if (typeof element.get === "function") {
      output.push({input: element.input_dataset, output: element.output_dataset});
    }
    else
      output.push(element);
  })
  this.input_output_map = output;
};

Collector.prototype.rawObject = function () {
  var obj = this.toObject();
  obj.schedule = this.schedule.toObject();
  obj.filter = this.filter instanceof BaseClass ? this.filter.toObject() : this.filter;

  var intersectionArray = [];
  this.intersection.forEach(function(intersect) {
    if (intersect instanceof BaseClass)
      intersectionArray.push(intersect.toObject());
    else
      intersectionArray.push(intersect);
  })
  obj.intersection = intersectionArray;
  return obj;
};

Collector.prototype.toObject = function() {
  // preparing intersection
  var intersectionOutput = {};
  this.intersection.forEach(function(intersectionInstance) {
    intersectionOutput.collector_id = intersectionInstance.collector_id;

    if (!intersectionOutput.attribute_map)
      intersectionOutput.attribute_map = {};

    if (!intersectionOutput.attribute_map[intersectionInstance.dataseries_id])
      intersectionOutput.attribute_map[intersectionInstance.dataseries_id] = [];

    if (intersectionInstance.attribute)
      intersectionOutput.attribute_map[intersectionInstance.dataseries_id].push(intersectionInstance.attribute);
  });

  if (Object.keys(intersectionOutput).length > 0)
    intersectionOutput.class = "Intersection";

  return Object.assign(BaseClass.prototype.toObject.call(this), {
    id: this.id,
    project_id: this.project_id,
    service_instance_id: this.service_instance_id,
    input_data_series: this.input_data_series,
    output_data_series: this.output_data_series,
    input_output_map: this.input_output_map || [],
    schedule: this['schedule'] instanceof BaseClass ? this['schedule'].toObject() : this['schedule'],
    filter: this['filter'] instanceof BaseClass ? this.filter.toObject() : this.filter,
    intersection: intersectionOutput,
    active: this.active
  });
};
