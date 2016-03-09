var assert = require('assert');

describe('DataManager', function() {
  var MainClass = require('../app');
  var app = require("../app");
  var config =  {
    "username": "postgres",
    "password": "postgres",
    "database": "nodejs_test",
    "host": "127.0.0.1",
    "dialect": "postgres",
    logging: false
  };
  app.set("databaseConfig", config);
  var DataManager = require("../core/DataManager");
  var expectedDataSerie = {
    name: "DataSeries1",
    description: "Desc DataSeries1",
    data_series_semantic_id: 1,
    data_provider_id: 1
  };

  // It runs before all tests. It initializes database, creating tables
  before(function(done){
    DataManager.init(function() {
      done();
    });
  });

  // It runs after all tests. It drops each table.
  after(function(done){
    var DataManager = require("../core/DataManager");
    DataManager.connection.drop({cascade: true}).then(function(e){
      console.log("Cleaning up database");

      done();
    });
  });

  //describe("DataManager#init", function() {
  it('initializes DataManager database connection', function(done) {
    DataManager.init(function(){
      assert.notEqual(DataManager.connection, null);
      return done();
    });

  });

  it('loads data models to DataManager', function(done) {
    DataManager.init(function(){
      DataManager.load(function(){
        assert.notEqual(DataManager.data, {});
        return done();
      });
    });
  });

  it('should insert Project in DataManager', function(done) {
    DataManager.init(function(){
      DataManager.load(function(){
        var project = {
          id: 1,
          name: "Project 1",
          version: 1,
          description: "Test Project"
        };

        DataManager.addProject(project).then(function(result) {
          assert(result.id > 0 && DataManager.data.projects.length == 1);
          return done();
        }).catch(function(err) {
          return done(err);
        });
      });
    });
  });

  it('should insert DataProvider in DataManager', function(done) {
    DataManager.init(function(){
      DataManager.load(function(){
        var provider = {
          name: "Provider 1",
          uri: "http://provider.com",
          description: "Test Provider",
          active: true,
          project_id: 1,
          data_provider_type_id: 1,
          data_provider_intent_id: 1
        };

        DataManager.addDataProvider(provider).then(function(result) {
          assert(result.id > 0 && DataManager.data.dataProviders.length == 1);
          return done();
        }).catch(function(err) {
          return done(err);
        })
      });
    });
  });

  it('should retrieve a DataProvider', function(done){
    DataManager.init(function(){
      DataManager.load(function(){
        var expected = {
          name: "Provider 1",
          uri: "http://provider.com",
          description: "Test Provider",
          active: true,
          project_id: 1
        };

        DataManager.getDataProvider({name: expected.name}).then(function(provider) {

          assert.equal(provider.name, expected.name);
          return done();

        }).catch(function(err) {

          return done(err);

        });
      });
    });
  });

  it('should update a DataProvider', function(done){
    DataManager.init(function(){
      DataManager.load(function(){
        var dataProvider = {
          name: "Provider 1"
        };
        DataManager.getDataProvider({name: dataProvider.name}).then(function(provider) {
          provider.name = "UpdatingProvider";

          DataManager.updateDataProvider(provider).then(function(result) {
            assert.equal(result.name, "UpdatingProvider");
            return done();
          }).catch(function(err) {
            return done(err);
          });
        });
      });
    });
  });

  it('should destroy a DataProvider', function(done){
    DataManager.init(function(){
      DataManager.load(function(){
        var expected = {
          name: "Provider 1"
        };

        DataManager.removeDataProvider({name: expected.name}).then(function() {
          assert.equal(DataManager.data.dataProviders.length, 0);
          return done();
        }).catch(function(err) {
          return done(err);
        });
      });
    });
  });

  //it('should insert DataSeries', function(done) {
  //  DataManager.init(function() {
  //    DataManager.load(function() {
  //      try {
  //        DataManager.addDataSerie(expectedDataSerie, function(result) {
  //          assert(result.id > 0 && DataManager.data.dataSeries.length == 1);
  //          return done();
  //        });
  //      }
  //      catch (err) {
  //        return done(err);
  //      }
  //    });
  //  });
  //});
});