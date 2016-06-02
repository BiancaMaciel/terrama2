var BaseClass = require("./AbstractData");

var Analysis = module.exports = function(params) {
  BaseClass.call(this, {'class': 'Analysis'});
  this.id = params.id;
  this.project_id = params.project_id;
  this.script_language = params.script_language;
  this.type_id = params.type_id;
  this.name = params.name;
  this.description = params.description;
  this.active = params.active;
  this.dataset_output = params.dataset_output;

  if (params.AnalysisMetadata)
    this.setMetadata(params.AnalysisMetadata);
  else
    this.metadata = params.metadata || {};

  this.analysis_dataseries_list = [];
  this.schedule_id = params.schedule_id;
};

Analysis.prototype = Object.create(BaseClass.prototype);
Analysis.prototype.constructor = Analysis;

Analysis.prototype.addAnalysisDataSeries = function(analysisDataSeries) {
  this.analysis_dataseries_list.push(analysisDataSeries);
};

Analysis.prototype.setMetadata = function(metadata) {
  var meta = {};
  if (metadata instanceof Array) {
    // array of sequelize model
    metadata.forEach(function(element) {
      meta[element.key] = element.value;
    })
  } else {
    for(var key in metadata) {
      if (metadata.hasOwnProperty(key)) {
        meta[key] = metadata[key];
      }
    }
  }

  this.metadata = meta;
};

Analysis.prototype.toObject = function() {
  var outputDataSeriesList = [];
  this.analysis_dataseries_list.forEach(function(analysisDataSeries) {
    if (analysisDataSeries instanceof BaseClass)
      outputDataSeriesList.push(analysisDataSeries.toObject());
    else
      outputDataSeriesList.push(analysisDataSeries);
  })

  return Object.assign(BaseClass.prototype.toObject.call(this), {
    id: this.id,
    project_id: this.project_id,
    script_language: this.script_language,
    type: this['type_id'],
    name: this.name,
    description: this.description,
    active: this.active,
    output_dataseries_id: this['dataset_output'],
    metadata: this.metadata,
    'analysis_dataseries_list': outputDataSeriesList,
    schedule: this['schedule_id']
  });
};
