var passport = require('../../config/Passport');

module.exports = function (app) {
  var controller = app.controllers.api.DataProvider;

  app.get('/api/DataProvider/', passport.isAuthenticated, controller.get);
  app.post('/api/DataProvider/', passport.isAuthenticated, controller.post);
  app.put('/api/DataProvider/:name', passport.isAuthenticated, controller.put);
};
